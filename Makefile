# (c) 1993 Roger Binns
#
# These tools were produced by Roger Binns for a fourth year project as part of
# a computer science degree, for the Computer Science department, Brunel
# University, Uxbridge, Middlesex UB8 3PH, United Kingdom.
#
# This software is provided in good faith, having been developed by Brunel
# University students as part of their normal course work.  It should not be
# assumed that Brunel has any rights of ownership, and the University cannot
# accept any liability for its subsequent use.  It is a condition of any such
# use that the user idemnifies the University against any claim (including
# third party claims) arising therefrom.
#
# Updated 1996 by R. Statsinger, C. Lott
# Updated 2015 by Stephen L Arnold

# the actual build stuff starts here
HEADERS = tokens.h lib.h list.h stack.h

# Compilation configuration
CC ?= gcc
CXX ?= g++

# we need flex to work around some limitations of certain lex versions
LEX = flex

DBG ?= -g

OPTIM	?= -O2
LFLAGS	= -t
LDFLAGS	= $(DBG)
CFLAGS	= $(OPTIM) $(DBG) #-Wall
LDLIBS	= -lstdc++ -lfl

.SUFFIXES:
.SUFFIXES: .l .c .C .o

GENSRCS	= mcstrip.c scan.c
BLDSRCS	= lib.C main.C strerror.c
MCSOBJS	= mcstrip.o strerror.o
CYCOBJS	= main.o lib.o
SCNOBJS	= scan.o

.INTERMEDIATE: $(GENSRCS)
.PHONY: all
.DEFAULT: all

all: mcstrip cyclo

mcstrip.c: mcstrip.l
	$(LEX) $(LFLAGS) $^ > $@

scan.c: scan.l
	$(LEX) $(LFLAGS) $^ > $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $^

%.o: %.C
	$(CXX) $(CPPFLAGS) $(CFLAGS) -o $@ -c $^

mcstrip: $(MCSOBJS)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

cyclo: $(CYCOBJS) $(SCNOBJS)
	$(CXX) -o $@ $(CFLAGS) $(LDFLAGS) $^ $(LDLIBS)

# not sure if we care about -DNEEDGETOPTDEFS or not...

clean:
	$(RM) cyclo mcstrip core *.o *~

cleanall: clean
	$(RM) *.pre *.mets *.strp $(GENSRCS)


