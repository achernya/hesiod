#include <ctype.h>
#include <stdio.h>
#include <hesiod.h>

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
