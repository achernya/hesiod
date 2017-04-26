#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([hesiod], [3.2.1], [hesiod@mit.edu])
AM_INIT_AUTOMAKE([-Wall])
AM_MAINTAINER_MODE([enable])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/lib/hesiod.h])
AC_CONFIG_HEADERS([config.h])
AC_USE_SYSTEM_EXTENSIONS

# Checks for programs.
AC_PROG_CC
# automake 1.12 seems to require this, but automake 1.11 doesn't recognize it
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
LT_INIT

# Checks for libraries.
hesiod_save_LIBS="$LIBS"
LIBS=
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

AC_ARG_WITH([libidn-headers],
       [AS_HELP_STRING([--with-libidn-headers], [Path to libidn headers @<:@default=/usr/include@:>@])],
       [CFLAGS="$CFLAGS -I$with_libidn_headers"],
       []) 

AC_ARG_WITH([libidn2],
  [AS_HELP_STRING([--with-libidn2], [Support IDN (needs GNU libidn2) @<:@check@:>@])],
  [with_libidn2="$withval"],
  [with_libidn2="check"])
AS_IF([test x"$with_libidn2" != xno],
  [AC_SEARCH_LIBS([idn2_check_version], [idn2],
    [AC_DEFINE([HAVE_LIBIDN2], [1], [Define if libidn2 is available.])],
    [AS_IF([test x"$with_idn2" = xcheck],
      [AC_MSG_WARN([Unable to find libidn2.])],
      [AC_MSG_ERROR([--with-libidn2 was given but libidn2 was not found.])])])])

AC_CHECK_LIB([resolv], [res_mkquery], [], [
    AC_MSG_CHECKING([if res_mkquery is provided by libresolv with mangled symbols])
    save_LIBS="$LIBS"
    LIBS="-lresolv $LIBS"
    AC_TRY_LINK([#include <resolv.h>],
                [res_mkquery(0,NULL,0,0,NULL,0,NULL,NULL,0);], [
                    AC_DEFINE(HAVE_LIBRESOLV, [1], [Define if your libresolv provides res_mkquery.])
		    AC_MSG_RESULT(yes)], [
		    LIBS="$save_LIBS"
		    AC_MSG_RESULT(no)])])

AC_SUBST([SUPPORTING_LIBS], $LIBS)
LIBS=$hesiod_save_LIBS

# Checks for header files.
AC_HEADER_RESOLV
AC_CHECK_HEADERS([netdb.h netinet/in.h stdlib.h string.h sys/socket.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_SSIZE_T
AC_TYPE_UID_T

AC_EGREP_HEADER([pw_quota], [pwd.h],
    AC_DEFINE([HAVE_PW_QUOTA], 1,
	      [Define if struct passwd has a field pw_quota]))
AC_EGREP_HEADER([pw_comment], [pwd.h],
AC_DEFINE([HAVE_PW_COMMENT], 1,
	  [Define if struct passwd has a field pw_comment]))
AC_EGREP_HEADER([pw_class], [pwd.h],
   AC_DEFINE([HAVE_PW_CLASS], 1,
	     [Define if struct passwd has a field pw_class]))
AC_EGREP_HEADER([pw_change], [pwd.h],
    AC_DEFINE([HAVE_PW_CHANGE], 1,
	      [Define if struct passwd has a field pw_change]))
AC_EGREP_HEADER([pw_expire], [pwd.h],
    AC_DEFINE([HAVE_PW_EXPIRE], 1,
	      [Define if struct passwd has a field pw_expire]))

# Checks for library functions.
AC_FUNC_MALLOC
AC_FUNC_REALLOC
AC_CHECK_FUNCS([strchr strdup secure_getenv])

AC_CONFIG_FILES([
	Makefile
	src/Makefile
	src/lib/Makefile
	src/hesinfo/Makefile
	man/Makefile
	src/hesiod.pc
])
AC_OUTPUT
