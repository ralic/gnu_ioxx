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
#if !defined(BOOST_VERSION) || BOOST_VERSION < 103401
#  error boost version is too old
#endif],
  [AC_MSG_RESULT([>= 1.34.1])],
  [AC_MSG_FAILURE([boost version is too old])])

orig_libs="${LIBS}"
AC_MSG_CHECKING([whether boost_unit_test_framework is available])
LIBS="${LIBS} -lboost_unit_test_framework${BOOST_SUFFIX}"
AC_LINK_IFELSE([AC_LANG_CALL([], [exit])],
  [AC_MSG_RESULT([yes])],
  [AC_MSG_FAILURE([no])])
LIBS="${orig_libs}"

dnl ----- check for epoll -----

AX_HAVE_EPOLL([AX_CONFIG_FEATURE_ENABLE(epoll)], [AX_CONFIG_FEATURE_DISABLE(epoll)])
AX_CONFIG_FEATURE(
  [epoll], [This platform supports epoll(7)],
  [HAVE_EPOLL], [This platform supports epoll(7).],
  [AC_SUBST([IOXX_HAVE_EPOLL], [1])], [AC_SUBST([IOXX_HAVE_EPOLL], [0])])
AX_HAVE_EPOLL_PWAIT

dnl ----- check for poll -----

AX_HAVE_POLL([AX_CONFIG_FEATURE_ENABLE(poll)], [AX_CONFIG_FEATURE_DISABLE(poll)])
AX_CONFIG_FEATURE(
  [poll], [This platform supports poll(2)],
  [HAVE_POLL], [This platform supports poll(2).],
  [AC_SUBST([IOXX_HAVE_POLL], [1])], [AC_SUBST([IOXX_HAVE_POLL], [0])])
AX_HAVE_PPOLL

dnl ----- check for select -----

AX_HAVE_SELECT([AX_CONFIG_FEATURE_ENABLE(select)], [AX_CONFIG_FEATURE_DISABLE(select)])
AX_CONFIG_FEATURE(
  [select], [This platform supports select(2)],
  [HAVE_SELECT], [This platform supports select().],
  [AC_SUBST([IOXX_HAVE_SELECT], [1])], [AC_SUBST([IOXX_HAVE_SELECT], [0])])
AX_HAVE_PSELECT

dnl ----- check for adns -----

AC_CHECK_HEADER([adns.h])
AS_IF([test "${ac_cv_header_adns_h}" = yes],
  [AX_CONFIG_FEATURE_ENABLE(adns)],
  [AX_CONFIG_FEATURE_DISABLE(adns)])
AX_CONFIG_FEATURE(
  adns, [build with GNU ADNS support],
  IOXX_HAVE_ADNS, [This host has GNU ADNS installed.],
  [AC_SUBST([IOXX_HAVE_ADNS], [1])], [AC_SUBST([IOXX_HAVE_ADNS], [0])])
AS_IF([test "${IOXX_HAVE_ADNS}" = 1], [AC_SEARCH_LIBS([adns_init], [adns])], [])

dnl ----- doxygen -----

AX_WITH_PROG([DOXYGEN], [doxygen], [], [:])
AX_WITH_PROG([DOT], [dot], [], [:])
AS_IF([test "${ac_cv_path_DOT}" != ":"], AC_SUBST([HAVE_DOT], [yes]), AC_SUBST([HAVE_DOT], [no]))
AX_WITH_PERL([], [:])

dnl ----- write results -----

AC_CONFIG_HEADERS([build-aux/config.h])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([include/Makefile])
AC_CONFIG_FILES([include/ioxx/detail/config.hpp])
AC_CONFIG_FILES([src/Makefile])
AC_CONFIG_FILES([test/Makefile])
AC_CONFIG_FILES([doc/Makefile])
AC_CONFIG_FILES([doc/Doxyfile])
AC_OUTPUT

dnl Responsibility shifts to the user after this point.

echo ""
echo "Optional features are configured as follows:"
echo ""
echo "${ECHO_N}" "    epoll(7) support ........... "; if test "${IOXX_HAVE_EPOLL}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    poll(2) support ............ "; if test "${IOXX_HAVE_POLL}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    select(2) support .......... "; if test "${IOXX_HAVE_SELECT}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    ADNS support ............... "; if test "${IOXX_HAVE_ADNS}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    doxygen support............. "; if test "${ac_cv_path_DOXYGEN}" != ":"; then echo "yes"; else echo "no"; fi
echo ""
