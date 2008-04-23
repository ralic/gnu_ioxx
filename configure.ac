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

AC_MSG_CHECKING([whether boost_unit_test_framework is available])
LIBS="${LIBS} -lboost_unit_test_framework${BOOST_SUFFIX}"
AC_LINK_IFELSE([AC_LANG_CALL([], [exit])],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_FAILURE([no])])

AC_MSG_CHECKING([whether this platform supports epoll(7)])
AC_PREPROC_IFELSE(
  [#include <sys/epoll.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION (2,5,45)
#  error linux kernel version is too old
#endif],
  [AC_DEFINE(IOXX_HAVE_EPOLL, 1, [Have epoll(7).]) AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])])

AC_MSG_CHECKING([whether this platform supports select(2)])
AC_PREPROC_IFELSE(
  [#include <sys/select.h>],
  [AC_DEFINE(IOXX_HAVE_SELECT, 1, [Have select(2).]) AC_MSG_RESULT([yes])],
  [AC_MSG_RESULT([no])])

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT
