/* This is the source code for the hesinfo program, used to test the
 * Hesiod name server.
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesinfo.c,v $
 *	$Author: probe $
 *	$Athena: hesinfo.c,v 1.4 88/08/07 21:52:19 treese Locked $
 *	$Log: not supported by cvs2svn $
 * Revision 1.5  88/08/07  23:16:50  treese
 * Second-public-distribution
 * 
 * Revision 1.4  88/08/07  21:52:19  treese
 * First public distribution
 * 
 * Revision 1.3  88/06/12  00:52:34  treese
 * Cleaned up to work with Saber.
 * First public distribution.
 * 
 * Revision 1.2  88/06/05  19:51:18  treese
 * Cleaned up for public distribution
 * 
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.  See the
 * file <mit-copyright.h> for copying and distribution information.
 */

#include "mit-copyright.h"

#ifndef lint
static char rcsid_hesinfo_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesinfo.c,v 1.6 1991-01-21 12:35:27 probe Exp $";
#endif

#include <stdio.h>
#include <hesiod.h>

main(argc, argv)
char *argv[];
{
	register char *cp, **cpp;
	char *hes_to_bind(), **hes_resolve();
	int lflag = 0, errflg = 0, bflag = 0;
	extern int optind;
	char *identifier, *type;
	int c;
	
	while ((c = getopt(argc, argv, "lb")) != EOF) {
		if (c == 'l') lflag = 1;
		else if (c == 'b') bflag = 1;
		else errflg++;
	}
	if (argc - optind != 2 || errflg) {
		fprintf(stderr,"Usage: %s [-bl] identifier type\n",argv[0]);
		fprintf(stderr,"	-l selects long format\n");
		fprintf(stderr,"	-b also does hes_to_bind conversion\n");
		exit(2);
	}
	identifier = argv[optind];
	type = argv[optind+1];
		
	if (bflag) {
		if (lflag)
			printf("hes_to_bind(%s, %s) expands to\n", 
			       identifier, type);
		cp = hes_to_bind(identifier, type);
		if (cp == NULL) {
			printf(" error %d\n", hes_error());
			exit(1);
		} 
		printf("%s\n", cp);
		if (lflag) printf("which ");
	}
	if (lflag) 
		printf("resolves to\n");

	cpp = hes_resolve(identifier, type);
	if (cpp == NULL) { 
		if (lflag) printf("nothing\n");
		switch(hes_error()) {
		case 0:
			break;
		case HES_ER_NOTFOUND:
			fputs("Hesiod name not found\n", stderr);
			break;
		case HES_ER_CONFIG:
			fputs("Hesiod configuration error\n", stderr);
			break;
		default:
			fputs("Unknown Hesiod error\n", stderr);
			break;
		}
	} else {
		while(*cpp) printf("%s\n", *cpp++);
	}
	if (!cpp)
		 exit(1);
	else
		exit(0);
}
