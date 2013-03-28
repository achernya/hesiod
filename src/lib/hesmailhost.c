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

/* This file contains hesiod_postoffice, which retrieves post-office
 * information for a user.
 */

static const char rcsid[] = "$Id: hesmailhost.c,v 1.9 1999-10-23 19:29:15 danw Exp $";

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
  while (!isspace((unsigned char)*p))
    p++;
  *p++ = 0;
  po->hesiod_po_host = p;
  while (!isspace((unsigned char)*p))
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
