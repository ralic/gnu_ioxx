dnl configure the ioxx libraries

AC_INIT([ioxx], [0.1], [simons@cryp.to])
AM_INIT_AUTOMAKE([-Wall foreign std-options filename-length-max=99])
AC_CONFIG_SRCDIR([include/ioxx.hpp])
AC_COPYRIGHT([Copyright (c) 2008 by Peter Simons <simons@cryp.to>])

AC_LANG([C++])
AC_PROG_LIBTOOL

AC_ARG_VAR([BOOST_SUFFIX], [library suffix used by boost libraries on this platform])

AC_MSG_CHECKING([whether boost/version.hpp is recent enough])
AC_PREPROC_IFELSE(
  [#include "boost/version.hpp"
#if !defined(BOOST_VERSION) || BOOST_VERSION < 103500
#  error boost version is too old
#endif],
  [AC_MSG_RESULT([>= 1.35])],
  [AC_MSG_FAILURE([boost version is too old])])

AC_MSG_CHECKING([whether boost_system is available])
LIBS="${LIBS} -lboost_system${BOOST_SUFFIX}"
AC_LINK_IFELSE([AC_LANG_CALL([], [exit])],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_FAILURE([no])])

# AC_CHECK_LIB([boost_thread${BOOST_SUFFIX}], [exit])
# AC_CHECK_LIB([boost_system${BOOST_SUFFIX}], [exit])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT
