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

/* This file is part of the Hesiod library.  It implements routines
 * for communicating with the name servers and interpreting their
 * results.
 */

static char rcsid[] = "$Id: resolve.c,v 1.11 1996-11-07 02:39:56 ghudson Exp $";

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "resscan.h"

static nsmsg_t *res_scan(const char *msg, int msg_len, char *nmsgbuf,
			 char *databuf);
static int dn_skip(const char *comp_dn);
static const char *rr_scan(const char *cp, rr_t *rr);

/* Resolve name into data records. */
nsmsg_t *hes__resolve(const char *name, int class, int type, char *nmsgbuf,
		      char *databuf)
{
  char qbuf[PACKETSZ], abuf[8192];
  int n;

  if (class >= 0 && type >= 0)
    {
      n = res_mkquery(QUERY, name, class, type, NULL, 0, NULL, qbuf, PACKETSZ);
      if (n >= 0)
	{
	  n = res_send(qbuf, n, abuf, sizeof(abuf));
	  if (n >= 0)
	    return res_scan(abuf, n, nmsgbuf, databuf);
	  else
	    errno = ECONNREFUSED;
	} else
	  errno = EMSGSIZE;
    } else
      errno = EINVAL;
  return NULL;
}

static nsmsg_t *res_scan(const char *msg, int msg_len, char *nmsgbuf,
			 char *databuf)
{
  const char *p;
  rr_t *rp;
  HEADER *hp = (HEADER *) msg;
  char *data = databuf;
  int i, n_an, n_ns, n_ar, nrec;
  nsmsg_t *mess = (nsmsg_t *) nmsgbuf;

  p = msg + sizeof(HEADER);
  n_an = ntohs(hp->ancount);
  n_ns = ntohs(hp->nscount);
  n_ar = ntohs(hp->arcount);
  nrec = n_an + n_ns + n_ar;

  mess->len = 0;
  mess->hd = hp;
  mess->ns_off = n_an;
  mess->ar_off = n_an + n_ns;
  mess->count = nrec;
  rp = &mess->rr;

  /* Skip over questions. */
  for (i = 0; i < ntohs(hp->qdcount); i++)
    {
      i = dn_skip(p);
      if (i < 0)
	return NULL;
      p += i + 4;
    }

  /* Scan answers, name servers, and additional records. */
  for (i = 0; i < n_an + n_ns + n_ar && p < msg + msg_len; i++)
    {
      p = rr_scan(p, rp);
      if (p == NULL || data + rp->dlen > databuf + DATASIZE)
	return NULL;
      memcpy(data, rp->data, rp->dlen);
      rp->data = data;
      data += rp->dlen;
      *data++ = 0;
      rp++;
    }

  mess->len = p - msg;

  return mess;
}

/* Skip over a compressed domain name.  Return the size or -1. */
static int dn_skip(const char *comp_dn)
{
  const char *p;

  p = comp_dn;
  while (*p && (*p & INDIR_MASK) == 0)
    p += *p + 1;
  if (*p && (*p & INDIR_MASK) != INDIR_MASK)
    return -1;
  return p + (*p ? 2 : 1) - comp_dn;
}

static const char *rr_scan(const char *p, rr_t *rr)
{
  int n;

  n = dn_skip(p);
  if (n >= 0)
    {
      rr->type = p[n] << 8 | p[n + 1];
      rr->class = p[n + 2] << 8 | p[n + 3];
      rr->dlen = p[n + 8] << 8 | p[n + 9];
      rr->data = p + n + 10;
      return rr->data + rr->dlen;
    }
  errno = EINVAL;
  return NULL;
}
