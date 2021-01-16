
/* strerror.c
 *
 * cml hacked on this function 940627
 * so that the error message would be copied into a static buffer, 
 * and then return a ptr to that buffer.
 * 
 * Previously, a ptr into the sys_errlist array was returned,
 * but this caused core dumps.  I don't understand the problem
 * but it's better now.
 */
#include "strerror.h"

#ifndef HAVE_STRERROR
static char *
private_strerror (errnum)
     int errnum;
{
  extern char *sys_errlist[];
  extern int sys_nerr;

  if (errnum > 0 && errnum <= sys_nerr)
    return sys_errlist[errnum];

  return "Unknown system error";
}
#define strerror private_strerror
#endif /* HAVE_STRERROR */

#ifdef DRIVER
#include <errno.h>

extern int sys_nerr;

int main(argc, argv)
     char **argv;
{
  int i;

  for (i = 0; i < sys_nerr; ++i)
    printf("Errno = %d, msg = %s\n", i, strerror(i));

  return 0;
}
#endif
