#include <stdio.h>
#include <pwd.h>

static struct passwd passwd;
static char buf[256];

struct passwd *
hes_getpwnam(nam)
	char *nam;
{
	register struct passwd *pw;
	register char *p, **pp; char *pwskip(), **hes_resolve();

	pp = hes_resolve(nam, "passwd");
	if (pp == NULL)
		return(NULL);
	/* choose only the first response (only 1 expected) */
	strcpy(buf, pp[0]);
	p = buf;
	passwd.pw_name = p;
	p = pwskip(p);
	passwd.pw_passwd = p;
	p = pwskip(p);
	passwd.pw_uid = atoi(p);
	p = pwskip(p);
	passwd.pw_gid = atoi(p);
	passwd.pw_quota = 0;
	passwd.pw_comment = "";
	p = pwskip(p);
	passwd.pw_gecos = p;
	p = pwskip(p);
	passwd.pw_dir = p;
	p = pwskip(p);
	passwd.pw_shell = p;
	while (*p && *p != '\n')
		p++;
	*p = '\0';
	return(&passwd);
}

static char *
pwskip(p)
register char *p;
{
	while (*p && *p != ':' && *p != '\n')
		++p;
	if (*p)
		*p++ = 0;
	return(p);
}

