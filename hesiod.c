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
	char *calloc();

	Hes_Errno = HES_ER_UNINIT;
	Hes_LHS = NULL; Hes_RHS = NULL;
	if ((fp = fopen(HesConfigFile, "r")) == NULL) {
		/* use defaults compiled in */
		/* no file or no access uses defaults */
		/* but poorly formed file returns error */
		Hes_LHS = DEF_LHS; Hes_RHS = DEF_RHS;
		Hes_Errno = HES_ER_OK;
		return(Hes_Errno);
	}
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
			return(Hes_Errno);
		}
		len = strlen(cp);
		*cpp = calloc((unsigned int) len, sizeof(char));
		(void) strncpy(*cpp, cp, len-1);
	}
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
		*index(bindname,'@') = '\0';
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
	char *calloc();
	int i, j;
	struct nsmsg *ns, *_resolve();
	rr_t *rp;
	extern int errno;

	cp = hes_to_bind(HesiodName, HesiodNameType);
	if (cp == NULL) return(NULL);
	errno = 0;
#ifdef USE_HS_QUERY
	ns = _resolve(cp, C_HESIOD, T_TXT, NoRetryTime);
#else
	ns = _resolve(cp, C_ANY, T_TXT, NoRetryTime);
#endif
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
#ifndef USE_HS_QUERY
		    rp->class == C_HESIOD &&
#endif
		    rp->type == T_TXT) { /* skip CNAME records */
			retvec[j] = calloc((unsigned int) rp->dlen,
					   sizeof(char));
			(void) strcpy(retvec[j++], rp->data);
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
