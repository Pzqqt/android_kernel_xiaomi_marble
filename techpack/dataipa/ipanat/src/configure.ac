#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT(data-ipanat, 1.0.0)
AM_INIT_AUTOMAKE(data-ipanat, 1.0.0)
AC_OUTPUT(Makefile)
AC_CONFIG_SRCDIR([ipa_nat_drv.c])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_CC
AC_PROG_LIBTOOL
AC_PROG_CXX

# Checks for libraries.

AC_ARG_WITH(sanitized-headers,
      AS_HELP_STRING([--with-sanitized-headers=DIR],
         [Specify the location of the sanitized Linux headers]),
      [CPPFLAGS="$CPPFLAGS -idirafter $withval"])
AM_CONDITIONAL(KERNELMODULES, [test -n -eq 0])

AC_ARG_WITH([glib],
      AC_HELP_STRING([--with-glib],
         [enable glib, building HLOS systems which use glib]))

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

AC_ARG_ENABLE(target,
    [AS_HELP_STRING([--enable-target=TARGET], [Specify the target product to build])],
    [TARGET=$enableval],
    [TARGET=none]
)

if test "x$TARGET" = "xsdx20"; then
    echo "\n UL firewall feature enabled\n"
    CPPFLAGS="${CPPFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS"
    CFLAGS="${CFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS"
fi

if test "x$TARGET" = "xsdxpoorwills"; then
    echo "\n UL firewall feature enabled\n"
    CPPFLAGS="${CPPFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN -DFEATURE_L2TP"
    CFLAGS="${CFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN -DFEATURE_L2TP"
fi

if test "x$TARGET" = "xsdxprairie"; then
    echo "\n UL firewall feature enabled\n"
    CPPFLAGS="${CPPFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN"
    CFLAGS="${CFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN"
fi

if test "x$TARGET" = "xsdxlemur"; then
    echo "\n UL firewall feature enabled\n"
    CPPFLAGS="${CPPFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN"
    CFLAGS="${CFLAGS} -DFEATURE_IPACM_UL_FIREWALL -DFEATURE_IPACM_PER_CLIENT_STATS -DFEATURE_VLAN_MPDN"
fi
AM_CONDITIONAL(NO_L2TP, test "x$TARGET" = "xsdx20" || test "x$TARGET" = "xsdxpoorwills" || test "x$TARGET" = "xsdxprairie" || test "x$TARGET" = "xsdxlemur")

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h netinet/in.h sys/ioctl.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_OFF_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_MMAP
AC_CHECK_FUNCS([memset munmap])

AC_OUTPUT
