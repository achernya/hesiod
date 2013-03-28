/* Copyright (c) 1996 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND INTERNET SOFTWARE CONSORTIUM DISCLAIMS
 * ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL INTERNET SOFTWARE
 * CONSORTIUM BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
 * SOFTWARE.
 */

/* Copyright 1996, 2000 by the Massachusetts Institute of Technology.
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

/* This file is part of the hesiod library.  It implements the core
 * portion of the hesiod resolver.
 *
 * This file is loosely based on an interim version of hesiod.c from
 * the BIND IRS library, which was in turn based on an earlier version
 * of this file.  Extensive changes have been made on each step of the
 * path.
 *
 * This implementation is not truly thread-safe at the moment because
 * it uses res_send() and accesses _res.
 */

static const char rcsid[] = "$Id: hesiod.c,v 1.30 2002-04-03 21:40:55 ghudson Exp $";

#include "config.h"
#include <sys/types.h>
#include <netinet/in.h>
#define BIND_4_COMPAT
#define BIND_8_COMPAT
#include <arpa/nameser.h>
#include <errno.h>
#include <netdb.h>
#include <resolv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#ifdef HAVE_LIBIDN
#include <idna.h>
#include <idn-free.h>
#endif
#include "hesiod.h"

/* A few operating systems don't define this. */
#ifndef T_TXT
#define T_TXT	16
#endif

/* Defaults if the configuration file is not present. */
#define DEF_RHS ".athena.mit.edu"
#define DEF_LHS ".ns"

/* Maximum size of a Hesiod response from the DNS. */
#define MAX_HESRESP 1024

/* The contents of a Hesiod context. */
struct hesiod_p {
  char *lhs;			/* normally ".ns" */
  char *rhs;			/* AKA the default hesiod domain */
};

char **hesiod__uidresolve(void *context, const char *uidstr);
static int read_config_file(struct hesiod_p *ctx, const char *filename);
static char **get_txt_records(struct hesiod_p *ctx, const char *name);
static int cistrcmp(const char *s1, const char *s2);

/* This function is called to initialize a hesiod_p. */
int hesiod_init(void **context)
{
  struct hesiod_p *ctx;
  const char *p, *configname;

  ctx = malloc(sizeof(struct hesiod_p));
  if (ctx)
    {
      *context = ctx;
      configname = ((getuid() == geteuid()) && (getgid() == getegid())) ? getenv("HESIOD_CONFIG") : NULL;
      if (!configname)
	configname = SYSCONFDIR "/hesiod.conf";
      if (read_config_file(ctx, configname) >= 0)
	{
	  /* The default rhs can be overridden by an environment variable. */
	  p = ((getuid() == geteuid()) && (getgid() == getegid())) ? getenv("HES_DOMAIN") : NULL;
	  if (p)
	    {
	      if (ctx->rhs)
		free(ctx->rhs);
	      ctx->rhs = malloc(strlen(p) + 2);
	      if (ctx->rhs)
		{
		  *ctx->rhs = '.';
		  strcpy(ctx->rhs + 1, (*p == '.') ? p + 1 : p);
		  return 0;
		}
	      else
		errno = ENOMEM;
	    }
	  else
	    return 0;
	}
    }
  else
    errno = ENOMEM;

  if (ctx->lhs)
    free(ctx->lhs);
  if (ctx->rhs)
    free(ctx->rhs);
  if (ctx)
    free(ctx);
  return -1;
}

/* This function deallocates the hesiod_p. */
void hesiod_end(void *context)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;

  free(ctx->rhs);
  if (ctx->lhs)
    free(ctx->lhs);
  free(ctx);
}

/* This function takes a hesiod (name, type) and returns a DNS
 * name which is to be resolved.
 */
