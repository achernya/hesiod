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

/* This file is part of the Hesiod library.  It implements the main
 * interface routines.
 */

static char rcsid[] = "$Id: hesiod.c,v 1.17 1996-11-08 04:04:18 ghudson Exp $";

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include <pwd.h>
#include <stdlib.h>
#include <ctype.h>
#include "resscan.h"
#include "hesiod.h"

/* Make sure none of these are longer than HES_MAX_ERRLEN - 1. */
static char *error_table[] = {
  "No error",
  "Hesiod library uninitialized",
  "Hesiod name not found by server",
  "Invalid configuration file",
  "Network problem",
  "Return buffer not large enough",
  "Insufficient memory is available",
  "Invalid response from hesiod server"
};

static char *hes_lhs;
static char *hes_rhs;
int hes_errno = HES_ER_UNINIT;

int hes_init()
{
  FILE *fp;
  char *key, *p, **which;
  int len;
  char buf[MAXDNAME + 7];

  if (hes_errno != HES_ER_UNINIT && hes_errno != HES_ER_CONFIG)
    return HES_ER_OK;
  res_init();
  hes_errno = HES_ER_UNINIT;
  hes_lhs = NULL;
  hes_rhs = NULL;

  /* Look for hesiod.conf in /etc, then fall back to hesiod.conf. */
  fp = fopen("/etc/hesiod.conf", "r");
  if (fp == NULL)
    fp = fopen(SYSCONFDIR "/hesiod.conf", "r");

  if (fp == NULL)
    {
      /* Use compiled-in defaults. */
      hes_lhs = DEF_LHS;
      hes_rhs = DEF_RHS;
    }
  else
    {
      while (fgets(buf, MAXDNAME + 7, fp) != NULL)
	{
	  p = buf;
	  if (*p == '#' || *p == '\n')
	    continue;
	  while (isspace(*p))
	    p++;
	  key = p;
	  while (!isspace(*p) && *p != '=')
	    p++;
	  *p++ = '\0';
	  if (strcmp(key, "lhs") == 0)
	    which = &hes_lhs;
	  else if (strcmp(key, "rhs") == 0)
	    which = &hes_rhs;
	  else
	    continue;
	  while (isspace(*p) || *p == '=')
	    p++;
	  if (*p != '.' && *p != '\n')
	    {
	      hes_errno = HES_ER_CONFIG;
	      fclose(fp);
	      return hes_errno;
	    }
	  len = strlen(p);
	  *which = malloc(len);
	  if (*which == NULL)
	    {
	      hes_errno = HES_ER_NOMEM;
	      fclose(fp);
	      return hes_errno;
	    }
	  strncpy(*which, p, len - 1);
	  p[len - 1] = 0;
	}
      fclose(fp);
    }

  /* See if the RHS is overridden by environment variable. */
  p = getenv("HES_DOMAIN");
  if (p)
    {
      hes_rhs = malloc(strlen(p) + 1);
      if (hes_rhs == NULL)
	{
	  hes_errno = HES_ER_NOMEM;
	  return hes_errno;
	}
      strcpy(hes_rhs, p);
    }

  /* The LHS may be null, but the RHS must not be. */
  hes_errno = (hes_rhs == NULL) ? HES_ER_CONFIG : HES_ER_OK;
  return hes_errno;
}

char *hes_to_bind(const char *name, const char *type)
{
  static char buffer[MAXDNAME];

  if (hes_errno == HES_ER_UNINIT || hes_errno == HES_ER_CONFIG)
    hes_init();
  if (hes_errno == HES_ER_CONFIG)
    return NULL;
  hes_errno = hes_to_bind_r(name, type, buffer, MAXDNAME);
  return (hes_errno == HES_ER_OK) ? buffer : NULL;
}

