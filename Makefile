# Build the scheduler library.

CXX		= g++
AR		= ar
RANLIB		= ranlib

CPPFLAGS	=
CXXFLAGS	= -finline-functions -pipe -Wall -pedantic
LDFLAGS		=

OBJS		=

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

all:		test

libscheduler.a:	$(OBJS)
	@rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

test:		test.o
	$(CXX) $(LDFLAGS) -o $@ test.o

clean::
	rm -f libscheduler.a $(OBJS)
	rm -f test test.o

# Dependencies

test.o: scheduler.hpp pollvector.hpp
