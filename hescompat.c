/* Copyright 1996 by the Massachusetts Institute of Technology.
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

/* This file is part of the Hesiod library.  It implements
 * backward-compatibility interfaces.
 */

static char rcsid[] = "$Id: hescompat.c,v 1.1 1996-12-08 21:28:41 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "hesiod.h"

static int inited = 0;
static void *context;
static char *bindname, **list;
static struct passwd *pw;
static struct servent *serv;
static struct hesiod_postoffice *po;
static struct hes_postoffice compatpo;
static int errval = HES_ER_UNINIT;

static int init_context(void);
static void translate_errors(void);

char *hes_to_bind(const char *name, const char *type)
{
  if (init_context() < 0)
    return NULL;
  if (bindname)
    free(bindname);
  bindname = hesiod_to_bind(context, name, type);
  if (!bindname)
    translate_errors();
  return bindname;
}

char **hes_resolve(const char *name, const char *type)
{
  if (init_context() < 0)
    return NULL;

  /* In the old Hesiod interface, the caller was responsible for freeing
   * the returned strings but not the vector of strings itself.
   */
  if (list)
    free(list);

  list = hesiod_resolve(context, name, type);
  if (!list)
    translate_errors();
  return list;
}

int hes_error(void)
{
  return errval;
}

struct passwd *hes_getpwnam(const char *name)
{
  if (init_context() < 0)
    return NULL;
  if (pw)
    hesiod_free_passwd(context, pw);
  pw = hesiod_getpwnam(context, name);
  if (!pw)
    translate_errors();
  return pw;
}

struct passwd *hes_getpwuid(uid_t uid)
{
  if (init_context() < 0)
    return NULL;
  if (pw)
    hesiod_free_passwd(context, pw);
  pw = hesiod_getpwuid(context, uid);
  if (!pw)
    translate_errors();
  return pw;
}

struct servent *hes_getservbyname(const char *name, const char *proto)
{
  if (init_context() < 0)
    return NULL;
  if (serv)
    hesiod_free_servent(context, serv);
  serv = hesiod_getservbyname(context, name, proto);
  if (!serv)
    translate_errors();
  return serv;
}

struct hes_postoffice *hes_getmailhost(const char *name)
{
  if (init_context() < 0)
    return NULL;
  if (po)
    hesiod_free_postoffice(context, po);
  po = hesiod_getmailhost(context, name);
  if (!po)
    {
      translate_errors();
      return NULL;
    }
  compatpo.po_type = po->hesiod_po_type;
  compatpo.po_host = po->hesiod_po_host;
  compatpo.po_name = po->hesiod_po_name;
  return &compatpo;
}

static int init_context()
{
  if (!inited)
    {
      inited = 1;
      if (hesiod_init(&context) < 0)
	{
	  errval = HES_ER_CONFIG;
	  return -1;
	}
      errval = HES_ER_OK;
    }
  return 0;
}

static void translate_errors()
{
  switch (errno)
    {
    case ENOENT:
      errval = HES_ER_NOTFOUND;
      break;
    case ECONNREFUSED:
    case EMSGSIZE:
      errval = HES_ER_NET;
      break;
    case ENOMEM:
    default:
      /* Not a good match, but the best we can do. */
      errval = HES_ER_CONFIG;
      break;
    }
}
