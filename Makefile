# Build the scheduler test program.

CXX		= g++
AR		= ar
RANLIB		= ranlib

CPPFLAGS	=
CXXFLAGS	= -O3 -pipe -Wall
LDFLAGS		=

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

test:		test.o
	$(CXX) $(LDFLAGS) -o $@ test.o

clean::
	rm -f test test.o

# Dependencies

test.o: scheduler.hh pollvector.hh
