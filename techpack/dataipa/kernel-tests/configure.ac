AC_PREREQ(2.64)

AC_INIT([ipa-kernel-tests],[1.0.0])
AM_INIT_AUTOMAKE([-Wall -Werror gnu foreign])

AM_MAINTAINER_MODE

AC_CONFIG_HEADER([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL
AC_PROG_CPP
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AC_PROG_CXX


AS_CASE([$host],
        [aarch64*], [ARM64=yes],
        [ARM64=no]
)

AM_CONDITIONAL(ARM64, [test "x$ARM64" = "xyes"])

AC_ARG_WITH([glib],
       AC_HELP_STRING([--with-glib],
          [enable glib, building HLOS systems which use glib]))

AC_ARG_WITH(ipanat-headers,
      AS_HELP_STRING([--with-ipanat-headers=DIR],
         [Specify the location of the ipanat headers]),
      [CPPFLAGS="$CPPFLAGS -idirafter $withval"])

if (test "x${with_glib}" = "xyes"); then
         AC_DEFINE(ENABLE_USEGLIB, 1, [Define if HLOS systems uses glib])
         PKG_CHECK_MODULES(GTHREAD, gthread-2.0 >= 2.16, dummy=yes,
                                 AC_MSG_ERROR(GThread >= 2.16 is required))
         PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.16, dummy=yes,
                                 AC_MSG_ERROR(GLib >= 2.16 is required))
         GLIB_CFLAGS="$GLIB_CFLAGS $GTHREAD_CFLAGS"
         GLIB_LIBS="$GLIB_LIBS $GTHREAD_LIBS"

         AC_SUBST(GLIB_CFLAGS)
         AC_SUBST(GLIB_LIBS)
fi

AM_CONDITIONAL(USE_GLIB, test "x${with_glib}" = "xyes")

AC_CONFIG_FILES([
	Makefile
	])
AC_OUTPUT
