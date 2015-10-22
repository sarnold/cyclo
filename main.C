/*

(c) 1993 Roger Binns

These tools were produced by Roger Binns for a fourth year project as part of
a computer science degree, for the Computer Science department, Brunel
University, Uxbridge, Middlesex UB8 3PH, United Kingdom.

This software is provided in good faith, having been developed by Brunel
University students as part of their normal course work.  It should not be
assumed that Brunel has any rights of ownership, and the University cannot
accept any liability for its subsequent use.  It is a condition of any such
use that the user idemnifies the University against any claim (including
third party claims) arising therefrom.

*/

#include <iostream>
#include <stdio.h>
#include <iomanip>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

using namespace std;

#define extern			/* instantiate lexer variables */
#include "tokens.h"
#undef extern

extern "C" TOKENS yylex (void);

struct Token
{
  enum TOKENS type;		/* token type (from tokens.h) */
  int lineno;			/* lineno the token appeared on */
  char *string;			/* optional information */
    Token ()
  {
    string = 0;
  }
  ~Token ()
  {
    if (string)
      free (string);
    string = 0;
  }
};

#undef _LIST_CLASSNAME
#undef _LIST_CLASSTYPE
#define _LIST_CLASSNAME TokenList
#define _LIST_CLASSTYPE Token
#include "list.h"

#undef _LIST_CLASSNAME
#undef _LIST_CLASSTYPE
#define _LIST_CLASSNAME IntList
#define _LIST_CLASSTYPE int
#include "list.h"

struct Place
{
  int tokenno;			/* which token this place corresponds to */
  int node;			/* node number of this place */
  IntList flows;		/* where we can go from this place */
  int implicitnext;		/* do we implicitly go to the next place */
  int display;			/* display this place in flowchart? */
    Place ()
  {
    tokenno = -1;
    implicitnext = 0;
    display=1;
    node=0;
  }
};

#undef _LIST_CLASSNAME
#undef _LIST_CLASSTYPE
#define _LIST_CLASSNAME PlaceList
#define _LIST_CLASSTYPE Place
#include "list.h"

struct Function
{
  char *name;
  PlaceList placelist;		/* list of places in function */
  int inl;			/* was it inline (nested in a struct/class/union) */
  int start;			/* token number function starts on (opening brace) */
  int end;			/* token number function ends on (closing brace) */
    Function ()
  {
    name = 0;
    inl = start = end = -1;
  }
   ~Function ()
  {
    if (name)
      free (name);
    name = 0;
  }
};

#undef _LIST_CLASSNAME
#undef _LIST_CLASSTYPE
#define _LIST_CLASSNAME FunctionList
#define _LIST_CLASSTYPE Function
#include "list.h"

#undef _STACK_TYPE
#undef _STACK_CLASSNAME
#define _STACK_TYPE      int
#define _STACK_CLASSNAME ScuStack
#include "stack.h"

TokenList tokenlist;		/* list of tokens */
FunctionList funclist;		/* list of functions */
ScuStack scustack;		/* nesting of struct/class/union scopes */
int ignoreinline;		/* should inline functions be ignored */

struct Info			/* for the recursive descent */
{
  int ibreak;			/* token number that keyword jumps to */
  int icontinue;
  int ireturn;
  int iswitch;			/* location of last switch statement */
  Function *function;		/* current Function */
    Info ()
  {
    ibreak = icontinue = ireturn = iswitch = -1;
    function = 0;
  }
};

/* consistency checking macro */
#define CHECK(x)  { if(!(x)) { cerr << "Check " << #x << " failed in " << __FILE__ << " line " << __LINE__ << endl; COREDUMP;} }

/* give Mr. User a nice output */
void print_token(int);
void pretty_token(int, ostream &);

/* some convenience macros - note that they also reduce the cyclomatic number! */
#define OUTOFMEMORY { cerr << "Out of memory in " << __FILE__ << " line " << __LINE__ << endl; exit(1); }
#define CHKMEM(x) { if(!(x)) {cerr << "Out of memory in " << __FILE__ << " line " << __LINE__ << endl; exit(1);} }