char *hesiod_to_bind(void *context, const char *name, const char *type)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;
  char bindname[MAXDNAME], *p, *ret, *idn_ret, **rhs_list = NULL;
  const char *rhs;
  int len, rc;

  if (strlen(name) > sizeof(bindname) - 1)
    {
      errno = EMSGSIZE;
      return NULL;
    }
  strcpy(bindname, name);

  /* Find the right right hand side to use, possibly truncating bindname. */
  p = strchr(bindname, '@');
  if (p)
    {
      *p++ = 0;
      if (strchr(p, '.'))
	rhs = name + (p - bindname);
      else
	{
	  rhs_list = hesiod_resolve(context, p, "rhs-extension");
	  if (rhs_list)
	    rhs = *rhs_list;
	  else
	    {
	      errno = ENOENT;
	      return NULL;
	    }
	}
    } else
      rhs = ctx->rhs;

  /* See if we have enough room. */
  len = strlen(bindname) + 1 + strlen(type);
  if (ctx->lhs)
    len += strlen(ctx->lhs) + ((ctx->lhs[0] != '.') ? 1 : 0);
  len += strlen(rhs) + ((rhs[0] != '.') ? 1 : 0);
  if (len > sizeof(bindname) - 1)
    {
      if (rhs_list)
	hesiod_free_list(context, rhs_list);
      errno = EMSGSIZE;
      return NULL;
    }

  /* Put together the rest of the domain. */
  strcat(bindname, ".");
  strcat(bindname, type);
  if (ctx->lhs)
    {
      if (ctx->lhs[0] != '.')
	strcat(bindname, ".");
      strcat(bindname, ctx->lhs);
    }
  if (rhs[0] != '.')
    strcat(bindname, ".");
  strcat(bindname, rhs);

  /* rhs_list is no longer needed, since we're done with rhs. */
  if (rhs_list)
    hesiod_free_list(context, rhs_list);

  /* Make a copy of the result and return it to the caller. */
#ifdef HAVE_LIBIDN
  rc = idna_to_ascii_lz(bindname, &idn_ret, 0);
  if (rc != IDNA_SUCCESS)
    {
      errno = EINVAL;
      return NULL;
    }
  ret = strdup(idn_ret);
  idn_free(idn_ret);
#else
  ret = strdup(bindname);
#endif
  if (!ret)
    {
      errno = ENOMEM;
      return NULL;
    }
  return ret;
}

/* This is the core function.  Given a hesiod name and type, it
 * returns an array of strings returned by the resolver.
 */
char **hesiod_resolve(void *context, const char *name, const char *type)
{
  struct hesiod_p *ctx = (struct hesiod_p *) context;
  char *bindname, **retvec;

  bindname = hesiod_to_bind(context, name, type);
  if (!bindname)
    return NULL;

  retvec = get_txt_records(ctx, bindname);

  hesiod_free_string(context, bindname);
  return retvec;
}

void hesiod_free_list(void *context, char **list)
{
  char **p;

  for (p = list; *p; p++)
    free(*p);
  free(list);
}

void hesiod_free_string(void *context, char *str)
{
  free(str);
}

/* This function parses the /etc/hesiod.conf file.  Returns 0 on success,
 * -1 on failure.  On failure, it might leave values in ctx->lhs or
 * ctx->rhs which need to be freed by the caller. */
static int read_config_file(struct hesiod_p *ctx, const char *filename)
{
  char *key, *data, *p, **which;
  char buf[MAXDNAME + 7];
  FILE *fp;

  /* Try to open the configuration file. */
  fp = fopen(filename, "r");
  if (!fp)
    {
      /* Use compiled in default domain names. */
      ctx->lhs = malloc(strlen(DEF_LHS) + 1);
      ctx->rhs = malloc(strlen(DEF_RHS) + 1);
      if (ctx->lhs && ctx->rhs)
	{
	  strcpy(ctx->lhs, DEF_LHS);
	  strcpy(ctx->rhs, DEF_RHS);
	  return 0;
	}
      else
	{
	  errno = ENOMEM;
	  return -1;
	}
    }

  ctx->lhs = NULL;
  ctx->rhs = NULL;
  while (fgets(buf, sizeof(buf), fp) != NULL)
    {
      p = buf;
      if (*p == '#' || *p == '\n' || *p == '\r')
	continue;
      while(*p == ' ' || *p == '\t')
	p++;
      key = p;
      while(*p != ' ' && *p != '\t' && *p != '=')
	p++;
      *p++ = 0;
		
      while(isspace((unsigned char)*p) || *p == '=')
	p++;
      data = p;
      while(!isspace((unsigned char)*p))
	p++;
      *p = 0;

      if (cistrcmp(key, "lhs") == 0 || cistrcmp(key, "rhs") == 0)
	{
	  which = (cistrcmp(key, "lhs") == 0) ? &ctx->lhs : &ctx->rhs;
	  *which = malloc(strlen(data) + 1);
	  if (!*which)
	    {
	      errno = ENOMEM;
	      return -1;
	    }
	  strcpy(*which, data);
	}
    }
  fclose(fp);

  /* Make sure that the rhs is set. */
  if (!ctx->rhs)
    {
      errno = ENOEXEC;
      return -1;
    }

  return 0;
}	

