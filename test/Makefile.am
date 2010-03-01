# Build the ioxx library.
#
# This file is free software; as a special exception the author gives
# unlimited permission to copy and/or distribute it, with or without
# modifications, as long as this notice is preserved.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include $(BOOST_UNIT_TEST_FRAMEWORK_CPPFLAGS)
LDADD = -lboost_unit_test_framework$(BOOST_SUFFIX)

TESTS =                         \
  iovec_is_valid_range          \
  schedule			\
  socket			\
  demux				\
  dns				\
  inetd

check_PROGRAMS = ${TESTS}
noinst_HEADERS = daytime.hpp echo.hpp

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
schedule_SOURCES = schedule.cpp
socket_SOURCES = socket.cpp
demux_SOURCES = demux.cpp
dns_SOURCES = dns.cpp
inetd_SOURCES = inetd.cpp

MAINTAINERCLEANFILES = Makefile.in
