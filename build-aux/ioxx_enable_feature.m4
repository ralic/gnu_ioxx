#------------------------------------------------------
# IOXX_ENABLE_FEATURE(FEATURE-NAME, MACRO, DESCRIPTION)
#------------------------------------------------------

AC_DEFUN([IOXX_ENABLE_FEATURE], [dnl
  AC_PREREQ([2.61])
  m4_pushdef([DEFINE], m4_translit([$1], -a-z, _A-Z))
  m4_pushdef([FEATURE], m4_translit([$1], -, _))
  $2([AX_CONFIG_FEATURE_ENABLE([$1])], [AX_CONFIG_FEATURE_DISABLE([$1])])
  AX_CONFIG_FEATURE([$1], [$3], [HAVE_]DEFINE, [$3], [enable_]FEATURE[="yes"], [enable_]FEATURE[="no"])
  m4_popdef([FEATURE])
  m4_popdef([DEFINE])
])dnl
