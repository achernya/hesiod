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

/* This file is a simple driver for the Hesiod library. */

static char rcsid[] = "$Id: hesinfo.c,v 1.7 1996-11-07 02:25:05 ghudson Exp $";

#include <stdio.h>
#include "hesiod.h"

extern int optind;

int main(int argc, char **argv)
{
  char errbuf[HES_MAX_ERRLEN], buf[1024], *vec[128], **p, *identifier, *type;
  int lflag = 0, errflg = 0, bflag = 0, c, status;

  while ((c = getopt(argc, argv, "lb")) != EOF)
    {
      switch (c)
	{
	case 'l':
	  lflag = 1;
	  break;
	case 'c':
	  bflag = 1;
	  break;
	default:
	  errflg++;
	}
    }
  if (argc - optind != 2 || errflg)
    {
      fprintf(stderr,"Usage: %s [-bl] identifier type\n",argv[0]);
      fprintf(stderr,"	-l selects long format\n");
      fprintf(stderr,"	-b also does hes_to_bind conversion\n");
      return 2;
    }

  identifier = argv[optind];
  type = argv[optind + 1];

  hes_init();

  /* Display bind name if requested. */
  if (bflag)
    {
      if (lflag)
	printf("hes_to_bind(%s, %s) expands to\n", identifier, type);
      status = hes_to_bind_r(identifier, type, buf, sizeof(buf));
      if (status != HES_ER_OK)
	{
	  if (lflag)
	    printf("nothing\n");
	  hes_strerror_r(status, errbuf, sizeof(errbuf));
	  fprintf(stderr, "%s\n", errbuf);
	  exit(1);
	}
      printf("%s\n", buf);
      if (lflag)
	printf("which ");
    }

  if (lflag) 
    printf("resolves to\n");

  /* Do the hesiod resolve and check for errors. */
  status = hes_resolve_r(identifier, type, vec, sizeof(vec));
  if (status != HES_ER_OK)
    { 
      if (lflag)
	printf("nothing\n");
      hes_strerror_r(status, errbuf, sizeof(errbuf));
      fprintf(stderr, "%s\n", errbuf);
      return 1;
    }

  /* Display the results. */
  for (p = vec; *p; p++)
    printf("%s\n", *p);
  return 0;
}
