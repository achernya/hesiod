/* $Id: resscan.h,v 1.6 1996-11-07 02:43:27 ghudson Exp $ */

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

/* Original version by Steve Dyer, IBM/Project Athena, and Sam Hsu,
 * DEC/Project Athena.
 */

/* This file is part of the Hesiod library.  It contains definitions
 * for the name resolver scanning routines used by the library.
 */

typedef struct rr {
  short type;			/* RR type */
  short class;			/* RR class */
  int dlen;			/* len of data section */
  const char *data;		/* pointer to data */
} rr_t;

typedef struct nsmsg {
  int len;			/* sizeof(msg) */
  int ns_off;			/* offset to name server RRs */
  int ar_off;			/* offset to additional RRs */
  int count;			/* total number of RRs */
  HEADER *hd;			/* message header */
  rr_t rr;			/* vector of (stripped-down) RR descriptors */
} nsmsg_t;

#define DATASIZE 8192
#define NMSGSIZE (sizeof(nsmsg_t) + \
		  sizeof(rr_t) * (DATASIZE / RRFIXEDSZ))

nsmsg_t *hes__resolve(const char *name, int class, int type, char *nmsgbuf,
		      char *databuf);
