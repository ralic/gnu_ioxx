AM_CPPFLAGS = -I$(top_srcdir)/include

TESTS =                         \
  iovec_is_valid_range          \
  timeout

check_PROGRAMS = ${TESTS}

iovec_is_valid_range_SOURCES = iovec-is-valid-range.cpp
iovec_is_valid_range_LDFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)

timeout_SOURCES = timeout.cpp
timeout_CPPFLAGS = $(BOOST_CPPFLAGS) $(AM_CPPFLAGS)
