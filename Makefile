# Build the scheduler library.

CXX		= g++
AR		= ar
RANLIB		= ranlib

CPPFLAGS	=
CXXFLAGS	= -Wall
LDFLAGS		=

OBJS		= scheduler.o

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

all:		libscheduler.a test

libscheduler.a:	$(OBJS)
	@rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

test:		test.o libscheduler.a
	$(CXX) $(LDFLAGS) -o $@ test.o libscheduler.a

clean::
	rm -f libscheduler.a $(OBJS)
	rm -f test test.o

# Dependencies

test.o:	scheduler.hpp
