# Makefile for the Project Athena Hesiod Nameserver library
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v $
#	$Author: treese $
#	$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v 1.4 1988-08-07 21:51:55 treese Exp $

DEFINES= -DHESIOD
INCPATH= -I../include
CFLAGS = -O ${INCPATH} ${DEFINES}

SRCS = hesiod.c hespwnam.c hesmailhost.c resolve.c cistrcmp.c 
OBJS = hesiod.o hespwnam.o hesmailhost.o resolve.o cistrcmp.o 
TOOLS = hesinfo
LIBDIR= /usr/athena/lib
INCDIR= /usr/include
BINDIR= /bin/athena
MANDIR= /usr/man
MAN1= man1
MAN3= man3

.c.o:
	cc -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	cc -c ${CFLAGS} $*.c

all:	hesiod.a ${TOOLS}

install:	all
	install -m 644 hesiod.a ${DESTDIR}${LIBDIR}/libhesiod.a
	ranlib ${DESTDIR}${LIBDIR}/libhesiod.a
	install -m 644 hesiod_p.a ${DESTDIR}${LIBDIR}/libhesiod_p.a
	ranlib ${DESTDIR}${LIBDIR}/libhesiod_p.a
	install -m 644 hesiod.h ${DESTDIR}${INCDIR}/hesiod.h
	cp hesiod.3 ${DESTDIR}${MANDIR}${MAN3}
	cp hesinfo.1 ${DESTDIR}${MANDIR}${MAN3}
	install -m 755 hesinfo ${DESTDIR}${BINDIR}

hesiod.a:	${OBJS}
	ar rc $@ ${OBJS}
	ranlib $@
	cd profiled; ar rc ../hesiod_p.a ${OBJS}
	ranlib hesiod_p.a

hesinfo:	hesiod.a hesinfo.c
	cc ${CFLAGS} -o hesinfo hesinfo.c hesiod.a

saber:
	saber ${INCPATH} ${DEFINES} hesinfo.c ${SRCS}

clean:	
	-rm -f hesiod.a hesiod_p.a *.o ${TOOLS} *~ profiled/*.o
