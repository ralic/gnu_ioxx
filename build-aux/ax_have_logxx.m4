#--------------------------------------------------------
# AX_HAVE_LOGXX([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#--------------------------------------------------------

AC_DEFUN([AX_HAVE_LOGXX], [dnl
  AC_CHECK_HEADER([logxx.hpp], [$1], [$2])
])dnl