/* recursive descent parser */
static int rd_buffer(int parse, int startat, Info &jumps);
static int rd_scudefinition(int parse, int startat, Info &jumps);
static int rd_topscope(int parse, int startat, Info &jumps);
static int rd_topstatement(int parse, int startat, Info &jumps);
static int rd_functiondefinition(int parse, int startat, Info &jumps);
static int rd_statements(int parse, int startat, Info &jumps);
static int rd_statement(int parse, int startat, Info &jumps);
static int rd_scope(int parse, int startat, Info &jumps);
static int rd_while(int parse, int startat, Info &jumps);
static int rd_switch(int parse, int startat, Info &jumps);
static int rd_return(int parse, int startat, Info &jumps);
static int rd_if(int parse, int startat, Info &jumps);
static int rd_elseif(int parse, int startat, Info &jumps);
static int rd_else(int parse, int startat, Info &jumps);
static int rd_goto(int parse, int startat, Info &jumps);
static int rd_for(int parse, int  startat, Info &jumps);
static int rd_do(int parse, int startat, Info &jumps);
static int rd_default(int parse, int startat, Info &jumps);
static int rd_case(int parse, int startat, Info &jumps);
static int rd_break(int parse, int startat, Info &jumps);
static int rd_continue(int parse, int startat, Info &jumps);

/* what to do if a rd_* function returns an error */
#define check(x)  {end=(x); if (end<-1) return end;}
/* macro to handle parse error */
#define PARSEFAIL(m,x)  { parsefail(m,x); return -10;}

void parsefail(char *msg, int at)
{
  cerr << "Parsing failure \"" << msg << "\" - detectected at token ";
  pretty_token(at, cerr);
  cerr << " at line number " << tokenlist[at].lineno << endl;
}

/* Adds a new place to the current function */
Place *AddPlace(Info & jumps, int tokennum, int impnext=1)
{
  Place *p;
  CHKMEM(p=new Place);
  p->implicitnext=impnext;
  p->tokenno=tokennum;
  CHKMEM(jumps.function->placelist.Add(p));
  return p;
}

/* Adds 'jump' to flows for place corresponding to 'which' */
void AddToPlace(Info &jumps, int which, int jump)
{
  int *t;
  CHKMEM(t=new int);
  *t=jump;
  CHECK((jumps.function->placelist[which-jumps.function->start].tokenno)==which);
  CHKMEM(jumps.function->placelist[which-jumps.function->start].flows.Add(t));
}

/* Turns implicitnext off for a token */
void NoImplicit(Info &jumps, int which)
{
  CHECK(jumps.function->placelist[which-jumps.function->start].tokenno==which);
  jumps.function->placelist[which-jumps.function->start].implicitnext=0;
}

/* The actual recursive descent parser starts here */
int rd_buffer(int parse, int startat, Info &jumps)
{
  int end;

  while(startat<tokenlist.Count())
  {
    if(tokenlist[startat].type==ENDSCOPE)
      break;
    check(rd_topstatement(parse, startat, jumps));
    if(end==-1)
      return -1;
    startat=end;
  }
  return startat;
}

int rd_scudefinition(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=CLASS && tokenlist[startat].type!=STRUCT && tokenlist[startat].type!=UNION)
    return -1;

  if(startat+1>=tokenlist.Count() || tokenlist[startat+1].type!=BEGINSCOPE)
    return -1;

  int *t;
  CHKMEM(t=new int);
  *t=startat;
  CHKMEM(scustack.Push(t));

  check(rd_buffer(parse, startat+2, jumps));
  if(end==-1) return end;

  if(end>=tokenlist.Count() || tokenlist[end].type!=ENDSCOPE)
      PARSEFAIL((char *)"No terminating '}' found", startat);

  scustack.Pop();

  return end+1;
}

int rd_topscope(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=BEGINSCOPE) return -1;

  check(rd_buffer(parse, startat+1, jumps));
  if(end==-1 || end>=tokenlist.Count()) 
    PARSEFAIL((char *)"Invalid scope contents", startat);

  if(tokenlist[end].type!=ENDSCOPE)
    PARSEFAIL((char *)"No terminating '}' found", startat);

  return end+1;
}

int rd_topstatement(int parse, int startat, Info &jumps)
{
  int end;

  while(startat<tokenlist.Count())
  {
#define ISIT(x) check( x (0, startat, jumps)); \
              if(end!=-1) \
                { \
                  if(parse) \
                    check( x (parse, startat, jumps)); \
                  return end; \
                }
    ISIT(rd_scudefinition);
    ISIT(rd_functiondefinition);
    ISIT(rd_topscope);
#undef ISIT

    if(tokenlist[startat].type==ENDSCOPE)
      return startat;

    startat++;
  }
 return startat;
}

