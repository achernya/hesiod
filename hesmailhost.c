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

/* This file contains hes_postoffice, which retrieves post-office information
 * for a user.
 */

static char rcsid[] = "$Id: hesmailhost.c,v 1.7 1996-11-07 02:30:18 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <pwd.h>
#include "hesiod.h"

#define LINESIZE 80

extern int hes_errno;

struct hes_postoffice *hes_getmailhost(const char *user)
{
  static struct hes_postoffice ret;
  static char linebuf[LINESIZE];

  hes_init();
  hes_errno = hes_getmailhost_r(user, &ret, linebuf, LINESIZE);
  return (hes_errno == HES_ER_OK) ? &ret : NULL;
}

int hes_getmailhost_r(const char *user, struct hes_postoffice *ret,
		      char *linebuf, int bufsize)
{
  char *p, *vec[2];
  int status;

  /* Get the result, sanity-check it, and copy it into linebuf. */
  status = hes_resolve_r(user, "pobox", vec, 2);
  if (status != HES_ER_OK)
    return(status);
  if (*vec == NULL)
    return HES_ER_INVAL;
  if (strlen(*vec) > bufsize - 1)
    {
      free(*vec);
      return HES_ER_RANGE;
    }
  strcpy(linebuf, *vec);
  free(*vec);

  /* Break up linebuf into fields. */
  ret->po_type = linebuf;
  p = linebuf;
  while (!isspace(*p))
    p++;
  *p++ = 0;
  ret->po_host = p;
  while (!isspace(*p))
    p++;
  *p++ = 0;
  ret->po_name = p;

  return HES_ER_OK;
}