int hes_to_bind_r(const char *name, const char *type, char *buffer,
		  int bufsize)
{
  char *p, *vec[2], *rhs;
  int len;

  *vec = NULL;
  if (hes_errno == HES_ER_CONFIG)
    return HES_ER_CONFIG;
  if (hes_errno == HES_ER_UNINIT)
    {
      fprintf(stderr, "Library not initialized in hes_to_bind_r().\n");
      abort();
    }
  p = strchr(name, '@');
  if (p)
    {
      if (p - name > bufsize - 1)
	return HES_ER_RANGE;
      if (strchr(p + 1, '.'))
	rhs = p + 1;
      else
	{
	  if (hes_resolve_r(p + 1, "rhs-extension", vec, 2) == HES_ER_OK
	      && *vec != NULL)
	    rhs = *vec;
	  else
	    return HES_ER_NOTFOUND;
	}
      strncpy(buffer, name, p - name);
      buffer[p - name] = '\0';
    }
  else
    {
      rhs = hes_rhs;
      if (strlen(name) > bufsize - 1)
	return HES_ER_RANGE;
      strcpy(buffer, name);
    }
  len = strlen(buffer) + 1 + strlen(type);
  if (hes_lhs)
    len += strlen(hes_lhs) + ((hes_lhs[0] != '.') ? 1 : 0);
  len += strlen(rhs) + ((rhs[0] != '.') ? 1 : 0);
  if (len > bufsize - 1)
    {
      if (*vec)
	free(*vec);
      return HES_ER_RANGE;
    }
  strcat(buffer, ".");
  strcat(buffer, type);
  if (hes_lhs)
    {
      if (hes_lhs[0] != '.')
	strcat(buffer, ".");
      strcat(buffer, hes_lhs);
    }
  if (rhs[0] != '.')
    strcat(buffer, ".");
  strcat(buffer, rhs);
  if (*vec)
    free(*vec);
  return HES_ER_OK;
}

char **hes_resolve(const char *name, const char *type)
{
  static char *retvec[100];

  if (hes_errno == HES_ER_UNINIT || hes_errno == HES_ER_CONFIG)
    hes_init();
  if (hes_errno == HES_ER_CONFIG)
    return NULL;
  hes_errno = hes_resolve_r(name, type, retvec, 100);
  return (hes_errno == HES_ER_OK) ? retvec : NULL;
}

int hes_resolve_r(const char *name, const char *type, char **retvec,
		  int retveclen)
{
  const char *p;
  char *dest, **vec;
  char buffer[MAXDNAME], nmsgbuf[NMSGSIZE], databuf[DATASIZE];
  int status, n;
  struct nsmsg *ns;
  rr_t *rp;

  if (hes_errno == HES_ER_CONFIG)
    return HES_ER_CONFIG;
  if (hes_errno == HES_ER_UNINIT)
    {
      fprintf(stderr, "Library not initialized in hes_resolve_r().\n");
      abort();
    }
  status = hes_to_bind_r(name, type, buffer, MAXDNAME);
  if (status != HES_ER_OK)
    return status;
  p = buffer;
  errno = 0;
  ns = hes__resolve(p, C_HS, T_TXT, nmsgbuf, databuf);
  if (ns == NULL && (errno == ETIMEDOUT || errno == ECONNREFUSED))
    return HES_ER_NET;
  if (ns == NULL || ns->ns_off <= 0)
    return HES_ER_NOTFOUND;
  vec = retvec;
  for (rp = &ns->rr; rp < &ns->rr + ns->ns_off; rp++)
    {
      if (vec - retvec >= retveclen - 1)
	{
	  for (; vec > retvec; vec--)
	    free(*(vec - 1));
	  return HES_ER_RANGE;
	}
      /* XXX we're being way too trusting here */
      if (rp->class == C_HS && rp->type == T_TXT)
	{
	  *vec = malloc(rp->dlen + 1);
	  if (*vec == NULL)
	    {
	      for (; vec > retvec; vec--)
		free(*(vec - 1));
	      return HES_ER_NOMEM;
	    }
	  dest = *vec;
	  p = rp->data;
	  while (p < rp->data + rp->dlen)
	    {
	      n = *p++;
	      memcpy(dest, p, n);
	      p += n;
	      dest += n;
	    }
	  *dest = 0;
	  vec++;
	}
    }
  *vec = 0;
  return HES_ER_OK;
}

int hes_error()
{
  return hes_errno;
}

const char *hes_strerror(int errval)
{
  static char buf[HES_MAX_ERRLEN];

  hes_strerror_r(errval, buf, sizeof(buf));
  return buf;
}

int hes_strerror_r(int errval, char *buf, int bufsize)
{
  if (errval < 0 || errval >= sizeof(error_table))
    {
      if (bufsize >= 32)
	sprintf(buf, "Unknown error %d", errval);
      else
	return -1;
    }
  else
    {
      if (bufsize > strlen(error_table[errval]))
	strcpy(buf, error_table[errval]);
      else
	return -1;
    }
  return 0;
}