int rd_functiondefinition(int parse, int startat, Info &jumps)
{
  int end;
	
  if(tokenlist[startat].type!=FUNCTION)
    return -1;

  if(startat+1>=tokenlist.Count() || tokenlist[startat+1].type!=BEGINSCOPE)
    return -1;

  check(rd_statements(0, startat+2, jumps));
again:
  if(end==-1)
    PARSEFAIL((char *)"Unrecognised function body", startat);

  if(end>=tokenlist.Count() || tokenlist[end].type!=ENDSCOPE)
    PARSEFAIL((char *)"Function body has no terminating '}'", startat);

  if(scustack.Size() && ignoreinline)
    parse=0;

  if(parse)
    {
      Info i=jumps;
      Function *f;
      CHKMEM(f=new Function);

      CHKMEM(funclist.Add(f));
      f->start=startat+1;
      f->end=end;
      if(scustack.Size())
        {
   	  int a;
   	  int sz=0;
   	  f->inl=1;
   	  for(a=0; a<scustack.Size(); a++)
   	    sz+=2+strlen(tokenlist[scustack.Top(a)].string);
   	  sz+=strlen(tokenlist[startat].string);
   	  CHKMEM(f->name=new char[sz+1]);
   	  f->name[0]=0;

   	  for(a=scustack.Size()-1; a>=0; a--)
   	    {
   	      strcat(f->name, tokenlist[scustack.Top(a)].string);
   	      strcat(f->name, "::");
   	    }
   	  strcat(f->name, tokenlist[startat].string);
        }
      else
        CHKMEM(f->name=strdup(tokenlist[startat].string));
      i.function=f;
      i.ireturn=end;
      AddPlace(i,startat+1);
      check(rd_statements(parse, startat+2, i));
      if(end>=0) AddPlace(i,end,0);
      parse=0;
      goto again; /* always useful for error handling */
    }
  return end+1; /* token after terminating '}' */
}

int rd_statements(int parse, int startat, Info &jumps)
{
  int end=0; /* stop compiler warning */
  while(startat<tokenlist.Count())
  {
    check(rd_statement(parse, startat, jumps));
    if(end==-1)
      return startat;
    startat=end;
  }
  return end;
}
  
int rd_statement(int parse, int startat, Info &jumps)
{
  int end;
  while(startat<tokenlist.Count())
  {
#define ISIT(x) check( x (0, startat, jumps)); \
              if(end!=-1) \
                { \
                  if(parse) \
                    check( x (parse, startat, jumps)); \
                  return end; \
                }
    ISIT(rd_scope);
    ISIT(rd_while);
    ISIT(rd_switch);
    ISIT(rd_return);
    ISIT(rd_if);
    ISIT(rd_elseif);
    ISIT(rd_else);
    ISIT(rd_goto);
    ISIT(rd_for);
    ISIT(rd_do);
    ISIT(rd_default);
    ISIT(rd_case);
    ISIT(rd_break);
    ISIT(rd_continue);
#undef ISIT
    switch(tokenlist[startat].type)
    {
    case FUNCTION:
    case NEWLINE:
    case LABEL:
    case STRUCT:
    case CLASS:
    case UNION:
    case AND:
    case OR:
    case IGNORE:
      if(parse)
	AddPlace(jumps, startat);
      break;
    case ENDSCOPE:
      return -1;
    case ENDOFSTATEMENT:
      if(parse)
        AddPlace(jumps, startat);
      return startat+1;
    default:
      PARSEFAIL((char *)"Unexpected token", startat);
    }
    startat++;
  }
  return startat;
}

int rd_scope(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=BEGINSCOPE)
    return -1;

  if(parse)
    AddPlace(jumps, startat);
  check(rd_statements(0, startat+1, jumps));
  if(end==-1 || end>=tokenlist.Count() || tokenlist[end].type!=ENDSCOPE)
    PARSEFAIL((char *)"Scope has no closing '}'", startat);
  if(parse)
    {
      check(rd_statements(parse, startat+1, jumps));
      if(end==-1 || end>=tokenlist.Count() || tokenlist[end].type!=ENDSCOPE)
         PARSEFAIL((char *)"Scope has no closing '}'", startat);
      AddPlace(jumps, end);
    }
  return end+1;
}

/* Ensures that there was a following statement */
#define HASSTATEMENT(x) { if(end==-1 || end>=tokenlist.Count()) PARSEFAIL((char *)"'" x "' has no following ';' or '{'..'}'", startat); }

int rd_while(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=WHILE)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("while");

  if(parse)
    {
      AddPlace(jumps, startat);
      AddToPlace(jumps, startat, end);
      Info i=jumps;
      i.ibreak=end;
      i.icontinue=startat;

      check(rd_statement(parse, startat+1, i));
      HASSTATEMENT("while");
      NoImplicit(jumps, end-1);
      AddToPlace(jumps, end-1, startat);
    }
  return end;
}

