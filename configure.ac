dnl configure the ioxx libraries

m4_define([ioxx_version_code],   [000100])
m4_define([ioxx_version_string], [0.1])

AC_INIT([ioxx], [ioxx_version_string], [simons@cryp.to])
AC_CONFIG_SRCDIR([include/ioxx.hpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall foreign no-define std-options filename-length-max=99])
AC_COPYRIGHT([Copyright (c) 2008 by Peter Simons <simons@cryp.to>])
AC_CONFIG_MACRO_DIR([build-aux])

AC_DEFINE([IOXX_VERSION_CODE],   [ioxx_version_code],     [Bit-encoded version information.])
AC_DEFINE([IOXX_VERSION_STRING], ["ioxx_version_string"], [Human-readable version information.])
AC_SUBST([IOXX_VERSION_STRING], [$PACKAGE_VERSION])

AC_LANG([C++])
AC_PROG_LIBTOOL

dnl ----- check for boost -----

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

dnl ----- check for epoll -----

AC_CACHE_CHECK(
  [whether this platform supports epoll],
  [ax_cv_have_epoll],
  [AC_PREPROC_IFELSE([dnl
#include <sys/epoll.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION (2,5,45)
#  error linux kernel version is too old
#endif], [ax_cv_have_epoll=yes], [ax_cv_have_epoll=no])])

AS_IF([test "$ax_cv_have_epoll" = yes],[AX_CONFIG_FEATURE_ENABLE(epoll)],[])

AX_CONFIG_FEATURE(
  epoll, [This platform supports epoll(7)],
  IOXX_HAVE_EPOLL, [This platform supports epoll(7).]
  [ioxx_have_epoll=yes], [ioxx_have_epoll=no])

dnl ----- check for poll -----

AC_CACHE_CHECK(
  [whether this platform supports poll()],
  [ax_cv_have_poll],
  [AC_PREPROC_IFELSE(
    [#include <sys/poll.h>],
    [ax_cv_have_poll=yes],
    [ax_cv_have_poll=no])])

AS_IF([test "$ax_cv_have_poll" = yes],[AX_CONFIG_FEATURE_ENABLE(poll)],[])

AX_CONFIG_FEATURE(
  poll, [This platform supports poll(2)],
  IOXX_HAVE_POLL, [This platform supports poll(2).]
  [ioxx_have_poll=yes], [ioxx_have_poll=no])

dnl ----- check for select -----

AC_CACHE_CHECK(
  [whether this platform supports select()],
  [ax_cv_have_select],
  [AC_PREPROC_IFELSE(
    [#include <sys/select.h>],
    [ax_cv_have_select=yes],
    [ax_cv_have_select=no])])

AS_IF([test "$ax_cv_have_select" = yes],[AX_CONFIG_FEATURE_ENABLE(select)],[])

AX_CONFIG_FEATURE(
  select, [This platform supports select(2)],
  IOXX_HAVE_SELECT, [This platform supports select(2).]
  [ioxx_have_select=yes], [ioxx_have_select=no])

dnl ----- check for adns -----

AC_CACHE_CHECK(
  [whether we have the ADNS library],
  [ax_cv_have_adns],
  [AC_PREPROC_IFELSE(
    [#include <adns.h>],
    [ax_cv_have_adns=yes],
    [ax_cv_have_adns=no])])

AS_IF([test "$ax_cv_have_adns" = yes],[AX_CONFIG_FEATURE_ENABLE(adns)],[])

AX_CONFIG_FEATURE(
  adns, [This host has ADNS installed],
  IOXX_HAVE_ADNS, [This host has ADNS installed.]
  [ioxx_have_adns=yes], [ioxx_have_adns=no])

dnl ----- write results -----

AC_CONFIG_HEADER([include/ioxx/config.hpp])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/Doxyfile])
AC_OUTPUT
