# SYNOPSIS
#
#   AX_HAVE_POLL([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#   AX_HAVE_PPOLL([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#

AC_DEFUN([AX_HAVE_POLL], [dnl
  AC_CHECK_HEADER([poll.h])
  AS_IF([test "${ac_cv_header_poll_h}" = yes],
    [AC_SEARCH_LIBS([poll], [])],
    [])
  AS_IF([test "${ac_cv_search_poll}" != no], [$1], [$2])
])

AC_DEFUN([AX_HAVE_PPOLL], [dnl
  AC_REQUIRE([AX_HAVE_POLL])
  AS_IF([test "${ac_cv_header_poll_h}" = yes],
    [AC_SEARCH_LIBS([ppoll], [])],
    [])
  AS_IF([test "${ac_cv_search_ppoll}" != no], [$1], [$2])
])
