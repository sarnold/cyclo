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

// This is a quick and ugly list.  Note that the Remove() method calls
// the object's destructor

// NOTE: _LIST_CLASSTYPE should be a text only single token.  Use typedefs.

#ifndef COREDUMP
#define COREDUMP  *((char*)0)=0  // causes a segmentation fault (only under UNIX)
#endif

#ifndef _LIST_CLASSNAME
#error "_LIST_CLASSNAME must be defined"
#endif

#ifndef _LIST_CLASSTYPE
#error "_LIST_CLASSTYPE must be defined"
#endif

class _LIST_CLASSNAME
{
  _LIST_CLASSTYPE **list;
  int members;

public:
  _LIST_CLASSNAME() { list=0; members=0; }
  ~_LIST_CLASSNAME();
  int Add(_LIST_CLASSTYPE *);
  inline void Remove(int member);
  int Count() { return members; }
  int Empty(void) { return !Count(); }
  _LIST_CLASSTYPE & operator[](int);
};

// By using inline, it won't complain about the functions being instantiated
// in every module this file is included in.

inline _LIST_CLASSNAME::~_LIST_CLASSNAME()
{
  while(!Empty())
    Remove(Count()-1);
}

inline int _LIST_CLASSNAME::Add(_LIST_CLASSTYPE *element)
{
  if(Empty())
    {
      list=(_LIST_CLASSTYPE **)malloc(sizeof(_LIST_CLASSTYPE *));
      if(!list)
        return 0; // failed
      list[0]=element;
      members=1;
      return 1; // succeeded
    }
  _LIST_CLASSTYPE **newlist=(_LIST_CLASSTYPE **)realloc(list, sizeof(_LIST_CLASSTYPE *)*(members+1));
  if(!newlist)
    return 0; // failed
  list=newlist;
  list[members]=element;
  members++;
  return 1; // succeeded
}

inline void _LIST_CLASSNAME::Remove(int member)
{
  if(member<0 || member>=Count())
    COREDUMP; // Give the user debugging info

  delete list[member];
  memmove(&list[member], &list[member+1], Count()-member-1);
  members--;

  if(!Empty())
    {
      // attempt to shrink list size - it doesn't matter if we fail
      _LIST_CLASSTYPE **newlist=(_LIST_CLASSTYPE **)realloc(list, sizeof(_LIST_CLASSTYPE *)*members);
      list=newlist?newlist:list;
    }
  else
    {
      free(list);
      list=0;
    }
}

inline _LIST_CLASSTYPE & _LIST_CLASSNAME::operator[](int member)
{
  if(member<0 || member>=Count())
    COREDUMP; // Give the user debugging info

  return *list[member];
}
