AM_CFLAGS = -DSYSCONFDIR=\"${sysconfdir}\"

lib_LTLIBRARIES = libhesiod.la
libhesiod_la_SOURCES = \
	hesiod.c \
	hesmailhost.c \
	hespwnam.c \
	hesservbyname.c \
	hescompat.c

bin_PROGRAMS = hestest
hestest_SOURCES = hestest.c
hestest_LDADD = libhesiod.la
