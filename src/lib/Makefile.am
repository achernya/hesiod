AM_CFLAGS = -DSYSCONFDIR=\"${sysconfdir}\"

include_HEADERS = hesiod.h

lib_LTLIBRARIES = libhesiod.la
libhesiod_la_SOURCES = \
	hesiod.c \
	hesmailhost.c \
	hespwnam.c \
	hesservbyname.c \
	hescompat.c
libhesiod_la_LIBADD = @SUPPORTING_LIBS@

noinst_PROGRAMS = hestest
hestest_SOURCES = hestest.c
hestest_LDADD = libhesiod.la

TESTS_ENVIRONMENT = ./hestest
TESTS = hestest.conf

EXTRA_DIST = hesiod.conf.sample hestest.conf