int rd_switch(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=SWITCH)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("switch");

  if(parse)
    {
      Place *p=AddPlace(jumps, startat, 0);

      Info i=jumps;
      i.ibreak=end;
      i.iswitch=startat;

      check(rd_statement(parse, startat+1, i));
      HASSTATEMENT("switch");

      /* check for default */
      int founddefault=0;
      for(int a=0; a<p->flows.Count(); a++)
        if(tokenlist[p->flows[a]].type==DEFAULT)
          founddefault=1;
      if(!founddefault)
        AddToPlace(jumps, startat, end);
    }
  return end;
}

int rd_return(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=RETURN)
    return -1;

  check(rd_statement(0, startat+1, jumps));
again:
  HASSTATEMENT("return"); /* must have a following ; */

  if(parse)
    {
      AddPlace(jumps, startat, 0);
      check(rd_statement(parse, startat+1, jumps));
      parse=0;
      AddToPlace(jumps, startat, jumps.ireturn);
      goto again;
    }
  return end;
}

int rd_elseif(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=ELSEIF)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("elseif");

  if(parse)
    {
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("elseif");
      AddToPlace(jumps, startat, end);
    }

  if(tokenlist[end].type!=ELSE)
    return end;

  startat=end; /* else statement */
  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("else");

  if(parse)
    {
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("else");
      NoImplicit(jumps, startat-1);
      AddToPlace(jumps, startat-1, end);
    }

  return end;
}

int rd_else(int parse, int startat, Info &jumps)
{
  int end;

	if(tokenlist[startat].type!=ELSE)
    	return -1; 

     check(rd_statement(0, startat+1, jumps));
     HASSTATEMENT("else");

     if(parse)
      {
       AddPlace(jumps, startat);
       check(rd_statement(parse, startat+1, jumps));
       HASSTATEMENT("else");
       NoImplicit(jumps, startat-1);
       AddToPlace(jumps, startat-1, end);
       }
	return end;
}            

int rd_if(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=IF)
    return -1;
	
  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("if");

  if(parse)
    {
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("if");
      AddToPlace(jumps, startat, end);
    }

	return end;
}

int rd_goto(int parse, int startat, Info &jumps)
{
  int a, end;

  if(tokenlist[startat].type!=GOTO)
    return -1;

  check(rd_statement(0, startat+1, jumps));
again:
  HASSTATEMENT("goto");

  if(parse)
    {
      AddPlace(jumps, startat, 0);
      for(a=jumps.function->start; a<jumps.function->end; a++)
        if(tokenlist[a].type==LABEL && !strcmp(tokenlist[a].string, tokenlist[startat].string))
          AddToPlace(jumps, startat, a);

      if(a<jumps.function->end)
        PARSEFAIL((char *)"Label not found", startat);
      check(rd_statement(parse, startat+1, jumps));
      parse=0;
      goto again;
    }
  return end;
}
      
int rd_for(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=FOR)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("for-initial");
  check(rd_statement(0, end, jumps));
  HASSTATEMENT("for-condition");
  check(rd_statement(0, end, jumps));
  HASSTATEMENT("for(;;)");
  int endloop=end;

  if(parse)
    {
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("for-initial");
      check(rd_statement(parse, end, jumps));
      HASSTATEMENT("for-condition");
      Info i=jumps;
      i.ibreak=endloop;
      i.icontinue=startat;
      check(rd_statement(parse, end, i));
      HASSTATEMENT("for(;;)");
      AddToPlace(jumps, startat, end);
      AddToPlace(jumps, end-1, startat);
      NoImplicit(jumps, end-1);
    }
  return end;
}

int rd_do(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=DO)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("do");
/*
  if(tokenlist[end].type!=WHILE)
    PARSEFAIL((char *)"Expecting 'while' after 'do'", end);
*/
  int whileat=end;
  check(rd_statement(0, end+1, jumps));
  HASSTATEMENT("'do'..'while'");

  if(parse)
    {
      Info i=jumps;

      AddPlace(jumps, startat);
      i.ibreak=end;
      i.icontinue=whileat;

      check(rd_statement(parse, startat+1, i));
      HASSTATEMENT("do");
      CHECK(end==whileat);
      AddPlace(jumps, whileat);
      check(rd_statement(parse, end+1, jumps));
      HASSTATEMENT("'do'..'while'");
      AddToPlace(jumps, end-1, startat);
    }
  return end;
}

