# SYNOPSIS
#
#   AX_HAVE_SELECT([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#   AX_HAVE_PSELECT([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#

AC_DEFUN([AX_HAVE_SELECT], [dnl
  AC_CHECK_HEADER([sys/select.h])
  AS_IF([test "${ac_cv_header_sys_select_h}" = yes],
    [AC_SEARCH_LIBS([select], [])],
    [])
  AS_IF([test "${ac_cv_search_select}" != no], [$1], [$2])
])

AC_DEFUN([AX_HAVE_PSELECT], [dnl
  AC_REQUIRE([AX_HAVE_SELECT])
  AS_IF([test "${ac_cv_header_sys_select_h}" = yes],
    [AC_SEARCH_LIBS([pselect], [])],
    [])
  AS_IF([test "${ac_cv_search_pselect}" != no], [$1], [$2])
])
