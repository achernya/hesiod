/*
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.  The Berkeley software License Agreement
 * specifies the terms and conditions for redistribution.
 */

#if defined(LIBC_SCCS) && !defined(lint)
static char sccsid[] = "@(#)getservbyname.c	5.3 (Berkeley) 5/19/86";
#endif LIBC_SCCS and not lint

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#ifdef POSIX
#include <stdlib.h>
#else
extern char *malloc();
#endif
#include "hesiod.h"

#define LISTSIZE 15
struct servent *
hes_getservbyname(name, proto)
	char *name, *proto;
{
	register struct servent *p;
	register char *l, **cp;
	register int i = 0;
	static char *aliases[LISTSIZE];

	cp = hes_resolve(name, "service");
	if (cp == NULL) return(NULL);
	p = (struct servent *) malloc(sizeof(struct servent));
	while (l = *cp++) {
		register char *servicename, *protoname, *port;

		while(*l && (*l == ' ' || *l == '\t')) l++;
		servicename = l;
		while(*l && *l != ' ' && *l != '\t' && *l != ';') l++;
		if (*l == '\0') continue; /* malformed entry */
		*l++ = '\0';
		while(*l && (*l == ' ' || *l == '\t')) l++;
		protoname = l;
		while(*l && *l != ' ' && *l != ';') l++;
		if (*l == '\0') continue; /* malformed entry */
		*l++ = '\0';
		if (cistrcmp(proto, protoname)) continue; /* wrong port */
		while(*l && (*l == ' ' || *l == '\t' || *l == ';')) l++;
		if (*l == '\0') continue; /* malformed entry */
		port = l;
		while(*l && (*l != ' ' && *l != '\t' && *l != ';')) l++;
		if (*l) *l++ = '\0';
		if (*l != '\0') {
			do {
				aliases[i++] = l;
				while(*l && !isspace(*l)) l++;
				if (*l) *l++ = 0;
			} while(*l);
		}
		aliases[i] = NULL;
		p->s_name = servicename;
		p->s_port = htons((u_short)atoi(port));
		p->s_proto = protoname;
		p->s_aliases = aliases;
		return (p);
	}
	return(NULL);
}
