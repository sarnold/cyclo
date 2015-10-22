%{
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

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>

#include "lib.h"
#include "tokens.h"
%}

%o 7000
%a 9000
%p 5000
%n 3000
%e 4000


alpha	[a-zA-Z_]
alphat	[a-zA-Z_\~]
alnum	[a-zA-Z_0-9]
space	[\t ]
ops	[\<\>\!\%\^\&\*\-\+\=\[\]\~\,\.\/]
white	{space}*

%%

"template"{white}"\<".*"\>"	;
"class"{white}{alpha}{alnum}*	{
			  yylval.yy_str=getarg(yytext, yyleng, "class");
			  return CLASS;
			}
"struct"{white}{alpha}{alnum}*	{
			  yylval.yy_str=getarg(yytext, yyleng, "struct");
			  return STRUCT;
			}
"union"{white}{alpha}{alnum}*	{
			  yylval.yy_str=getarg(yytext, yyleng, "union");
			  return UNION;
			}
"goto"{white}{alpha}{alnum}*	{ 
                          yylval.yy_str=getarg(yytext, yyleng, "goto");
                          return GOTO; 
                        }
({alpha}{alnum}*{white}"::"{white}){0,1}{alphat}{alnum}*{white}"(" 		|
({alpha}{alnum}*{white}"::"{white}){0,1}"operator"{white}("\("{white}"\)"){0,1}({ops}|{white})*"("	|
({alpha}{alnum}*{white}){1,4}"(" {
			  char *t;
			  if(0!=(t=strchr(yytext,'~')))
			    if(!isalpha(*(t+1)) && *(t+1)!='_')
			    {
			      /* Not a destructor */
			      REJECT;
			    }
			  atelparen=1; /* ate left parenthesis */
			  yylval.yy_str=getfunc(yytext, yyleng);
			  /* the following three must appear to be function calls */
			  if(!strcmp(yylval.yy_str, "if")) return IF;
			  else if(!strcmp(yylval.yy_str, "elseif")) return ELSEIF;
			  else if(!strcmp(yylval.yy_str, "while")) return WHILE; 
			  else if(!strcmp(yylval.yy_str, "switch")) return SWITCH;
			  else if(!strcmp(yylval.yy_str, "elseswitch")) return SWITCH;
			  else if(!strcmp(yylval.yy_str, "for")) return FOR;
			  else if(!strcmp(yylval.yy_str, "return")) return RETURN;
			  else return FUNCTION;
			}
{alpha}{alnum}*{white}":"	{
			  yylval.yy_str=getlabel(yytext, yyleng);
			  if(!strcmp(yylval.yy_str, "default")) return DEFAULT;
			  return LABEL;
			}		
{alpha}{alnum}*		{      if(!strcmp(yytext, "do")) return DO;
			  else if(!strcmp(yytext, "case")) return CASE;
			  else if(!strcmp(yytext, "break")) return BREAK;
			  else if(!strcmp(yytext, "continue")) return CONTINUE;
			  else if(!strcmp(yytext, "return")) return RETURN;
			  else if(!strcmp(yytext, "else")) return ELSE;
			  /* ignore other identifiers */
			}
"&&"			{ return AND; }
"||"			{ return OR; }
";"			{ return ENDOFSTATEMENT; }
"{"			{ return BEGINSCOPE; }
"}"			{ return ENDSCOPE; }
"\n"			{ return NEWLINE; }
.			{ return yytext[0]; }
