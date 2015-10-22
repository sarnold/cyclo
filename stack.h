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

// This should be implemented using templates, but g++ doesn't
// support them yet.

// Quick and dirty.  See the initial comments in list.h upon which this is based.

// NOTE: _STACK_TYPE should be a text only single token.  Use typedefs.

#ifndef COREDUMP
#define COREDUMP  *((char*)0)=0  // causes a segmentation fault (only under UNIX)
#endif

#ifndef _STACK_CLASSNAME
#error "_STACK_CLASSNAME must be defined."
#endif

#ifndef _STACK_TYPE
#error "_STACK_TYPE must be defined."
#endif

#define _ST_paste(x,y) x##y
#undef _LIST_CLASSNAME
#define _LIST_CLASSNAME _ST_paste(_STACK_LIST_, _STACK_TYPE)
#undef _LIST_CLASSTYPE
#define _LIST_CLASSTYPE _STACK_TYPE

#include "list.h"

class _STACK_CLASSNAME : private _LIST_CLASSNAME
{
  // default constructor & destructors

public:
  int Size(void) { return Count(); }
  int Push(_STACK_TYPE * item) { return Add(item); }
  _STACK_TYPE &Top(int which=0);
  void Pop(void);
};

inline _STACK_TYPE & _STACK_CLASSNAME::Top(int which)
{
  if(Empty() || which>=Size()) COREDUMP; 
  return operator[](Count()-1-which);
}

inline void _STACK_CLASSNAME::Pop(void)
{
  if(Empty()) COREDUMP;
  Remove(Size()-1);
}
