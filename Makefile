# Makefile for the Project Athena Hesiod Nameserver library
#
#	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v $
#	$Author: probe $
#	$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/Makefile,v 1.8 1989-10-18 09:43:53 probe Exp $

DEFINES= -DHESIOD
INCPATH=
CFLAGS = -O ${INCPATH} ${DEFINES}
LINTFLAGS= -uhvpb

SRCS = hesiod.c hespwnam.c hesmailhost.c hesservbyname.c resolve.c cistrcmp.c 
OBJS = hesiod.o hespwnam.o hesmailhost.o hesservbyname.o resolve.o cistrcmp.o 
TOOLS = hesinfo
LIBDIR= /usr/athena/lib
LINTDIR= /usr/lib/lint
INCDIR= /usr/include
BINDIR= /bin/athena
MANDIR= /usr/man/
MAN1= man1
MAN3= man3

.c.o:
	cc -c -pg ${CFLAGS} $*.c
	mv $*.o profiled/$*.o
	cc -c ${CFLAGS} $*.c

all:	hesiod.a ${TOOLS} llib-lhesiod.ln

install:	all
	install -m 644 hesiod.a ${DESTDIR}${LIBDIR}/libhesiod.a
	ranlib ${DESTDIR}${LIBDIR}/libhesiod.a
	install -m 644 hesiod_p.a ${DESTDIR}${LIBDIR}/libhesiod_p.a
	ranlib ${DESTDIR}${LIBDIR}/libhesiod_p.a
	install -m 644 hesiod.h ${DESTDIR}${INCDIR}/hesiod.h
	cp hesiod.3 ${DESTDIR}${MANDIR}${MAN3}
	cp hesinfo.1 ${DESTDIR}${MANDIR}${MAN3}
	install -m 755 hesinfo ${DESTDIR}${BINDIR}
	install -c -m 644 llib-lhesiod.ln ${DESTDIR}${LINTDIR}/llib-lhesiod.ln

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
	-rm -f hesiod.a hesiod_p.a *.o ${TOOLS} *~ profiled/*.o llib-lhesiod.ln

llib-lhesiod.ln: $(SRCS)
	lint -Chesiod $(LINTFLAGS) $(CFLAGS) $(SRCS)

depend:
	makedepend ${CFLAGS} ${SRCS} hesinfo.c
# DO NOT DELETE THIS LINE -- make depend depends on it.

hesiod.o: hesiod.c mit-copyright.h /usr/include/stdio.h /usr/include/errno.h
hesiod.o: /usr/include/strings.h /usr/include/sys/types.h
hesiod.o: /usr/include/netinet/in.h /usr/include/arpa/nameser.h
hesiod.o: /usr/include/resolv.h resscan.h hesiod.h
hespwnam.o: hespwnam.c mit-copyright.h /usr/include/stdio.h
hespwnam.o: /usr/include/pwd.h /usr/include/strings.h
hesmailhost.o: hesmailhost.c mit-copyright.h /usr/include/ctype.h
hesmailhost.o: /usr/include/stdio.h /usr/include/strings.h hesiod.h
resolve.o: resolve.c /usr/include/strings.h /usr/include/sys/param.h
resolve.o: /usr/include/machine/machparam.h /usr/include/sys/signal.h
resolve.o: /usr/include/sys/types.h /usr/include/netinet/in.h
resolve.o: /usr/include/sys/errno.h /usr/include/arpa/nameser.h
resolve.o: /usr/include/resolv.h resscan.h
cistrcmp.o: cistrcmp.c
hesinfo.o: hesinfo.c mit-copyright.h /usr/include/stdio.h
