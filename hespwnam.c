/* This file contains hes_getpwnam, for retrieving passwd information about
 * a user.
 *
 * For copying and distribution information, see the file <mit-copyright.h>
 *
 * Original version by Steve Dyer, IBM/Project Athena.
 *
 *	$Author: treese $
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hespwnam.c,v $
 *	$Athena: hespwnam.c,v 1.4 88/08/07 21:52:51 treese Locked $
 *	$Log: not supported by cvs2svn $
 * Revision 1.4  88/08/07  21:52:51  treese
 * First public distribution
 * 
 * Revision 1.3  88/08/07  21:48:34  treese
 * Cleaned up using saber.
 * 
 * Revision 1.2  88/06/05  19:51:41  treese
 * Cleaned up for public distribution
 * 
 *
 */

#include "mit-copyright.h"

#ifndef lint
static char rcsid_pwnam_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hespwnam.c,v 1.5 1988-08-07 23:17:19 treese Exp $";
#endif
#include <stdio.h>
#include <pwd.h>
#include <strings.h>

static struct passwd pw_entry;
static char buf[256];

struct passwd *
hes_getpwnam(nam)
	char *nam;
{
	register char *p, **pp; char *_NextPWField(), **hes_resolve();

	pp = hes_resolve(nam, "passwd");
	if (pp == NULL)
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
	pw_entry.pw_quota = 0;
	pw_entry.pw_comment = "";
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

