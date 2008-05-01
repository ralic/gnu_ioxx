# ~/test/Makefile.am

AM_CPPFLAGS = -I$(top_srcdir)/include
AM_LDFLAGS  = -lboost_unit_test_framework${BOOST_SUFFIX}

TESTS =                         \
  iovec_is_valid_range          \
  schedule			\
  socket			\
  demux				\
  dispatch			\
  dns

check_PROGRAMS = ${TESTS}

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
schedule_SOURCES = schedule.cpp
socket_SOURCES = socket.cpp
dispatch_SOURCES = dispatch.cpp
demux_SOURCES = demux.cpp
dns_SOURCES = dns.cpp

MAINTAINERCLEANFILES = Makefile.in
