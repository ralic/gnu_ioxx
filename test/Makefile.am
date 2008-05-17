# ~/test/Makefile.am

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include @BOOST_UNIT_TEST_FRAMEWORK_CPPFLAGS@
LDADD = -lboost_unit_test_framework$(BOOST_SUFFIX)

TESTS =                         \
  iovec_is_valid_range          \
  schedule			\
  socket			\
  demux				\
  dns				\
  core

check_PROGRAMS = ${TESTS}
noinst_HEADERS = daytime.hpp echo.hpp

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
schedule_SOURCES = schedule.cpp
socket_SOURCES = socket.cpp
demux_SOURCES = demux.cpp
dns_SOURCES = dns.cpp
core_SOURCES = core.cpp

MAINTAINERCLEANFILES = Makefile.in
