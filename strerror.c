
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

char *strerror(err)
     int err;
{

  extern int sys_nerr;
  extern char *sys_errlist[];

  static char errmsg[256];

  if (err >= 0 && err < sys_nerr)
    strcpy(errmsg, sys_errlist[err]);
  else
    (void) sprintf(errmsg, "Error %d", err);

  return errmsg;
}

  
#if 0
char *
strerror(err)
        int err;
{
        extern int sys_nerr;
        extern char *sys_errlist[];

        static char errmsg[32];

        if (err >= 0 && err < sys_nerr)
                return sys_errlist[sys_nerr];

        (void) sprintf(errmsg, "Error %d", err);
        return errmsg;
}
#endif

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
