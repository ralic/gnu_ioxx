AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS =                         \
  iovec_is_valid_range          \
  scheduler			\
  socket			\
  demux				\
  dispatch

check_PROGRAMS = ${TESTS}

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
iovec_is_valid_range_LDFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

scheduler_SOURCES = scheduler.cpp
scheduler_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

socket_SOURCES = socket.cpp
socket_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

dispatch_SOURCES = dispatch.cpp
dispatch_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

demux_SOURCES = demux.cpp
demux_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

MAINTAINERCLEANFILES = Makefile.in
