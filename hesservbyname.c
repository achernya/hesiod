/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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

/* This file is part of the Hesiod library.  It contains
 * hes_getservbyname, used to get information about network services.
 */

static char rcsid[] = "$Id: hesservbyname.c,v 1.5 1996-11-07 02:30:21 ghudson Exp $";

#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include <stdlib.h>
#include "hesiod.h"

extern int hes_errno;

static int cistrcmp(const char *s1, const char *s2);

struct servent *hes_getservbyname(const char *name, const char *proto)
{
  static struct servent result;
  static char buf[2048];

  hes_init();
  hes_errno = hes_getservbyname_r(name, proto, &result, buf, sizeof(buf));
  return (hes_errno == HES_ER_OK) ? &result : NULL;
}

int hes_getservbyname_r(const char *name, const char *proto,
			struct servent *result, char *buffer, int buflen)
{
  char **p, **aliases = (char **) buffer, *vec[100], *line;
  int i = 0, status;

  status = hes_resolve_r(name, "service", vec, 100);
  if (status != HES_ER_OK)
    return status;
  status = HES_ER_NOTFOUND;
  for (p = vec; *p; p++)
    {
      char *servicename, *protoname, *port, *l = *p;
      int len = strlen(l);

      /* Find the service name. */
      while (isspace(*l))
	l++;
      servicename = l;
      while (*l && !isspace(*l) && *l != ';')
	l++;
      if (!*l) /* Malformed entry */
	continue;
      *l++ = 0;

      /* Find the protocol name and check it. */
      while (isspace(*l))
	l++;
      protoname = l;
      while (*l && !isspace(*l) && *l != ';')
	l++;
      if (!*l) /* Malformed entry */
	continue;
      *l++ = 0;
      if (cistrcmp(proto, protoname)) /* Wrong protocol */
	continue;

      /* Find the port number. */
      while (isspace(*l) || *l == ';')
	l++;
      if (!*l) /* Malformed entry */
	continue;
      port = l;

      while (*l && !isspace(*l) && *l != ';')
	l++;
      if (*l)
	*l++ = 0;
      while (*l)
	{
	  if ((i + 2) * sizeof(char *) + len >= buflen)
	    break;
	  aliases[i++] = l;
	  while (*l && !isspace(*l))
	    l++;
	  if (*l)
	    *l++ = 0;
	}
      if ((i + 1) * sizeof(char *) + len >= buflen)
	{
	  status = HES_ER_RANGE;
	  break;
	}
      aliases[i++] = 0;
      line = (char *) (aliases + i);
      memcpy(line, *p, len + 1);
      result->s_name = line;
      result->s_port = htons(atoi(port));
      result->s_proto = line + (protoname - *p);
      result->s_aliases = aliases;
      for (; *aliases; aliases++)
	*aliases = line + (*aliases - *p);
      status = HES_ER_OK;
      break;
    }
  for (p = vec; *p; p++)
    free(*p);
  return status;
}

static int cistrcmp(const char *s1, const char *s2)
{
  while (*s1 && tolower(*s1) == tolower(*s2))
    {
      s1++;
      s2++;
    }
  return tolower(*s1) - tolower(*s2);
}
