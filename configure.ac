AC_PREREQ(2.57)
AC_INIT(eabout, 0.1, avn@openinkpot.org)
AM_INIT_AUTOMAKE(foreign)

AC_LANG_C
AC_PROG_CC

PKG_CHECK_MODULES(EFL, [ecore ecore-file ecore-evas ecore-x evas edje])
PKG_CHECK_MODULES(LIBKEYS, [libkeys])
PKG_CHECK_MODULES(LIBEOI, [libeoi])
PKG_CHECK_MODULES(LOPS, [liblops])

AC_OUTPUT([Makefile src/Makefile scripts/Makefile data/Makefile themes/Makefile po/Makefile help/Makefile])
