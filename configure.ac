dnl ~/configure.ac -- see http://ioxx.cryp.to/

AC_INIT([ioxx], [0.1], [simons@cryp.to])
AC_CONFIG_SRCDIR([include/ioxx.hpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall gnu std-options filename-length-max=99])
AC_CONFIG_MACRO_DIR([build-aux])

dnl AC_SUBST([IOXX_VERSION_STRING], [$PACKAGE_VERSION])

AC_COPYRIGHT([dnl
Copyright (C) 2008 Peter Simons <simons@cryp.to>

Ioxx is free software: you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

Ioxx is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Ioxx. If not, see <http://www.gnu.org/licenses/>.])


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
  IOXX_HAVE_EPOLL, [This platform supports epoll(7).],
  [ioxx_have_epoll=1], [ioxx_have_epoll=0])

AC_SUBST([IOXX_HAVE_EPOLL], [$ioxx_have_epoll])

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
  poll, [build with poll(2) support],
  IOXX_HAVE_POLL, [This platform supports poll(2).],
  [ioxx_have_poll=1], [ioxx_have_poll=0])

AC_SUBST([IOXX_HAVE_POLL], [$ioxx_have_poll])

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
  select, [build with select(2) support],
  IOXX_HAVE_SELECT, [This platform supports select(2).],
  [ioxx_have_select=1], [ioxx_have_select=0])

AC_SUBST([IOXX_HAVE_SELECT], [$ioxx_have_select])

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
  adns, [build with GNU ADNS support],
  IOXX_HAVE_ADNS, [This host has GNU ADNS installed.],
  [ioxx_have_adns=1], [ioxx_have_adns=0])

AC_SUBST([IOXX_HAVE_ADNS], [$ioxx_have_adns])

dnl ----- write results -----

AC_CONFIG_HEADERS([build-aux/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([include/ioxx/config.hpp])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/Doxyfile])
AC_OUTPUT

dnl Responsibility shifts to the user after this point.

echo ""
echo "Optional features are configured as follows:"
echo ""
echo "${ECHO_N}" "    epoll(7) support ........... "; if test "${ioxx_have_epoll}"; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    poll(2) support ............ "; if test "${ioxx_have_poll}"; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    select(2) support .......... "; if test "${ioxx_have_select}"; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    ADNS support ............... "; if test "${ioxx_have_adns}"; then echo "yes"; else echo "no"; fi
echo ""
