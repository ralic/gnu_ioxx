#----------------------------------------------------------------------------
# AX_HAVE_BOOST_UNIT_TEST_FRAMEWORK([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#----------------------------------------------------------------------------

AC_DEFUN([AX_TRY_LINK_BOOST_UNIT_TEST_FRAMEWORK], [dnl
  AC_LINK_IFELSE([dnl
#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
BOOST_AUTO_TEST_CASE( try_linking_unit_test_framework ) { }
],  [$1],
    [$2])
])

AC_DEFUN([AX_HAVE_BOOST_UNIT_TEST_FRAMEWORK], [dnl
  AC_MSG_CHECKING([for boost unit test framework library])
  AC_CACHE_VAL([ax_cv_have_boost_unit_test_framework], [dnl
    AC_LANG_PUSH([C++])
    ax_have_boost_unit_test_framework_libs="${LIBS}"
    LIBS="${LIBS} -lboost_unit_test_framework${BOOST_SUFFIX}"
    AX_TRY_LINK_BOOST_UNIT_TEST_FRAMEWORK(
      [AC_SUBST([BOOST_UNIT_TEST_FRAMEWORK_CPPFLAGS], [])
       ax_cv_have_boost_unit_test_framework=yes],
      [ax_have_boost_unit_test_framework_cppflags="${CPPFLAGS}"
       CPPFLAGS="${CPPFLAGS} -DBOOST_TEST_DYN_LINK=1"
       AX_TRY_LINK_BOOST_UNIT_TEST_FRAMEWORK(
         [AC_SUBST([BOOST_UNIT_TEST_FRAMEWORK_CPPFLAGS], [-DBOOST_TEST_DYN_LINK=1])
          ax_cv_have_boost_unit_test_framework=yes],
         [AC_SUBST([BOOST_UNIT_TEST_FRAMEWORK_CPPFLAGS], [])
          ax_cv_have_boost_unit_test_framework=no])
       CPPFLAGS="${ax_have_boost_unit_test_framework_cppflags}"
      ])
    LIBS="${ax_have_boost_unit_test_framework_libs}"
    AC_LANG_POP([C++])])
  AS_IF([test "${ax_cv_have_boost_unit_test_framework}" = "yes"],
    [AC_MSG_RESULT([yes])
$1],[AC_MSG_RESULT([no])
$2])
])dnl
