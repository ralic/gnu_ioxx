# SYNOPSIS
#
#   AX_HAVE_EPOLL([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#   AX_HAVE_EPOLL_PWAIT([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#

AC_DEFUN([AX_HAVE_EPOLL], [dnl

AC_CHECK_HEADER([sys/epoll.h])

AS_IF([test "${ac_cv_header_sys_epoll_h}" = yes],
  [AC_CACHE_CHECK(
    [whether <linux/version.h> is recent enough to have epoll support],
    [ac_cv_linux_version_h_newer_2_5_45],
    [AC_PREPROC_IFELSE([dnl
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION (2,5,45)
#  error linux kernel version is too old
#endif
], [ac_cv_linux_version_h_newer_2_5_45=yes], [ac_cv_linux_version_h_newer_2_5_45=no])])])

AS_IF([test "${ac_cv_linux_version_h_newer_2_5_45}" = yes],
  [AC_SEARCH_LIBS([epoll_create], [])],
  [])

AS_IF([test "${ac_cv_search_epoll_create}" != no], [$1], [$2])

])

AC_DEFUN([AX_HAVE_EPOLL_PWAIT], [dnl

AC_REQUIRE([AX_HAVE_EPOLL])

AS_IF([test "${ac_cv_header_sys_epoll_h}" != no],
  [AC_CACHE_CHECK(
    [whether <linux/version.h> is recent enough to have epoll_pwait],
    [ac_cv_linux_version_h_newer_2_6_19],
    [AC_PREPROC_IFELSE([dnl
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION (2,6,19)
#  error linux kernel version is too old
#endif
], [ac_cv_linux_version_h_newer_2_6_19=yes], [ac_cv_linux_version_h_newer_2_6_19=no])])])

AS_IF([test "${ac_cv_linux_version_h_newer_2_6_19}" = yes],
  [AC_SEARCH_LIBS([epoll_pwait], [])],
  [])

AS_IF([test "${ac_cv_search_epoll_pwait}" != no], [$1], [$2])
])
