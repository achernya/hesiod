/* Copyright 1988, 1996 by the Massachusetts Institute of Technology.
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */

/* This file is part of the Hesiod library.  It implements hes_getpwnam,
 * for retrieving passwd information about a user.
 */

static char rcsid[] = "$Id: hespwnam.c,v 1.12 1996-11-07 02:30:19 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <string.h>
#include <netdb.h>
#include "hesiod.h"
#include "config.h"

extern int hes_errno;

static struct passwd pw_entry;
static char buf[256];

static int hes_getpwcommon(const char *arg, int which, struct passwd *entry,
			   char *buf, int bufsize);
static char *next_field(char *ptr);

struct passwd *hes_getpwnam(const char *nam)
{
  hes_init();
  hes_errno = hes_getpwcommon(nam, 0, &pw_entry, buf, sizeof(buf));
  return (hes_errno == HES_ER_OK) ? &pw_entry : NULL;
}

struct passwd *hes_getpwuid(int uid)
{
  char uidstr[16];

  hes_init();
  sprintf(uidstr, "%d", uid);
  hes_errno = hes_getpwcommon(uidstr, 1, &pw_entry, buf, sizeof(buf));
  return (hes_errno == HES_ER_OK) ? &pw_entry : NULL;
}

int hes_getpwnam_r(const char *nam, struct passwd *entry, char *buf,
		   int bufsize, struct passwd **result)
{
  int status;

  status = hes_getpwcommon(nam, 0, entry, buf, bufsize);
  *result = (status == HES_ER_OK) ? entry : NULL;
  return status;
}

int hes_getpwuid_r(int uid, struct passwd *entry, char *buf, int bufsize,
		   struct passwd **result)
{
  char uidstr[16];
  int status;

  sprintf(uidstr, "%d", uid);
  status = hes_getpwcommon(uidstr, 1, entry, buf, bufsize);
  *result = (status == HES_ER_OK) ? entry : NULL;
  return status;
}

static int hes_getpwcommon(const char *arg, int which, struct passwd *entry,
			   char *buf, int bufsize)
{
  char *p;
  char *pp[2];
  int status;

  /* Get the response, sanity-check it, and copy it into buf. */
  status = hes_resolve_r(arg, which ? "uid" : "passwd", pp, 2);
  if (status != HES_ER_OK)
    return(status);
  if (*pp == NULL)
    return(HES_ER_INVAL);
  if (strlen(pp[0]) > bufsize - 1)
    {
      free(pp[0]);
      return(HES_ER_RANGE);
    }
  strcpy(buf, pp[0]);
  free(pp[0]);

  /* Split up buf into fields. */
  p = buf;
  entry->pw_name = p;
  p = next_field(p);
  entry->pw_passwd = p;
  p = next_field(p);
  entry->pw_uid = atoi(p);
  p = next_field(p);
  entry->pw_gid = atoi(p);
  p = next_field(p);
  entry->pw_gecos = p;
  p = next_field(p);
  entry->pw_dir = p;
  p = next_field(p);
  entry->pw_shell = p;
  while (*p && *p != '\n')
    p++;
  *p = 0;

#ifdef HAVE_PW_QUOTA
  entry->pw_quota = 0;
#endif
#ifdef HAVE_PW_COMMENT
  entry->pw_comment = "";
#endif

  return HES_ER_OK;
}

/* Null-terminate the current colon-separated field in the passwd entry and
 * return a pointer to the beginning of the next one.
 */
static char *next_field(char *ptr)
{
  while (*ptr && *ptr != '\n' && *ptr != ':')
    ptr++;
  if (*ptr)
    *ptr++ = '\0';
  return ptr;
}
