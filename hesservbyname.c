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
 * hesiod_getservbyname, used to get information about network
 * services.
 */

static char rcsid[] = "$Id: hesservbyname.c,v 1.6 1996-12-08 21:40:44 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <pwd.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "hesiod.h"

static int cistrcmp(const char *s1, const char *s2);

struct servent *hesiod_getservbyname(void *context, const char *name,
				     const char *proto)
{
  char **item, **list;
  int i = 0, status;

  /* Ask for all entries matching the given service name. */
  list = hesiod_resolve(context, name, "service");
  if (!list)
    return NULL;

  /* Look through the returned list for entries matching the given protocol. */
  for (item = list; *item; item++)
    {
      char **alias, *servicename, *protoname, *port, *p2, *p = *item;
      int len = strlen(p), naliases;
      struct servent *serv;

      /* Find the service name. */
      while (isspace(*p))
	p++;
      servicename = p;
      while (*p && !isspace(*p) && *p != ';')
	p++;
      if (!*p) /* Malformed entry */
	continue;
      *p++ = 0;

      /* Find the protocol name and check it. */
      while (isspace(*p))
	p++;
      protoname = p;
      while (*p && !isspace(*p) && *p != ';')
	p++;
      if (!*p) /* Malformed entry */
	continue;
      *p++ = 0;
      if (cistrcmp(proto, protoname)) /* Wrong protocol */
	continue;

      /* Find the port number. */
      while (isspace(*p) || *p == ';')
	p++;
      if (!*p) /* Malformed entry */
	continue;
      port = p;

      while (*p && !isspace(*p) && *p != ';')
	p++;
      while (isspace(*p) || *p == ';')
	p++;

      /* Count the number of aliases. */
      naliases = 0;
      p2 = p;
      while (*p2)
	{
	  naliases++;
	  while (*p2 && !isspace(*p2))
	    p2++;
	  while (isspace(*p2))
	    p2++;
	}

      /* Allocate space for the answer. */
      serv = (struct servent *) malloc(sizeof(struct servent));
      if (serv)
	{
	  serv->s_name = malloc(strlen(servicename) + strlen(proto)
				 + strlen(p) + 3);
	  if (serv->s_name)
	    serv->s_aliases = (char **)
	      malloc((naliases + 1) * sizeof(char *));
	  if (serv->s_name && !serv->s_aliases)
	    free(serv->s_name);
	  if (!serv->s_name || !serv->s_aliases)
	    free(serv);
	}
      if (!serv || !serv->s_name || !serv->s_aliases)
	{
	  errno = ENOMEM;
	  return NULL;
	}

      /* Copy the information we found into the answer. */
      serv->s_port = htons(atoi(port));
      strcpy(serv->s_name, name);
      serv->s_proto = serv->s_name + strlen(name) + 1;
      strcpy(serv->s_proto, proto);
      p2 = serv->s_proto + strlen(proto) + 1;
      strcpy(p2, p);
      alias = serv->s_aliases;
      while (*p2)
	{
	  *alias++ = p2;
	  while (*p2 && !isspace(*p2))
	    p2++;
	  if (*p2)
	    {
	      *p2++ = 0;
	      while (isspace(*p2))
		p2++;
	    }
	}
      *alias = NULL;

      hesiod_free_list(context, list);
      return serv;
    }
  hesiod_free_list(context, list);
  return NULL;
}

void hesiod_free_servent(void *context, struct servent *serv)
{
    free(serv->s_name);
    free(serv->s_aliases);
    free(serv);
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
