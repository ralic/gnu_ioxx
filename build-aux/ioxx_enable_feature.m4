#------------------------------------------------------
# IOXX_ENABLE_FEATURE(FEATURE-NAME, MACRO, DESCRIPTION)
#------------------------------------------------------

AC_DEFUN([IOXX_ENABLE_FEATURE], [dnl
  AC_PREREQ([2.61])
  m4_pushdef([DEFINE], m4_translit([$1], -a-z, _A-Z))
  $2([AX_CONFIG_FEATURE_ENABLE([$1])], [AX_CONFIG_FEATURE_DISABLE([$1])])
  AX_CONFIG_FEATURE([$1], [$3], [HAVE_]DEFINE, [$3],
    [AC_SUBST([IOXX_HAVE_]DEFINE, [1])],
    [AC_SUBST([IOXX_HAVE_]DEFINE, [0])])
  m4_popdef([DEFINE])
])dnl