int rd_default(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=DEFAULT)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("default");

  if(parse)
    {
/*
      if(jumps.iswitch==-1)
        PARSEFAIL((char *)"No enclosing switch statement", startat);
*/
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("default");
      AddToPlace(jumps, jumps.iswitch, startat);
    }
  return end;
}

int rd_case(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=CASE)
    return -1;

  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("case");

  if(parse)
    {
/*
      if(jumps.iswitch==-1)
        PARSEFAIL((char *)"No enclosing switch statement", startat);
*/
      AddPlace(jumps, startat);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("case");
      AddToPlace(jumps, jumps.iswitch, startat);
    }
  return end;
}

int rd_break(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=BREAK)
    return -1;
  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("break");

  if(parse)
    {
/*
      if(jumps.ibreak==-1)
        PARSEFAIL((char *)"No enclosing loop to break from", startat);
*/
      AddPlace(jumps, startat,0);
      AddToPlace(jumps, startat, jumps.ibreak);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("break");
    }
  return end;
}

int rd_continue(int parse, int startat, Info &jumps)
{
  int end;

  if(tokenlist[startat].type!=CONTINUE)
    return -1;
  check(rd_statement(0, startat+1, jumps));
  HASSTATEMENT("continue");

  if(parse)
    {
/*
      if(jumps.icontinue==-1)
        PARSEFAIL((char *)"No enclosing loop to continue from", startat);
*/
      AddPlace(jumps, startat,0);
      AddToPlace(jumps, startat, jumps.icontinue);
      check(rd_statement(parse, startat+1, jumps));
      HASSTATEMENT("continue");
    }
  return end;
}

/* work out control graph for all functions in tokenlist */
void
cyclo (void)
{
  Info i;
  int x;

  x=rd_buffer(1, 0, i);
  if(x>=0 && x<tokenlist.Count())
    parsefail((char *)"Unexpected token at top level", x);
  if(x!=tokenlist.Count()) exit(1);
}

/* tokenise standard input */
void
tokenise (void)
{
  enum TOKENS c;
  int line = 1;
  Token *t;
  int inparens=0;

  while (1)
    {
      c = yylex ();
      if(c=='"' || c=='\'')
        {
          cerr << "WARNING: This source should be run through 'mcstrip' first." << endl;
        }
      if (c == NEWLINE)
	{
	  line++;
	  continue;
	}
      if(c=='(' || atelparen) 
        { inparens++; atelparen=0; if(inparens>1) continue; }
      else if(c==')')
        {
        inparens-=inparens?1:0;
        continue;
        }
      else if(c && inparens && c!=AND && c!=OR && c!=ENDOFSTATEMENT) continue; /* have to keep ';' for 'for' */
      if(c>0 && c<NEWLINE)
        continue;
      t = new Token;
      if (!t)
	{
	  cerr << "Out of memory in " << __FILE__ << " line " << __LINE__ << "\n";
	  exit (1);
	}
      if (!c)
	{
	t->type = IGNORE;	/* end of file buffer */
	t->lineno=-1;
	}
      else
	{
	  t->type = c;
	  t->lineno = line;
	}
      if (c == GOTO || c == FUNCTION || c == LABEL || c == STRUCT || c == UNION || c == CLASS)
	t->string = yylval.yy_str;
      if (!tokenlist.Add (t))
	{
	  cerr << "Out of memory in " << __FILE__ << " line " << __LINE__ << "\n";
	  exit (1);
	}
      if (!c)
	break;
    }
}

/* print a 'pretty' token */
void pretty_token(int which, ostream &os)
{
  switch(tokenlist[which].type)
  {
  case ENDOFSTATEMENT: os << "';'"; break;
  case IF:             os << "'if'"; break;
  case ELSE:           os << "'else'"; break;
  case ELSEIF:           os << "'elseif'"; break;
  case FOR:            os << "'for'"; break;
  case WHILE:          os << "'while'"; break;
  case BREAK:          os << "'break'"; break;
  case CONTINUE:       os << "'continue'"; break;
  case BEGINSCOPE:     os << "'{'"; break;
  case ENDSCOPE:       os << "'}'"; break;
  case RETURN:         os << "'return'"; break;
  case DO:             os << "'do'"; break;
  case SWITCH:         os << "'switch'"; break;
  case CASE:           os << "'case'"; break;
  case AND:            os << "'&&'"; break;
  case OR:	       os << "'||'"; break;
  case DEFAULT:        os << "'default'"; break;
  case NEWLINE:
  case IGNORE:         os << "INTERNAL"; break;
  case GOTO:           os << "'goto " << tokenlist[which].string << "'"; break;
  case FUNCTION:       os << "'" << tokenlist[which].string << "()'"; break;
  case LABEL:          os << "'" << tokenlist[which].string << ":'"; break;
  case STRUCT:         os << "'struct " << tokenlist[which].string << "'"; break;
  case CLASS:          os << "'class " << tokenlist[which].string << "'"; break;
  case UNION:          os << "'union " << tokenlist[which].string << "'"; break;
  default:             os << "character '" << (char)tokenlist[which].type << "'"; break;
  }
}

