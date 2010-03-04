# Build the scheduler test program.
#
# Copyright (c) 2001-2010 Peter Simons <simons@cryp.to>
#
# Copying and distribution of this file, with or without modification, are
# permitted in any medium without royalty provided the copyright notice
# and this notice are preserved. This file is offered as-is, without any
# warranty.

CXX		= g++
CPPFLAGS	=
CXXFLAGS	= -O3 -pipe -Wall
LDFLAGS		=

.cc.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

test:		test.o
	$(CXX) $(LDFLAGS) -o $@ $<

clean::
	rm -f test test.o

# Dependencies

test.o: scheduler.hh pollvector.hh
