# (c) 1993 Roger Binns
# changes (c) 2015 Stephen L Arnold
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

# don't forget to update the version!
MAJOR_VERSION = 2
MINOR_VERSION = 1
PATCH_VERSION = 0
FULL_VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(PATCH_VERSION)

PV = $(FULL_VERSION)

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
MYLDFLAGS	= $(LDFLAGS) $(DBG)
MYCFLAGS	= -D_DEFAULT_SOURCE $(OPTIM) $(DBG) $(CFLAGS) -Wall
MYCXXFLAGS	= $(MYCFLAGS)
LDADD	= -lstdc++ -lfl

.SUFFIXES:
.SUFFIXES: .l .c .C .o

GENSRCS	= mcstrip.c scan.c
BLDSRCS	= lib.C main.C
MCSOBJS	= mcstrip.o
CYCOBJS	= main.o lib.o
SCNOBJS	= scan.o

.INTERMEDIATE: $(GENSRCS)
.PHONY: all
.DEFAULT: all

PGMS	= mcstrip cyclo

all: $(PGMS)

mcstrip.c: mcstrip.l
	$(LEX) $(LFLAGS) $^ > $@

scan.c: scan.l
	$(LEX) $(LFLAGS) $^ > $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(MYCFLAGS) -o $@ -c $^

%.o: %.C
	$(CXX) $(CPPFLAGS) $(MYCFLAGS) -o $@ -c $^

mcstrip: $(MCSOBJS)
	$(CC) -o $@ $(MYCFLAGS) $(MYLDFLAGS) $^ $(LDADD)

cyclo: $(CYCOBJS) $(SCNOBJS)
	$(CXX) -o $@ $(MYCFLAGS) $(MYLDFLAGS) $^ $(LDADD)

# not sure if we care about -DNEEDGETOPTDEFS or not...

PREFIX		= /usr/local
EPREFIX		= $(PREFIX)
DATADIR		= $(PREFIX)/share
MANPREFIX	= $(PREFIX)/share

BINDIR		= $(DESTDIR)$(EPREFIX)/bin
MANDIR		= $(DESTDIR)$(MANPREFIX)/man
DOCDIR		= $(DESTDIR)$(DATADIR)/doc/cyclo-$(PV)

INSTALL		= install -p
INSTALL_PROGRAM	= $(INSTALL)
INSTALL_DATA	= $(INSTALL) -m 644

make-install-dirs:
	mkdir -p $(BINDIR)
	mkdir -p $(MANDIR)/man0
	mkdir -p $(MANDIR)/man1
	mkdir -p $(DOCDIR)

uninstall:
	$(RM) $(BINDIR)/cyclo
	$(RM) $(BINDIR)/mcstrip
	rm -rf $(DOCDIR)
	$(RM) $(MANDIR)/man0/cyclo.0.gz
	$(RM) $(MANDIR)/man1/cyclo.1.gz
	$(RM) $(MANDIR)/man1/mcstrip.1.gz

install: install-targets

install-targets: make-install-dirs
	$(INSTALL_PROGRAM) -t $(BINDIR) $(PGMS)
	$(INSTALL_DATA) cyclo.0 $(MANDIR)/man0
	$(INSTALL_DATA) cyclo.1 $(MANDIR)/man1
	$(INSTALL_DATA) mcstrip.1 $(MANDIR)/man1
	$(INSTALL_DATA) README.rst $(DOCDIR)
	$(INSTALL_DATA) mccabe.example $(DOCDIR)

clean:
	$(RM) cyclo mcstrip core *.o *~

cleanall: clean
	$(RM) $(GENSRCS)


