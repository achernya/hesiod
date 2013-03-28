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

/* This file is a simple driver for the Hesiod library. */

static const char rcsid[] = "$Id: hesinfo.c,v 1.5 2000-01-05 22:00:46 ghudson Exp $";

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include "hesiod.h"

extern int optind;

int main(int argc, char **argv)
{
  char **list, **p, *bindname, *name, *type;
  int lflag = 0, errflg = 0, bflag = 0, c;
  void *context;

  while ((c = getopt(argc, argv, "lb")) != EOF)
    {
      switch (c)
	{
	case 'l':
	  lflag = 1;
	  break;
	case 'b':
	  bflag = 1;
	  break;
	default:
	  errflg++;
	  break;
	}
    }
  if (argc - optind != 2 || errflg)
    {
      fprintf(stderr,"Usage: %s [-bl] name type\n",argv[0]);
      fprintf(stderr,"\t-l selects long format\n");
      fprintf(stderr,"\t-b also does hes_to_bind conversion\n");
      return 2;
    }

  name = argv[optind];
  type = argv[optind + 1];

  if (hesiod_init(&context) < 0)
    {
      if (errno == ENOEXEC)
	fprintf(stderr, "hesiod_init: Invalid Hesiod configuration file.\n");
      else
	perror("hesiod_init");
    }

  /* Display bind name if requested. */
  if (bflag)
    {
      if (lflag)
	printf("hes_to_bind(%s, %s) expands to\n", name, type);
      bindname = hesiod_to_bind(context, name, type);
      if (!bindname)
	{
	  if (lflag)
	    printf("nothing\n");
	  if (errno == ENOENT)
	    fprintf(stderr, "hesiod_to_bind: Unknown rhs-extension.\n");
	  else
	    perror("hesiod_to_bind");
	  exit(1);
	}
      printf("%s\n", bindname);
      hesiod_free_string(context, bindname);
      if (lflag)
	printf("which ");
    }

  if (lflag) 
    printf("resolves to\n");

  /* Do the hesiod resolve and check for errors. */
  list = hesiod_resolve(context, name, type);
  if (!list)
    { 
      if (lflag)
	printf("nothing\n");
      if (errno == ENOENT)
	fprintf(stderr, "hesiod_resolve: Hesiod name not found.\n");
      else
	perror("hesiod_resolve");
      return 1;
    }

  /* Display the results. */
  for (p = list; *p; p++)
    printf("%s\n", *p);

  hesiod_free_list(context, list);
  hesiod_end(context);
  return 0;
}
