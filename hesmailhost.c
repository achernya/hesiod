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

/* This file contains hesiod_postoffice, which retrieves post-office
 * information for a user.
 */

static char rcsid[] = "$Id: hesmailhost.c,v 1.8 1996-12-08 21:40:32 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <pwd.h>
#include "hesiod.h"

struct hesiod_postoffice *hesiod_getmailhost(void *context, const char *user)
{
  char *p, **list;
  struct hesiod_postoffice *po;

  /* Get the result, sanity-check it, and copy it into linebuf. */
  list = hesiod_resolve(context, user, "pobox");
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
  po = (struct hesiod_postoffice *) malloc(sizeof(struct hesiod_postoffice));
  if (!po)
    {
      free(p);
      errno = ENOMEM;
      return NULL;
    }

  /* Break up linebuf into fields. */
  po->hesiod_po_type = p;
  while (!isspace(*p))
    p++;
  *p++ = 0;
  po->hesiod_po_host = p;
  while (!isspace(*p))
    p++;
  *p++ = 0;
  po->hesiod_po_name = p;

  return po;
}

void hesiod_free_postoffice(void *context, struct hesiod_postoffice *po)
{
    free(po->hesiod_po_type);
    free(po);
}