/* print a token */
void
print_token (int which)
{
  char temp[1024];		/* temporary space */

  switch (tokenlist[which].type)
    {
#define tk(x)  case x: cout << #x << " "; break;
        tk (ENDOFSTATEMENT);
	tk (IF);
	tk (ELSE);
	tk (ELSEIF);
	tk (FOR);
	tk (WHILE);
	tk (BREAK);
	tk (CONTINUE);
	tk (BEGINSCOPE);
	tk (ENDSCOPE);
	tk (RETURN);
	tk (DO);
	tk (SWITCH);
	tk (CASE);
	tk (AND);
	tk (OR);
	tk (DEFAULT);
	tk (IGNORE);
	tk (NEWLINE);
#undef tk
#define tk(x) case x:sprintf(temp,"%s %s",#x,tokenlist[which].string);cout<<temp<<" ";break;
	tk (GOTO);
	tk (FUNCTION);
	tk (LABEL);
	tk (STRUCT);
	tk (CLASS);
	tk (UNION);
	break;
    }
}

/* print results of tokenisation */
void
print_tokens (void)
{
  int line = -1, curscope = 1;
  for (int a = 0; a < tokenlist.Count (); a++)
    {
      if(tokenlist[a].type==IGNORE) continue;
      if (tokenlist[a].lineno != line)
	{
	  cout << endl;
	  line = tokenlist[a].lineno;
	  cout << setw (5) << line;
	  for (int b = 0; b < curscope; b++)
	    cout << "  ";
	}
      print_token (a);
      if (tokenlist[a].type == BEGINSCOPE)
	curscope++;
      if (tokenlist[a].type == ENDSCOPE)
	curscope--;
    }
  cout << endl;
}

/* print results of cyclo */
void
print_funcs (void)
{
  int a, indent;
  for (a = 0; a < funclist.Count (); a++)
    {
      cout << "Function Name: " << funclist[a].name << endl;
      indent=1;
      for (int b = 0; b < funclist[a].placelist.Count (); b++)
	{
	  cout << setw (5) << tokenlist[funclist[a].placelist[b].tokenno].lineno << setw (5) << funclist[a].placelist[b].tokenno;
	  for (int c = 0; c < indent; c++)
	    cout << "  ";
	cout << setw (32 - indent * 2) << setiosflags (ios::left);
	  if (tokenlist[funclist[a].placelist[b].tokenno].type == BEGINSCOPE)
	    indent++;
	  if (tokenlist[funclist[a].placelist[b].tokenno].type == ENDSCOPE)
	    indent--;
	  print_token (funclist[a].placelist[b].tokenno);
	cout << resetiosflags (ios::left);

	  if (funclist[a].placelist[b].implicitnext)
	    cout << " next";
	  for (int d = 0; d < funclist[a].placelist[b].flows.Count (); d++)
	    cout << setw (5) << funclist[a].placelist[b].flows[d];
	  cout << endl;
	}
    }
}

int	TotalComplexity=0;

/* calculate cyclomatic complexity */
void print_complexity(int fnno)
{
  if(fnno<0 || fnno>funclist.Count())
    {
       cerr << "Function number " << fnno << " does not exist!" << endl;
       exit(1);
    }

  int cc,b;

  for(int a=0; a<funclist.Count(); a++)
    {
      if(fnno && fnno-1!=a)
        continue;
      cout << funclist[a].name << "\t" ;
      cc=2; /* one + one to make it strongly connected */
      for(b=0; b<funclist[a].placelist.Count(); b++)
        {
          cc-=1; /* sub one for the node */
          cc+=funclist[a].placelist[b].flows.Count();
          cc+=funclist[a].placelist[b].implicitnext;
          if(tokenlist[funclist[a].placelist[b].tokenno].type==AND ||
             tokenlist[funclist[a].placelist[b].tokenno].type==OR)
                cc++;  /* McCabe cites these as adding to complexity */
        }
		TotalComplexity += cc;
      cout << cc << endl;
    }
}

