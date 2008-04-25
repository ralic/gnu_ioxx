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

AC_CACHE_CHECK(
  [whether this platform supports epoll],
  [ax_cv_have_epoll],
  [AC_PREPROC_IFELSE([dnl
#include <sys/epoll.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION (2,5,45)
#  error linux kernel version is too old
#endif], [ax_cv_have_epoll=yes], [ax_cv_have_epoll=no])])

if test "$ax_cv_have_epoll" = "yes"; then
  AC_DEFINE([IOXX_HAVE_EPOLL], 1, [This platform supports epoll(7).])
fi

AC_CACHE_CHECK(
  [whether this platform supports select()],
  [ax_cv_have_select],
  [AC_PREPROC_IFELSE(
    [#include <sys/select.h>],
    [ax_cv_have_select=yes],
    [ax_cv_have_select=no])])

if test "$ax_cv_have_select" = "yes"; then
  AC_DEFINE([IOXX_HAVE_SELECT], 1, [This platform supports select(2).])
fi

AC_CACHE_CHECK(
  [whether this platform supports poll()],
  [ax_cv_have_poll],
  [AC_PREPROC_IFELSE(
    [#include <sys/poll.h>],
    [ax_cv_have_poll=yes],
    [ax_cv_have_poll=no])])

if test "$ax_cv_have_poll" = "yes"; then
  AC_DEFINE([IOXX_HAVE_POLL], 1, [This platform supports poll(2).])
fi

AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_OUTPUT
