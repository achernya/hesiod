/* This file contains hes_getpwnam, for retrieving passwd information about
 * a user.
 *
 * For copying and distribution information, see the file <mit-copyright.h>
 *
 * Original version by Steve Dyer, IBM/Project Athena.
 *
 *	$Author: epeisach $
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hespwnam.c,v $
 *	$Athena: hespwnam.c,v 1.4 88/08/07 21:52:51 treese Locked $
 */

#include "mit-copyright.h"

#ifndef lint
static char rcsid_pwnam_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hespwnam.c,v 1.10 1993-10-22 14:06:11 epeisach Exp $";
#endif
#include <stdio.h>
#include <pwd.h>
#include <string.h>
#include "hesiod.h"

static struct passwd pw_entry;
static char buf[256];

static char *_NextPWField();    /* For later definition in file */

static struct passwd *
hes_getpwcommon(arg, which)
	char *arg;
	int which;	/* 0=hes_getpwnam, 1=hes_getpwuid */
{
	register char *p, **pp;

	pp = hes_resolve(arg, which ? "uid" : "passwd");
	if (pp == NULL || *pp == NULL)
		return(NULL);
	/* choose only the first response (only 1 expected) */
	(void) strcpy(buf, pp[0]);
	p = buf;
	pw_entry.pw_name = p;
	p = _NextPWField(p);
	pw_entry.pw_passwd = p;
	p = _NextPWField(p);
	pw_entry.pw_uid = atoi(p);
	p = _NextPWField(p);
	pw_entry.pw_gid = atoi(p);
#if (!defined(_AIX) || (AIXV < 31)) && !defined(sun)
	pw_entry.pw_quota = 0;
#if defined(_AIX) && (AIXV < 31)
	pw_entry.pw_age =
#endif
	pw_entry.pw_comment = "";
#endif
	p = _NextPWField(p);
	pw_entry.pw_gecos = p;
	p = _NextPWField(p);
	pw_entry.pw_dir = p;
	p = _NextPWField(p);
	pw_entry.pw_shell = p;
	while (*p && *p != '\n')
		p++;
	*p = '\0';
	return(&pw_entry);
}

/* Move the pointer forward to the next colon-separated field in the
 * password entry.
 */

static char *
_NextPWField(ptr)
char *ptr;
{
	while (*ptr && *ptr != '\n' && *ptr != ':')
		ptr++;
	if (*ptr)
		*ptr++ = '\0';
	return(ptr);
}

struct passwd *
hes_getpwnam(nam)
	char *nam;
{
	return hes_getpwcommon(nam, 0);
}

struct passwd *
hes_getpwuid(uid)
	int uid;
{
	char uidstr[16];

	sprintf(uidstr, "%d", uid);
	return hes_getpwcommon(uidstr, 1);
}
