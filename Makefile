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

HEADERS = tokens.h lib.h list.h stack.h

# Compilation configuration
# your C compiler must understand ANSI std C
CC = gcc
# and you'll need a C++ compiler as well
CCPLUS = g++
# flex is not absolutely required, but some versions of 
# lex run out of space while processing the input file
LEX = flex
#LEX = lex

DEBUG = # -g
OPTIM = -O
LEXOPTS = -t
LDFLAGS = $(DEBUG)
CFLAGS = $(OPTIM) $(DEBUG) #-Wall
LIBS = -ll

all: mcstrip cyclo

mcstrip: mcstrip.o strerror.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o mcstrip mcstrip.o strerror.o $(LIBS)

mcstrip.o: mcstrip.l
	$(LEX) $(LEXOPTS) mcstrip.l > mcstrip.c 
	$(CC) $(LDFLAGS) $(CFLAGS) -c mcstrip.c
	rm mcstrip.c

strerror.o: strerror.c
	$(CC) $(LDFLAGS) $(CFLAGS) -c strerror.c

cyclo: main.o scan.o lib.o
	$(CCPLUS) $(LDFLAGS) -o $@ main.o scan.o lib.o $(LIBS)

main.o: main.C $(HEADERS)
	#$(CCPLUS) $(CFLAGS) -DNEEDGETOPTDEFS -c main.C
	$(CCPLUS) $(CFLAGS) -c main.C

lib.o: lib.C $(HEADERS)
	$(CCPLUS) $(CFLAGS) -c lib.C

scan.o: scan.l $(HEADERS)
	$(LEX) $(LEXOPTS) scan.l > scan.c
	$(CC) $(CFLAGS) -c scan.c
	rm scan.c

clean:
	rm -f cyclo mcstrip core *.o *~ 

veryclean: clean
	rm -f mcstrip.c scan.c
