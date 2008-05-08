dnl ~/configure.ac -- see http://ioxx.cryp.to/

AC_INIT([ioxx], [0.1], [simons@cryp.to])
AC_CONFIG_SRCDIR([include/ioxx.hpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall gnu std-options filename-length-max=99])
AC_CONFIG_MACRO_DIR([build-aux])

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

AC_PROG_CXX
AC_LANG([C++])

dnl ----- check for boost -----

AC_ARG_VAR([BOOST_SUFFIX], [Suffix to append to Boost link library names on this platform.])
AC_CHECK_HEADERS(
  [boost/shared_ptr.hpp boost/assert.hpp boost/range.hpp boost/function.hpp boost/test/unit_test.hpp],
  [],
  [AC_MSG_FAILURE([please install boost.org libraries])])
orig_libs="${LIBS}"
AC_CHECK_LIB([boost_unit_test_framework${BOOST_SUFFIX}], [exit], [],
  [AC_MSG_FAILURE([please install boost.org libraries])])
LIBS="${orig_libs}"

dnl ----- check for logxx -----

IOXX_ENABLE_FEATURE([logging],     [AX_HAVE_LOGXX],       [Support the http://logxx.cryp.to/ library.])

dnl ----- check for various i/o probes -----

IOXX_ENABLE_FEATURE([epoll],       [AX_HAVE_EPOLL],       [Support epoll(7) on this platform.])
IOXX_ENABLE_FEATURE([epoll-pwait], [AX_HAVE_EPOLL_PWAIT], [Support epoll_pwait(2) on this platform.])
IOXX_ENABLE_FEATURE([poll],        [AX_HAVE_POLL],        [Support poll(2) on this platform.])
IOXX_ENABLE_FEATURE([ppoll],       [AX_HAVE_PPOLL],       [Support ppoll(2) on this platform.])
IOXX_ENABLE_FEATURE([select],      [AX_HAVE_SELECT],      [Support select(2) on this platform.])
IOXX_ENABLE_FEATURE([pselect],     [AX_HAVE_PSELECT],     [Support pselect(2) on this platform.])

dnl ----- check for adns -----

IOXX_ENABLE_FEATURE([adns],        [AX_HAVE_ADNS],        [Support GNU ADNS on this platform.])

dnl ----- doxygen -----

AX_WITH_PROG([DOXYGEN], [doxygen], [no])
AS_IF([test "${DOXYGEN}" = "no"], [DOXYGEN=":"], [])
AX_WITH_PROG([DOT], [dot], [no])
AS_IF([test "${DOT}" != "no"], [AC_SUBST([HAVE_DOT], [yes])], [AC_SUBST([HAVE_DOT], [no])])
AX_WITH_PERL([:])

dnl ----- write results -----

AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/include/ioxx/detail/config.hpp.in $(top_srcdir)/doc/Doxyfile.in'])
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
echo "${ECHO_N}" "    epoll_pwait(2) support ..... "; if test "${IOXX_HAVE_EPOLL_PWAIT}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    poll(2) support ............ "; if test "${IOXX_HAVE_POLL}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    ppoll(2) support ........... "; if test "${IOXX_HAVE_PPOLL}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    select(2) support .......... "; if test "${IOXX_HAVE_SELECT}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    pselect(2) support ......... "; if test "${IOXX_HAVE_PSELECT}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    ADNS support ............... "; if test "${IOXX_HAVE_ADNS}" = 1; then echo "yes"; else echo "no"; fi
echo "${ECHO_N}" "    doxygen support............. "; if test "${DOXYGEN}" != ":"; then echo "yes"; else echo "no"; fi
echo ""