/* print functions for user */
void print_func_list(int detail, int fnno)
{
  if(fnno<0 || fnno>funclist.Count())
    {
      cerr << "Function number " << fnno << " does not exist!" << endl;
      exit(1);
    }
  int a,b,c,found;
	printf ("funclist.Count() is %d\n", funclist.Count());
  for(a=0; a<funclist.Count(); a++)
    {
      if(fnno && fnno-1!=a)
        continue;
      cout << funclist[a].name << endl;
      if(detail>1)
        for(b=0; b<funclist[a].placelist.Count(); b++)
          if(tokenlist[funclist[a].placelist[b].tokenno].type==FUNCTION)
            {
              found=0;
              for(c=0; c<b; c++)
                if(tokenlist[funclist[a].placelist[c].tokenno].type==FUNCTION &&
                   !strcmp(tokenlist[funclist[a].placelist[b].tokenno].string, tokenlist[funclist[a].placelist[c].tokenno].string))
                     found=1;
              if(!found)
                cout << "    " << tokenlist[funclist[a].placelist[b].tokenno].string << endl;
             }
     }
}

/* display arc in postscript */
void display_arc(int nodefrom, int nodeto)
{
  if(nodefrom==nodeto)
    cout << nodefrom << " arcloop" << endl;
  else if(nodefrom>nodeto)
    cout << nodeto << " " << nodefrom << " arcback" << endl;
  else cout << nodefrom << " " << nodeto << " arcfore" << endl;
}

/* display nodes */
void display(int which, int debug)
{
  int numnodes=0, nodeto, a, b;

  for(a=0; a<funclist[which].placelist.Count(); a++)
    if(funclist[which].placelist[a].node>numnodes)
      numnodes=funclist[which].placelist[a].node;

  cout << "/numnodes " << numnodes << " def    % number of nodes" << endl;
  if(debug) cout << "( Graph of function " << funclist[which].name << " \\n) print" << endl;

  for(a=0; a<funclist[which].placelist.Count(); a++)
    {
      if(!funclist[which].placelist[a].display)
        continue;
      cout << "(" << tokenlist[funclist[which].placelist[a].tokenno].lineno << ") " <<
        funclist[which].placelist[a].node << " node " << endl;

      if(funclist[which].placelist[a].implicitnext)
        {
        nodeto=1+funclist[which].placelist[a].node;
        display_arc(funclist[which].placelist[a].node, nodeto);
        }

      for(b=0; b<funclist[which].placelist[a].flows.Count(); b++)
        {
          nodeto=funclist[which].placelist[funclist[which].placelist[a].flows[b]-funclist[which].start].node;
          display_arc(funclist[which].placelist[a].node, nodeto);
        }
    }
  cout << endl;
}

/* set redundant places to not display */
void opt_display(int which)
{
  int a,b,c,found;

  for(a=1; a<funclist[which].placelist.Count(); a++)
    {
    if(funclist[which].placelist[a].flows.Count())
      continue;
    found=0;
    for(c=0; c<funclist[which].placelist.Count(); c++)	
      for(b=0; b<funclist[which].placelist[c].flows.Count(); b++)
        if(funclist[which].placelist[c].flows[b]==funclist[which].placelist[a].tokenno)
          found++;
    if(!(found+funclist[which].placelist[a-1].implicitnext))
       {  /* unreachable */
         funclist[which].placelist[a].implicitnext=0;
         funclist[which].placelist[a].display=0;
         continue;
       }
    if (!found && funclist[which].placelist[a].implicitnext && !funclist[which].placelist[a-1].flows.Count())
      funclist[which].placelist[a].display=0;
    }

  int node=1;

  for(a=0; a<funclist[which].placelist.Count(); a++)
    if(funclist[which].placelist[a].display)
      {
        funclist[which].placelist[a].node=node;
        node++;
      }
}

/* print the postscript header */
void print_ps_header(float)
{
  cout <<
  "%!PS\n" <<
  "%%Creator: cyclo - Control flow graph\n" <<
#include "graph.psh"
   << endl;
  
  /* parameters */
  cout <<
"/xconst    10 def       % distance bubbles are apart (x)"  "\n"
"/yconst    10 def       % distance bubbles are apart (y)"  "\n"
"/radius     6 def       % radius of bubble"  "\n"
"/loopfactor 2 def       % multiply by radius for loopback distance"  "\n"
"/fontsize   6 def       % point size of text in bubbles"  "\n"
"/fudge     .4 def       % flatness of lines between bubbles"  "\n"
"/linewid  0.2 def       % width of lines between bubbles"  "\n"
"/bubble   0.6 def       % width of circle around bubbles "  "\n"
"/arrow    0.6 def       % arrowhead length as proportion of bubble radius"  "\n"
"" << endl;
}

