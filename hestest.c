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

/* This file is a test driver for the Hesiod library. */

static char rcsid[] = "$Id: hestest.c,v 1.2 1996-12-08 21:41:45 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <pwd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include "hesiod.h"

static char *word_end(char *s);
static char *find_word(char *s);
static char *get_word(char *p, char *buf);
static char *get_field(char *p, int delim, char *buf);
static int compare_vector(char **vector, char *spec);
static int compare_pwnam(struct passwd *pw, char *spec);
static int compare_serv(struct servent *serv, char *spec);
static int compare_office(struct hesiod_postoffice *office, char *spec);
static int compare_compat_office(struct hes_postoffice *office, char *spec);
static void free_ptrs(char **ptrs);
static void failure(const char *fmt, ...);

int saw_failure = 0;

int main(int argc, char **argv)
{
  FILE *fp;
  char buf[1024], buf2[1024], *p, *q, name[128], type[128], proto[128], **list;
  int line, errval;
  struct passwd *pw;
  struct servent *serv;
  struct hesiod_postoffice *po;
  struct hes_postoffice *compatpo;
  void *context;

  if (argc != 2)
    {
      fprintf(stderr, "Usage: %s filename\n", argv[0]);
      return 1;
    }

  fp = fopen(argv[1], "r");
  if (!fp)
    {
      fprintf(stderr, "Couldn't open %s for reading.\n", argv[1]);
      return 1;
    }

  if (hesiod_init(&context) < 0)
    {
      fprintf(stderr, "Can't initialize hesiod library.\n");
      return 1;
    }

  line = 0;
  while (fgets(buf, sizeof(buf), fp))
    {
      line++;

      /* Strip off trailing spaces (inefficiently). */
      while (isspace(buf[strlen(buf) - 1]))
	buf[strlen(buf) - 1] = 0;

      /* Get the first word, discard comment lines and invalid lines. */
      q = word_end(p = find_word(buf));
      if (*p == '#' || !*p)
	continue;

      if (*q && q - p == 7 && strncmp(p, "resolve", 7) == 0)
	{
	  /* Test for hesiod_resolve and hes_resolve. */
	  q = get_word(find_word(q), name);
	  q = get_word(find_word(q), type);
	  p = find_word(q);
	  list = hesiod_resolve(context, name, type);
	  errval = errno;
	  if (!list && !(*p == 'E' && p[1] == 0))
	    {
	      failure("Line %d failed (hesiod_resolve error %d).\n", line,
		      errval);
	      continue;
	    }
	  if (list && compare_vector(list, p) < 0)
	    {
	      failure("Line %d failed in hesiod_resolve().\n", line);
	      hesiod_free_list(context, list);
	      continue;
	    }
	  if (list)
	    hesiod_free_list(context, list);
	  list = hes_resolve(name, type);
	  if (!list)
	    {
	      if (*p == 'E' && p[1] == 0)
		printf("Line %d passed (%d, %d).\n", line, errval,
		       hes_error());
	      else
		failure("Line %d failed (hes_resolve error %d).\n", line,
			hes_error());
	      continue;
	    }
	  if (compare_vector(list, p) < 0)
	    failure("Line %d failed in hes_resolve().\n", line);
	  else
	    printf("Line %d passed.\n", line);
	  free_ptrs(list);
	}
      else if (*q && q - p == 8 && strncmp(p, "getpwnam", 8) == 0)
	{
	  /* Test for hesiod_getpwnam and hes_getpwnam. */
	  q = get_word(find_word(q), name);
	  p = find_word(q);
	  pw = hesiod_getpwnam(context, name);
	  errval = errno;
	  if (!pw && !(*p == 'E' && p[1] == 0))
	    {
	      failure("Line %d failed (hesiod_getpwnam error %d).\n", line,
		      errval);
	      continue;
	    }
	  if (pw && compare_pwnam(pw, p) < 0)
	    {
	      failure("Line %d failed in hesiod_getpwnam().\n", line);
	      continue;
	    }
	  if (pw)
	    hesiod_free_passwd(context, pw);
	  pw = hes_getpwnam(name);
	  if (!pw)
	    {
	      if (*p == 'E' && p[1] == 0)
		printf("Line %d passed (%d, %d).\n", line, errval,
		       hes_error());
	      else
		failure("Line %d failed (hes_getpwnam error %d).\n", line,
			hes_error());
	      continue;
	    }
	  if (compare_pwnam(pw, p) < 0)
	    failure("Line %d failed in hes_getpwnam().\n", line);
	  else
	    printf("Line %d passed.\n", line);
	}
      else if (*q && q - p == 8 && strncmp(p, "getpwuid", 8) == 0)
	{
	  /* Test for hesiod_getpwuid and hes_getpwuid. */
	  q = get_word(find_word(q), name);
	  p = find_word(q);
	  pw = hesiod_getpwuid(context, atoi(name));
	  errval = errno;
	  if (!pw && !(*p == 'E' && p[1] == 0))
	    {
	      failure("Line %d failed (hesiod_getpwuid error %d).\n", line,
		      errval);
	      continue;
	    }
	  if (pw && compare_pwnam(pw, p) < 0)
	    {
	      failure("Line %d failed in hesiod_getpwuid().\n", line);
	      continue;
	    }
	  if (pw)
	    hesiod_free_passwd(context, pw);
	  pw = hes_getpwuid(atoi(name));
	  if (!pw)
	    {
	      if (*p == 'E' && p[1] == 0)
		printf("Line %d passed (%d, %d).\n", line, errval,
		       hes_error());
	      else
		failure("Line %d failed (hes_getpwuid error %d).\n", line,
			hes_error());
	      continue;
	    }
	  if (compare_pwnam(pw, p) < 0)
	    failure("Line %d failed in hes_getpwuid().\n", line);
	  else
	    printf("Line %d passed.\n", line);
	}
      else if (*q && q - p == 13 && strncmp(p, "getservbyname", 13) == 0)
	{
	  /* Test for hesiod_getservbyname and hes_getservbyname. */
	  q = get_word(find_word(q), name);
	  q = get_word(find_word(q), proto);
	  p = find_word(q);
	  serv = hesiod_getservbyname(context, name, proto);
	  errval = errno;
	  if (!serv && !(*p == 'E' && p[1] == 0))
	    {
	      failure("Line %d failed (hesiod_getservbyname error %d).\n",
		      line, errno);
	      continue;
	    }
	  if (serv && compare_serv(serv, p) < 0)
	    {
	      failure("Line %d failed in hesiod_getservbyname.\n", line);
	      continue;
	    }
	  if (serv)
	    hesiod_free_servent(context, serv);
	  serv = hes_getservbyname(name, proto);
	  if (!serv)
	    {
	      if (*p == 'E' && p[1] == 0)
		printf("Line %d passed (%d, %d).\n", line, errval,
		       hes_error());
	      else
		failure("Line %d failed (hes_getservbyname error %d).\n", line,
			hes_error());
	      continue;
	    }
	  if (compare_serv(serv, p) < 0)
	    failure("Line %d failed in hes_getservbyname().\n", line);
	  else
	    printf("Line %d passed.\n", line);
	}
      else if (*q && q - p == 11 && strncmp(p, "getmailhost", 11) == 0)
	{
	  /* Test for hesiod_getmailhost and hes_getmailhost. */
	  q = get_word(find_word(q), name);
	  p = find_word(q);
	  po = hesiod_getmailhost(context, name);
	  errval = errno;
	  if (!po && !(*p == 'E' && p[1] == 0))
	    {
	      failure("Line %d failed (hesiod_getmailhost error %d).\n",
		      line, errval);
	      continue;
	    }
	  if (po && compare_office(po, p) < 0)
	    {
	      failure("Line %d failed in hesiod_getmailhost.\n", line);
	      continue;
	    }
	  if (po)
	    hesiod_free_postoffice(context, po);
	  compatpo = hes_getmailhost(name);
	  if (!compatpo)
	    {
	      if (*p == 'E' && p[1] == 0)
		printf("Line %d passed (%d, %d).\n", line, errval,
		       hes_error());
	      else
		failure("Line %d failed (hes_getmailhost error %d).\n", line,
			hes_error());
	      continue;
	    }
	  if (compare_compat_office(compatpo, p) < 0)
	    failure("Line %d failed in hes_getmailhost().\n", line);
	  else
	    printf("Line %d passed.\n", line);

	}
      else
	{
	  fprintf(stderr, "Line %d invalid: %s\n", line, buf);
	  return 2;
	}
    }

  return saw_failure;
}

