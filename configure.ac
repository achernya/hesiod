#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([hesiod], [3.1.1], [hesiod@mit.edu])
AM_INIT_AUTOMAKE([-Wall])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/lib/hesiod.h])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AC_PROG_LIBTOOL


# Checks for libraries.
AC_ARG_WITH([libidn],
  [AS_HELP_STRING([--with-libidn], [Support IDN (needs GNU libidn) @<:@check@:>@])],
  [with_libidn="$withval"],
  [with_libidn="check"])
AS_IF([test x"$with_libidn" != xno],
  [AC_SEARCH_LIBS([stringprep_check_version], [idn],
    [AC_DEFINE([HAVE_LIBIDN], [1], [Define if libidn is available.])],
    [AS_IF([test x"$with_idn" = xcheck],
      [AC_MSG_WARN([Unable to find libidn.])],
      [AC_MSG_ERROR([--with-libidn was given but libidn was not found.])])])])

AC_SEARCH_LIBS([res_mkquery], [resolv], [],
  [AC_SEARCH_LIBS([__res_mkquery], [resolv], [],
    [AC_MSG_ERROR([Could not find a library providing res_mkquery])])])

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([netdb.h netinet/in.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_UID_T

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strchr strdup])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/lib/Makefile
	man/Makefile
])
AC_OUTPUT
