# Build the scheduler library.

CXX		= g++
AR		= ar
RANLIB		= ranlib

CPPFLAGS	=
CXXFLAGS	= -O -finline-functions -pipe -Wall -pedantic
LDFLAGS		= -s

OBJS		= scheduler.o

.cpp.o:
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $<

libscheduler.a:		$(OBJS)
	@rm -f $@
	$(AR) cr $@ $(OBJS)
	$(RANLIB) $@

test:			test.o libscheduler.a
	$(CXX) $(LDFLAGS) -o $@ test.o libscheduler.a

clean::
	rm -f libscheduler.a $(OBJS)
	rm -f test test.o

# Dependencies

scheduler.o: scheduler.hpp pollvector.hpp pollvector.hpp
test.o: scheduler.hpp pollvector.hpp
