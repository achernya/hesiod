/* Copyright 1988, 1996 by the Massachusetts Institute of Technology.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in
 *   the documentation and/or other materials provided with the
 *   distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* This file is part of the Hesiod library.  It implements
 * hesiod_getpwnam and hesiod_getpwuid, for retrieving passwd
 * information about a user.
 */

static const char rcsid[] = "$Id: hespwnam.c,v 1.16 2002-04-03 21:40:55 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#include <errno.h>
#include <netdb.h>
#include "hesiod.h"
#include "config.h"

extern char **hesiod__uidresolve(void *context, const char *uidstr);

static struct passwd *getpwcommon(void *context, const char *arg, int which);
static char *next_field(char *ptr);

struct passwd *hesiod_getpwnam(void *context, const char *name)
{
  return getpwcommon(context, name, 0);
}

struct passwd *hesiod_getpwuid(void *context, uid_t uid)
{
  char uidstr[16];

  sprintf(uidstr, "%u", (unsigned int)uid);
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
