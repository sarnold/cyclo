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

/*
  This little piece of lex removes all comments, string and character constants,
  and can provide some 'traditional' metrics.
*/


#include <stdio.h>
#include <stdlib.h>



/*
extern char **optarg;
*/

extern int optind;
extern int getopt();

#include <errno.h>
#include <string.h>

#include "strerror.h"

/* this aren't declared yet */
int yylex();

int numcomments=0;
int numlines=0;
int incomment=0;
int op=1;

int main(int argc, char *argv[])
{
   int plines=0, pcomm=0, pratio=0, pall=0;
   int c;

   while(0<=(c=getopt(argc, argv, "lcra")))
   switch(c)
   {
   case 'l': plines=1; op=0; break;
   case 'c': pcomm=1; op=0; break;
   case 'r': pratio=1; op=0; break;
   case 'a': pall=1; op=0; break;
   default: fprintf(stderr, "Usage: %s [-l] [-c] [-r] [-a] [inputfile]\n"
   			    "  -l  print lines in inputs\n"
   			    "  -c  print lines of comments\n"
   			    "  -r  print ratio of comments to lines as a percentage\n"
   			    "  -a  print all of above\n"
   			    "Standard input is read if no file is given.\n"
   			    "If no options are given, the input is stripped of comments,\n"
   			    "character and string constants, and sent to standard output.\n",
   			     argv[0]);
   		exit(1);
   }

   if(optind<argc)
     {
       if(!freopen(argv[optind], "rt", stdin))
         {
           /* This is highly unportable - but the ANSI stream stuff won't tell you
              why the call fails.  Replace if not in a UNIX or MSDOS environment or
              workalike. */
#ifndef _BRUNEL_SUN
           fprintf(stderr, "%s: \"%s\" - %s\n", argv[0], argv[optind], strerror(errno));
#else
		   fprintf(stderr, "%s: ", argv[0]);
		   perror(argv[optind]);
#endif
           exit(1);
         }
     }
   while(yylex()) ;
   
   if(pcomm)
     printf("%d ", numcomments);
   if(plines)
     printf("%d ", numlines);
   if(pratio)
     printf("%d ", numcomments*100/numlines);
   if(pcomm||plines||pratio) printf("\n");
   if(pall)
     printf("%d comment lines\n%d lines\n%d %% comments to lines ratio\n",
		numcomments, numlines, numcomments*100/numlines);
  return 0;
}
%}

%%

^#.*$			;
\"([^"\\\n]|\\.)*\"     ;
\'([^\'\\\n]|\\.)*\'    ;
"//".*$			{ numcomments++; }
"\n"			{ if(op) {putchar('\n');} numlines++; if(incomment) numcomments++;}
"/*"			{ incomment=1; numcomments++; }
"*/"			{ incomment=0; if(op){putchar(' ');}}
.			{ if(!incomment) if(op){putchar(yytext[0]);} }
