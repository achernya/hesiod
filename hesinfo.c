/*
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesinfo.c,v $
 *	$Author: treese $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesinfo.c,v 1.1 1988-06-05 17:50:15 treese Exp $
 */

#ifndef lint
static char rcsid_hesinfo_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesinfo.c,v 1.1 1988-06-05 17:50:15 treese Exp $";
#endif

#include <stdio.h>

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
	} else {
		while(*cpp) printf("%s\n", *cpp++);
	}
	if (!cpp) exit(1);
}
