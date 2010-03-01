dnl Configure the ioxx library.
dnl
dnl This file is free software; as a special exception the author gives
dnl unlimited permission to copy and/or distribute it, with or without
dnl modifications, as long as this notice is preserved.
dnl
dnl This program is distributed in the hope that it will be useful, but
dnl WITHOUT ANY WARRANTY, to the extent permitted by law; without even the
dnl implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

AC_INIT([ioxx], m4_esyscmd([build-aux/git-version-gen .tarball-version]),
	[simons@cryp.to], [ioxx], [http://www.nongnu.org/ioxx/])
AC_CONFIG_SRCDIR([include/ioxx.hpp])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror 1.11 gnu std-options filename-length-max=99])
AC_CONFIG_MACRO_DIR([build-aux])
AC_MSG_NOTICE([$PACKAGE_STRING])

AC_PROG_CXX
AC_LANG([C++])

dnl ----- check for boost -----

AC_ARG_VAR([BOOST_SUFFIX], [Suffix to append to Boost link library names on this platform.])
AC_CHECK_HEADER([boost/version.hpp], [], [AC_MSG_FAILURE([please install boost.org libraries])])
AX_HAVE_BOOST_UNIT_TEST_FRAMEWORK

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
AC_ARG_ENABLE(
  [internal-docs],
  [AC_HELP_STRING([--enable-internal-docs], [Include internal classes in doxygen documentation.])],
  [case "${enableval}" in
     yes|no)
       ;;
     *)
       AC_MSG_ERROR([bad value ${enableval} for feature --enable-internal-docs])
       ;;
   esac],
   [enable_internal_docs="no"])
AC_SUBST([IOXX_INTERNAL_DOCS], [${enable_internal_docs}])

dnl ----- write results -----

AC_SUBST([CONFIG_STATUS_DEPENDENCIES], ['$(top_srcdir)/build-aux/config.h.in $(top_srcdir)/doc/Doxyfile.in'])
AC_CONFIG_HEADER([build-aux/config.h])
AX_PREFIX_CONFIG_H([include/ioxx/detail/config.hpp])
AC_CONFIG_FILES([Makefile include/Makefile test/Makefile doc/Makefile doc/Doxyfile])
AC_OUTPUT

dnl Responsibility shifts to the user after this point.

echo ""
echo "Optional features are configured as follows:"
echo ""
echo "    epoll(7) support ........... ${enable_epoll}"
echo "    epoll_pwait(2) support ..... ${enable_epoll_pwait}"
echo "    poll(2) support ............ ${enable_poll}"
echo "    ppoll(2) support ........... ${enable_ppoll}"
echo "    select(2) support .......... ${enable_select}"
echo "    pselect(2) support ......... ${enable_pselect}"
echo "    ADNS support ............... ${enable_adns}"
echo "    logxx support .............. ${enable_logging}"
echo "${ECHO_N}" "    doxygen support............. "; if test "${DOXYGEN}" != ":"; then echo "yes"; else echo "no"; fi
echo "    generate internal docs ..... ${enable_internal_docs}"
echo ""
