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
#include <iostream>
#include <string.h>
#include "lib.h"

using namespace std;

extern "C" // called from 'C' scan.c
{

char *my_strndup(char *str, int n)
{
	char *t, *t2;
	t2=t=(char *)malloc(n+1*sizeof(char));
	if(!t) { cerr << "Out of memory in " << __FILE__ << " line " << __LINE__ << "\n"; exit(1);}

	while(n && str)
	{
		*t2=*str;
		n--;
		str++;
		t2++;
	}
	*t2=0;
	return t;
}

char *getfunc(char *str, int n)
{
	char *t=my_strndup(str, n-1); /* leave off terminating parenthesis */
	char *t2=t;

	while(*t)
	{
	if(*t==' ' || *t=='\t' || *t=='\n')
	  strcpy(t, t+1);
	t++;
	}
	return t2;
}

char *getarg(char *str, int n, char *keyword)
{
	char *t=my_strndup(str, n);
	char *t2=t;

	strcpy(t, t+strlen(keyword)); /* remove keyword */

	while(*t)
	{
	if(*t==' ' || *t=='\t' || *t=='\n')
	  strcpy(t, t+1);
	t++;
	}
	return t2;
}

char *getlabel(char *str, int n)
{
	char *t=my_strndup(str, n-1); /* remove trailing ':' */
	char *t2=t;

	while(*t)
	{
	if(*t==' ' || *t=='\t') 
	  {
	    *t=0;
	    break;
	  }
	t++;
	}
	return t2;
}



} // extern "C"