/* Given a DNS class and a DNS name, do a lookup for TXT records, and
 * return a list of them.
 */
static char **get_txt_records(struct hesiod_p *ctx, const char *name)
{
  unsigned char qbuf[PACKETSZ], *abuf;
  char **tmp;
  int n, i, len;

  /* Make sure the resolver is initialized. */
  if ((_res.options & RES_INIT) == 0 && res_init() == -1)
    return NULL;

  /* Construct the query. */
  n = res_mkquery(QUERY, name, C_IN, T_TXT, NULL, 0, NULL, qbuf, PACKETSZ);
  if (n < 0)
    {
      errno = EMSGSIZE;
      return NULL;
    }

  /* Send the query. */
  abuf = NULL;
  len = 1024;
  i = n;
  do
    {
      abuf = realloc(abuf, len);
      if (abuf == NULL)
        {
          n = -1;
          break;
        }
      n = res_send(qbuf, i, abuf, len);
      if (n < len)
        {
          break;
        }
      len = n + 1024;
    } while(1);
  if (n < (ssize_t) sizeof(HEADER))
    {
      errno = ECONNREFUSED;
      free(abuf);
      return NULL;
    }

  tmp = hesiod_parse_result(ctx, abuf, n);

  free(abuf);

  return tmp;
}

char **hesiod_parse_result(void *ctx, const unsigned char *abuf, int alen)
{
  HEADER *hp;
  unsigned const char *p, *eom, *eor;
  char *dst, **list;
  int ancount, qdcount, i, j, skip, type, class, len, n;

  /* Parse the header of the result. */
  hp = (HEADER *) abuf;
  ancount = ntohs(hp->ancount);
  qdcount = ntohs(hp->qdcount);
  p = abuf + sizeof(HEADER);
  eom = abuf + alen;

  /* Skip questions, trying to get to the answer section which follows. */
  for (i = 0; i < qdcount; i++)
    {
      skip = dn_skipname(p, eom);
      if (skip < 0 || p + skip + QFIXEDSZ > eom)
	{
	  errno = EMSGSIZE;
	  return NULL;
	}
      p += skip + QFIXEDSZ;
    }

  /* Allocate space for the text record answers. */
  list = malloc((ancount + 1) * sizeof(char *));
  if (!list)
    {
      errno = ENOMEM;
      return NULL;
    }

  /* Parse the answers. */
  j = 0;
  for (i = 0; i < ancount; i++)
    {
      /* Parse the header of this answer. */
      skip = dn_skipname(p, eom);
      if (skip < 0 || p + skip + 10 > eom)
	break;
      type = p[skip + 0] << 8 | p[skip + 1];
      class = p[skip + 2] << 8 | p[skip + 3];
      len = p[skip + 8] << 8 | p[skip + 9];
      p += skip + 10;
      if (p + len > eom)
	{
	  errno = EMSGSIZE;
	  break;
	}

      /* Skip entries of the wrong type. */
      if (type != T_TXT)
	{
	  p += len;
	  continue;
	}

      /* Allocate space for this answer. */
      list[j] = malloc(len);
      if (!list[j])
	{
	  errno = ENOMEM;
	  break;
	}
      dst = list[j++];

      /* Copy answer data into the allocated area. */
      eor = p + len;
      while (p < eor)
	{
	  n = (unsigned char) *p++;
	  if (p + n > eor)
	    {
	      errno = EMSGSIZE;
	      break;
	    }
	  memcpy(dst, p, n);
	  p += n;
	  dst += n;
	}
      if (p < eor)
	{
	  errno = EMSGSIZE;
	  break;
	}
      *dst = 0;
    }

  /* If we didn't terminate the loop normally, something went wrong. */
  if (i < ancount)
    {
      for (i = 0; i < j; i++)
	free(list[i]);
      free(list);
      return NULL;
    }

  if (j == 0)
    {
      errno = ENOENT;
      free(list);
      return NULL;
    }

  list[j] = NULL;
  return list;
}

static int cistrcmp(const char *s1, const char *s2)
{
  while (*s1 && *s2 && tolower(*s1) == tolower(*s2))
    {
      s1++;
      s2++;
    }
  return tolower(*s1) - tolower(*s2);
}
