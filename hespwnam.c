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

/* This file is part of the Hesiod library.  It implements
 * hesiod_getpwnam and hesiod_getpwuid, for retrieving passwd
 * information about a user.
 */

static char rcsid[] = "$Id: hespwnam.c,v 1.13 1996-12-08 21:40:37 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <netdb.h>
#include "hesiod.h"
#include "config.h"

static struct passwd *getpwcommon(void *context, const char *arg, int which);
static char *next_field(char *ptr);

struct passwd *hesiod_getpwnam(void *context, const char *name)
{
  return getpwcommon(context, name, 0);
}

struct passwd *hesiod_getpwuid(void *context, uid_t uid)
{
  char uidstr[16];

  sprintf(uidstr, "%d", uid);
  return getpwcommon(context, uidstr, 1);
}

void hesiod_free_passwd(void *context, struct passwd *pw)
{
  free(pw->pw_name);
  free(pw);
}

static struct passwd *getpwcommon(void *context, const char *arg, int which)
{
  char *p, **list;
  struct passwd *pw;

  /* Get the response and copy the first entry into a buffer. */
  list = hesiod_resolve(context, arg, which ? "uid" : "passwd");
  if (!list)
    return NULL;
  p = malloc(strlen(*list) + 1);
  if (!p)
    {
      hesiod_free_list(context, list);
      errno = ENOMEM;
      return NULL;
    }
  strcpy(p, *list);
  hesiod_free_list(context, list);

  /* Allocate memory for the result. */
  pw = (struct passwd *) malloc(sizeof(struct passwd));
  if (!pw)
    {
      free(p);
      errno = ENOMEM;
      return NULL;
    }

  /* Split up buf into fields. */
  pw->pw_name = p;
  p = next_field(p);
  pw->pw_passwd = p;
  p = next_field(p);
  pw->pw_uid = atoi(p);
  p = next_field(p);
  pw->pw_gid = atoi(p);
  p = next_field(p);
  pw->pw_gecos = p;
  p = next_field(p);
  pw->pw_dir = p;
  p = next_field(p);
  pw->pw_shell = p;
  while (*p && *p != '\n')
    p++;
  *p = 0;

#ifdef HAVE_PW_QUOTA
  pw->pw_quota = 0;
#endif
#ifdef HAVE_PW_COMMENT
  pw->pw_comment = "";
#endif
#ifdef HAVE_PW_CLASS
  pw->pw_class = "";
#endif
#ifdef HAVE_PW_CHANGE
  pw->pw_change = 0;
#endif
#ifdef HAVE_PW_EXPIRE
  pw->pw_expire = 0;
#endif

  return pw;
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