/* print postscript trailer */
void print_ps_trailer(void)
{
  /* nothing */
}

/* print postscript header for each page */
void print_ps_pageheader(int which, float scale)
{
  cout << "% Flow graph for function " << funclist[which].name << endl;
  cout  << "/Helvetica-Narrow findfont fontsize scalefont setfont" "\n"
     "% move away from margins" "\n"
     "20 40 translate" "\n"
     << scale << " " << scale << " scale" << endl;
}

/* print postscript trailer for each page */
void print_ps_pagetrailer(void)
{
  cout << "showpage" << endl;
}

void usage(char *argv[])
{
  cerr << "Usage: " << argv[0] << " [-l] [-t] [-f|-F] [-n func#] [-p] [-s scale] [-c] [-d] [-i]" "\n"
  "Information:" "\n"
  "  -l        print results of tokenisation " "\n"
  "  -t        print results of flow generation" "\n"
  "Function details:" "\n"
  "  -i        ignore functions implemented within a class/structure/union declaration" "\n"
  "  -n func#  set function for -F/-f/-p/-c to number given" "\n"
  "  -f        print function names found in source" "\n"
  "  -F           ...  with the list of functions they call" "\n"
  "  -c        print cyclomatic complexity of functions" "\n"
  "  -p        produce postscript flow graph" "\n"
  "  -d        log function name to ps stdout" "\n"
  "  -s scale  scale postscipt output (float)" "\n"
  "Notes: Input is expected from standard input.  Ensure you filter the code" "\n"
  "       through 'mcstrip' first.  The utility 'ps2epsi' from ghostscript can" "\n"
  "       turn the postscript output into encapsulated postscript.  The default" "\n"
  "       scale of 1 produces ten bubbles per inch." << endl;

  exit(1);
}

int	Printtotal=1;

#ifdef NEEDGETOPTDEFS
// declarations to keep g++ happy
// messes up other compilers; e.g., sunpro
// cml 960805
extern "C" int getopt(int, char **, char *);
extern "C" char *optarg;
#endif

int
main (int argc, char *argv[])
{
  int c;
  int plt=0, funcs=0, pft=0, ps=0, psd=0,fnno=0, complex=0;
  float scale=1;

  while(0<=(c=getopt(argc, argv, "lfFtn:ps:cdi")))
     switch(c)
     {
     case 'c':  complex=1; break;
     case 'i':  ignoreinline=1; break;
     case 'd':  psd=1; break;
     case 'l':  plt=1; break;
     case 'f':  funcs=1; break;
     case 'F':  funcs=2; break;
     case 't':  pft=1; break;
     case 'n':  Printtotal=0;fnno=atoi(optarg); break;
     case 'p':  ps=1; break;
     case 's':  scale=atof(optarg); break;
     default: usage(argv);
     }

  if(!plt && !funcs && !pft && !ps && !complex)
    {
      cerr << "You must give the program something to do!" << endl;
      usage(argv);
    }
  if(!!funcs+!!(plt+pft)+ps+complex!=1)
    {
      cerr << "You have given the program too many things to do!" << endl;
      usage(argv);
    }
  tokenise ();
  if(plt) print_tokens ();
  cyclo ();
  if(pft) print_funcs ();
  if(plt || pft) return 0; /* exit after printing trees */
  if(complex)
    {
      print_complexity(fnno);
	if (complex && Printtotal)
		printf ("\nTotal complexity:    %d\n", TotalComplexity);
      exit(0);
    }

  if(funcs)
    {
    print_func_list(funcs, fnno);
    exit(0);
    }

  /* do postscript output */
  if(fnno<0 || fnno>funclist.Count())
    {
      cerr << "There aren't that many (" << fnno << ") functions!" << endl;
      exit(1);
    }

  print_ps_header(scale);

  for(c=0; c<funclist.Count(); c++)
    {
      if(fnno && c+1!=fnno)
        continue;

      opt_display(c);
      print_ps_pageheader(c, scale);
      display(c, psd);
      print_ps_pagetrailer();
    }
  print_ps_trailer();
  return 0;
}
