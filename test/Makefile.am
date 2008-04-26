AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS =                         \
  iovec_is_valid_range          \
  scheduler			\
  probe				\
  demux

check_PROGRAMS = ${TESTS}

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
iovec_is_valid_range_LDFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

scheduler_SOURCES = scheduler.cpp
scheduler_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

probe_SOURCES = probe.cpp
probe_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

demux_SOURCES = demux.cpp
demux_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

MAINTAINERCLEANFILES = Makefile.in
