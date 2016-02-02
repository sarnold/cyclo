Cyclo - the cyclomatic complexity tool for C
============================================

.. image:: https://img.shields.io/badge/license-GPL2-green.svg?dummy
   :target: https://github.com/sarnold/cyclo/blob/master/LICENSE

.. image:: https://badge.fury.io/gh/sarnold%2Fcyclo.svg?dummy
   :target: https://badge.fury.io/gh/sarnold%2Fcyclo

.. image:: https://travis-ci.org/sarnold/cyclo.svg?branch=master
   :target: https://travis-ci.org/sarnold/cyclo

.. image:: http://githubbadges.herokuapp.com/sarnold/cyclo/issues.svg?style=flat-square&dummy
   :target: https://github.com/sarnold/cyclo/issues

Original version is copyright (c) 1993 Roger Binns

Updates are copyright (c) 2016 Stephen L Arnold

These tools were produced by Roger Binns for a fourth year project as part of
a computer science degree, for the Computer Science department, Brunel
University, Uxbridge, Middlesex UB8 3PH, United Kingdom.

This software is provided in good faith, having been developed by Brunel
University students as part of their normal course work.  It should not be
assumed that Brunel has any rights of ownership, and the University cannot
accept any liability for its subsequent use.  It is a condition of any such
use that the user idemnifies the University against any claim (including
third party claims) arising therefrom.

This software is provided 'as is'.  If you have any doubts that could lead
to legal matters, then don't use it.


To compile::

        $ make
  
To run the test::

        $ make -f Makefile.test test

For instructions on use, and other info::

        $ nroff -man cyclo.1 | more

For an example of using cyclo on all C source files in the current dir
to create a simple text complexity report, see the file cyclo.mk.inc
and include it in your static makefile (or adapt it to Makefile.ac).

History and Authors
-------------------

Christopher Lott made the package slightly more portable sometime in 1994
by cleaning up the Makefile and by adding .h/.c files for the strerror
function.

Updated June 1996 by Bob Statsinger.  These changes are
copyright (c) 1996 1996 B.S. Software.  Changes made by
Bob Statsinger <rstats@interbase.borland.com>:

- In addition to C++ style function declarations
(e.g. cname::foo()), ANSI or K&R style declarations
are now properly handled.

- if  else if....else if..... else  is properly handled

- cyclo -c now prints the total number of excutable C statements 
in the source, along with the mccabe numbers  so you can kinda get 
a complexity ratio as mccabe number/# statements

- I've disabled some of the parser failure checking - IMHO 
what's important here is the cyclomatic number, not syntax 
correctness. YMMV.

- the file 'mccabe.example' is a sample shell script to run mcstrip
  and cyclo.  It will need to be tailored for your site.

- test-input.c is an example test file.


Updated further in August 1996 by Christopher Lott to fix some
small glitches in the code that g++ picked up.  If my fixes
cause further problems on other compilers, please let me know.

Beware the defs for getopt in main.C.  You can turn them off by
editing the Makefile (search for NEEDGETOPTDEFS).


Updated even further and packaged for Gentoo in Sep 2013 by Stephen Arnold.
Added flex 3.x noyywrap option in Aug 2015, and pushed to github in Oct 2015.

Unlike cccc, cyclo is intended to be incorporated as a make target (using
cpp, mcstrip, and cyclo) so you'll need to collect the output yourself.

Stay tuned for some examples...