static char *word_end(char *s)
{
  while (*s && !isspace(*s))
    s++;
  return s;
}

static char *find_word(char *s)
{
  while (isspace(*s))
    s++;
  return s;
}

static char *get_word(char *p, char *buf)
{
  char *q = word_end(p);

  strncpy(buf, p, q - p);
  buf[q - p] = 0;
  return q;
}

static char *get_field(char *p, int delim, char *buf)
{
  char *q = strchr(p, delim);

  if (q) {
    strncpy(buf, p, q - p);
    buf[q - p] = 0;
    return q + 1;
  } else {
    strcpy(buf, p);
    return NULL;
  }
}

static int compare_vector(char **vector, char *spec)
{
  char field[100];

  for (; *vector; vector++) {
    spec = get_field(spec, '\\', field);
    if ((!spec && vector[1]) || strcmp(*vector, field) != 0)
      return -1;
  }
  return (spec) ? -1 : 0;
}

static int compare_pwnam(struct passwd *pw, char *spec)
{
  char field[100];

  spec = get_field(spec, ':', field);
  if (!spec || strcmp(pw->pw_name, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(pw->pw_passwd, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (pw->pw_uid != atoi(field))
    return -1;
  spec = get_field(spec, ':', field);
  if (pw->pw_gid != atoi(field))
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(pw->pw_gecos, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(pw->pw_dir, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (spec || strcmp(pw->pw_shell, field) != 0)
    return -1;
  return 0;
}

static int compare_serv(struct servent *serv, char *spec)
{
  char field[100], **aliases;

  spec = get_field(spec, ':', field);
  if (!spec || strcmp(serv->s_name, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(serv->s_proto, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (serv->s_port != htons(atoi(field)))
    return -1;
  for (aliases = serv->s_aliases; *aliases; aliases++)
    {
      if (!spec)
	return -1;
      spec = get_field(spec, '\\', field);
      if (strcmp(*aliases, field) != 0)
	return -1;
    }
  return (spec) ? -1 : 0;
}

static int compare_office(struct hesiod_postoffice *office, char *spec)
{
  char field[100];

  spec = get_field(spec, ':', field);
  if (!spec || strcmp(office->hesiod_po_type, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(office->hesiod_po_host, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (spec || strcmp(office->hesiod_po_name, field) != 0)
    return -1;
  return 0;
}

static int compare_compat_office(struct hes_postoffice *office, char *spec)
{
  char field[100];

  spec = get_field(spec, ':', field);
  if (!spec || strcmp(office->po_type, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (!spec || strcmp(office->po_host, field) != 0)
    return -1;
  spec = get_field(spec, ':', field);
  if (spec || strcmp(office->po_name, field) != 0)
    return -1;
  return 0;
}

static void free_ptrs(char **ptrs)
{
  for (; *ptrs; ptrs++)
    free(*ptrs);
}

static void failure(const char *fmt, ...)
{
  va_list ap;

  saw_failure = 1;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}
