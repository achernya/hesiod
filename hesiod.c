/* This file is part of the Hesiod library.
 *
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesiod.c,v $
 *	$Author: vrt $
 *	$Athena: hesiod.c,v 1.5 88/08/07 22:00:44 treese Locked $
 *	$Log: not supported by cvs2svn $
 * Revision 1.9  90/07/19  09:20:09  epeisach
 * Declare that getenv returns a char*
 * 
 * Revision 1.8  90/07/11  16:46:44  probe
 * Patches from <mar>
 * Support for HES_DOMAIN environment variable added
 * 
 * Revision 1.9  90/07/11  16:41:18  probe
 * Patches from <mar>
 * Added description about error codes and the HES_DOMAIN environment
 * variable
 * 
 * Revision 1.7  89/11/16  06:49:31  probe
 * Uses T_TXT, as defined in the RFC.
 * 
 * Revision 1.6.1.1  89/11/03  17:50:12  probe
 * Changes T_TXT to T_UNSPECA.
 * 
 * The BIND 4.8.1 implementation of T_TXT is incorrect; BIND 4.8.1 declares
 * it as a NULL terminated string.  The RFC defines T_TXT to be a length
 * byte followed by arbitrary changes.
 * 
 * Because of this incorrect declaration in BIND 4.8.1, when this bug is fixed,
 * T_TXT requests between machines running different versions of BIND will
 * not be compatible (nor is there any way of adding compatibility).
 * 
 * Revision 1.6  88/08/07  23:17:03  treese
 * Second-public-distribution
 * 
 * Revision 1.5  88/08/07  22:00:44  treese
 * Changed T_UNSPECA to T_TXT.
 * Changed C_HESIOD to C_HS.
 * Deleted ifdef's based on USE_HS_QUERY -- they're obsolete.
 * 
 * Revision 1.4  88/08/07  21:52:31  treese
 * First public distribution
 * 
 * Revision 1.3  88/06/12  00:52:58  treese
 * Cleaned up to work with Saber.
 * First public distribution.
 * 
 * Revision 1.2  88/06/11  22:36:38  treese
 * Cleaned up for public distribution.
 * 
 * 
 *
 * Copyright 1988 by the Massachusetts Institute of Technology.  See the
 * file <mit-copyright.h> for copying and distribution information.
 */

#include "mit-copyright.h"

#ifndef lint
static char rcsid_hesiod_c[] = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/hesiod.c,v 1.10 1993-04-27 14:03:44 vrt Exp $";
#endif

#include <stdio.h>
#include <errno.h>
#include <strings.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "resscan.h"
#include "hesiod.h"

#define USE_HS_QUERY	/* undefine this if your higher-level name servers */
			/* don't know class HS */

char *HesConfigFile = HESIOD_CONF;
static char *Hes_LHS, *Hes_RHS;
static int Hes_Errno = HES_ER_UNINIT;

retransXretry_t NoRetryTime = { 0, 0};

hes_init()
{
	register FILE *fp;
	register char *key, *cp, **cpp;
	int len;
	char buf[MAXDNAME+7];
	char *calloc(), *getenv();

	Hes_Errno = HES_ER_UNINIT;
	Hes_LHS = NULL; Hes_RHS = NULL;
	if ((fp = fopen(HesConfigFile, "r")) == NULL) {
		/* use defaults compiled in */
		/* no file or no access uses defaults */
		/* but poorly formed file returns error */
		Hes_LHS = DEF_LHS; Hes_RHS = DEF_RHS;
	} else {
		while(fgets(buf, MAXDNAME+7, fp) != NULL) {
			cp = buf;
			if (*cp == '#' || *cp == '\n') continue;
			while(*cp == ' ' || *cp == '\t') cp++;
			key = cp;
			while(*cp != ' ' && *cp != '\t' && *cp != '=') cp++;
			*cp++ = '\0';
			if (strcmp(key, "lhs") == 0) cpp = &Hes_LHS;
			else if (strcmp(key, "rhs") == 0) cpp = &Hes_RHS;
			else continue;
			while(*cp == ' ' || *cp == '\t' || *cp == '=') cp++;
			if (*cp != '.') {
				Hes_Errno = HES_ER_CONFIG;
				fclose(fp);
				return(Hes_Errno);
			}
			len = strlen(cp);
			*cpp = calloc((unsigned int) len, sizeof(char));
			(void) strncpy(*cpp, cp, len-1);
		}
		fclose(fp);
	}
	/* see if the RHS is overridden by environment variable */
	if ((cp = getenv("HES_DOMAIN")) != NULL)
		Hes_RHS = strcpy(malloc(strlen(cp)+1),cp);
	/* the LHS may be null, the RHS must not be null */
	if (Hes_RHS == NULL)
		Hes_Errno = HES_ER_CONFIG;
	else
		Hes_Errno = HES_ER_OK;	
	return(Hes_Errno);
}

