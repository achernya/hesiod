/* This file contains hes_postoffice, which retrieves post-office information
 * for a user.
 *
 * For copying and distribution information, see the file <mit-copyright.h>
 *
 * Original version by Steve Dyer, IBM/Project Athena.
 *
 *	$Author: treese $
 *	$Athena: hesmailhost.c,v 1.2 88/06/05 19:51:36 treese Locked $
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesmailhost.c,v $
 *	$Log: not supported by cvs2svn $
 * Revision 1.2  88/06/05  19:51:36  treese
 * Cleaned up for public distribution
 * 
 *
 */

#include "mit-copyright.h"

#ifndef lint
static char rcsid_hesmailhost_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesmailhost.c,v 1.3 1988-06-12 00:53:06 treese Exp $";
#endif

#include <ctype.h>
#include <stdio.h>
#include <strings.h>

#include "hesiod.h"

#define LINESIZE 80

struct hes_postoffice *
hes_getmailhost(user)
char *user;
{
	static struct hes_postoffice ret;
	static char linebuf[LINESIZE];
	char *p;
	char **cp;

	cp = hes_resolve(user, "pobox");
	if (cp == NULL) return(NULL);
	strcpy(linebuf, *cp);
	ret.po_type = linebuf;
	p = linebuf;
	while(!isspace(*p)) p++;
	*p++ = '\0';
	ret.po_host = p;
	while(!isspace(*p)) p++;
	*p++ = '\0';
	ret.po_name = p;
	return(&ret);
}
