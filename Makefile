# Makefile for the Project Athena Hesiod Nameserver library
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v $
#	$Author: treese $
#	$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v 1.1 1988-06-05 17:50:55 treese Exp $

CFLAGS = -O -I. -DHESIOD

OBJS = hesiod.o hespwnam.o hesservbyname.o hesmailhost.o resolve.o \
	cistrcmp.o 
TOOLS = hesinfo

.c.o:
	cc -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	cc -c ${CFLAGS} $*.c

all:	hesiod.a tools hesinfo

install:	all
	install -m 644 hesiod.a ${DESTDIR}/usr/athena/lib/libhesiod.a
	ranlib ${DESTDIR}/usr/athena/lib/libhesiod.a
	install -m 644 hesiod_p.a ${DESTDIR}/usr/athena/lib/libhesiod_p.a
	ranlib ${DESTDIR}/usr/athena/lib/libhesiod_p.a
	install -m 644 hesiod.h ${DESTDIR}/usr/include/hesiod.h
	cp hesiod.3 ${DESTDIR}/usr/man/man3
	cp hesinfo.1 ${DESTDIR}/usr/man/man1
	install -m 755 hesinfo ${DESTDIR}/bin/athena
	rm -f ${DESTDIR}/usr/athena/hesinfo
	ln -s /bin/athena/hesinfo ${DESTDIR}/usr/athena/hesinfo

hesiod.a:	${OBJS}
	ar rc $@ ${OBJS}
	ranlib $@
	cd profiled; ar rc ../hesiod_p.a ${OBJS}
	ranlib hesiod_p.a

tools:	hesiod.a ${TOOLS}

hesinfo:	hesiod.a hesinfo.c
	cc ${CFLAGS} -o hesinfo hesinfo.c hesiod.a

clean:	
	-rm -f hesiod.a hesiod_p.a *.o ${TOOLS} *~ profiled/*.o