char *
hes_to_bind(HesiodName, HesiodNameType)
char *HesiodName, *HesiodNameType;
{
	char **hes_resolve();
	register char *cp, **cpp;
	static char bindname[MAXDNAME];
	char *RHS;

	if (Hes_Errno == HES_ER_UNINIT || Hes_Errno == HES_ER_CONFIG)
		(void) hes_init();
	if (Hes_Errno == HES_ER_CONFIG) return(NULL);
	if (cp = index(HesiodName,'@')) {
		if (index(++cp,'.'))
			RHS = cp;
		else
			if (cpp = hes_resolve(cp, "rhs-extension"))
				RHS = *cpp;
			else {
				Hes_Errno = HES_ER_NOTFOUND;
				return(NULL);
			}
		(void) strcpy(bindname,HesiodName);
#ifndef SOLARIS
		*index(bindname,'@') = '\0';
#else
		*strchr(bindname,'@') = '\0';
#endif /* SOLARIS */
	} else {
		RHS = Hes_RHS;
		(void) strcpy(bindname, HesiodName);
	}
	(void) strcat(bindname, ".");
	(void) strcat(bindname, HesiodNameType);
	if (Hes_LHS) {
		if (Hes_LHS[0] != '.')
			(void) strcat(bindname,".");
		(void) strcat(bindname, Hes_LHS);
	}
	if (RHS[0] != '.')
		(void) strcat(bindname,".");
	(void) strcat(bindname, RHS);
	return(bindname);
}

char **
hes_resolve(HesiodName, HesiodNameType)
char *HesiodName, *HesiodNameType;
{
	register char *cp;
	static char *retvec[100];
	char *ocp, *dst;
	char *calloc();
	int i, j, n;
	struct nsmsg *ns, *_resolve();
	rr_t *rp;
	extern int errno;

	cp = hes_to_bind(HesiodName, HesiodNameType);
	if (cp == NULL) return(NULL);
	errno = 0;
	ns = _resolve(cp, C_HS, T_TXT, NoRetryTime);
	if (errno == ETIMEDOUT || errno == ECONNREFUSED) {
		Hes_Errno = HES_ER_NET;
		return(NULL);
	}
	if (ns == NULL || ns->ns_off <= 0) {
		Hes_Errno = HES_ER_NOTFOUND;
		return(NULL);
	}
	for(i = j = 0, rp = &ns->rr; i < ns->ns_off; rp++, i++) {
		if (
		    rp->class == C_HS &&
		    rp->type == T_TXT) { /* skip CNAME records */
			retvec[j] = calloc(rp->dlen + 1, sizeof(char));
			dst = retvec[j];
			ocp = cp = rp->data;
			while (cp < ocp + rp->dlen) {
			    n = (unsigned char) *cp++;
			    (void) bcopy(cp, dst, n);
			    cp += n;
			    dst += n;
			}
			*dst = 0;
			j++;
		}
	}
	retvec[j] = 0;
	return(retvec);
}

int
hes_error()
{
	return(Hes_Errno);
}
