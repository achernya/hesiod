#ifndef lint
static char *RCS_ID = "$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/resolve.c,v 1.8 1993-10-21 14:36:12 mar Exp $";
#endif
/*
 * $Author: mar $
 * $Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/resolve.c,v $
 * $Athena: resolve.c,v 1.4 88/08/07 21:58:40 treese Locked $
 */

#define _RESOLVE_C_

#include <string.h>
#include <sys/types.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <sys/errno.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include "resscan.h"

#define DEF_RETRANS 4
#define DEF_RETRY 3

extern int errno;

static caddr_t
rr_scan(cp, rr)
    char *cp;
    rr_t *rr;
{
    register int n;

    if ((n = dn_skip(cp)) < 0) {
        errno = EINVAL;
        return((char *)NULL);
    }

    cp += n;
    rr->type = _getshort(cp);
    cp += sizeof(u_short/*type*/);

    rr->class = _getshort(cp);
#ifdef __alpha
    cp += sizeof(u_short/*class*/) + sizeof(u_int/*ttl*/);
#else
    cp += sizeof(u_short/*class*/) + sizeof(u_long/*ttl*/);
#endif

    rr->dlen = (int)_getshort(cp);
    rr->data = cp + sizeof(u_short/*dlen*/);

    return(rr->data + rr->dlen);
}


nsmsg_p
res_scan(msg)
    char *msg;
{
    static char bigmess[sizeof(nsmsg_t) + sizeof(rr_t)*((PACKETSZ-sizeof(HEADER))/RRFIXEDSZ)];
    static char datmess[PACKETSZ-sizeof(HEADER)];
    register char *cp;
    register rr_t *rp;
    register HEADER *hp;
    register char *data = datmess;
    register int n, n_an, n_ns, n_ar, nrec;
    register nsmsg_t *mess = (nsmsg_t *)bigmess;

    hp = (HEADER *)msg;
    cp = msg + sizeof(HEADER);
    n_an = ntohs(hp->ancount);
    n_ns = ntohs(hp->nscount);
    n_ar = ntohs(hp->arcount);
    nrec = n_an + n_ns + n_ar;

    mess->len = 0;
    mess->hd = hp;
    mess->ns_off = n_an;
    mess->ar_off = n_an + n_ns;
    mess->count = nrec;
    rp = &mess->rr;

    /* skip over questions */
    if (n = ntohs(hp->qdcount)) {
        while (--n >= 0) {
            register int i;
            if ((i = dn_skip(cp)) < 0)
                return((nsmsg_t *)NULL);
            cp += i + (sizeof(u_short/*type*/) + sizeof(u_short/*class*/));
        }
    }

    /* scan answers */
    if (n = n_an) {
        while (--n >= 0) {
            if ((cp = rr_scan(cp, rp)) == NULL)
                return((nsmsg_t *)NULL);
            (void) strncpy(data, rp->data, rp->dlen);
            rp->data = data;
            data += rp->dlen;
            *data++ = '\0';
            rp++;
        }
    }

    /* scan name servers */
    if (n = n_ns) {
        while (--n >= 0) {
            if ((cp = rr_scan(cp, rp)) == NULL)
                return((nsmsg_t *)NULL);
            (void) strncpy(data, rp->data, rp->dlen);
            rp->data = data;
            data += rp->dlen;
            *data++ = '\0';
            rp++;
        }
    }

    /* scan additional records */
    if (n = n_ar) {
        while (--n >= 0) {
            if ((cp = rr_scan(cp, rp)) == NULL)
                return((nsmsg_t *)NULL);
            (void) strncpy(data, rp->data, rp->dlen);
            rp->data = data;
            data += rp->dlen;
            *data++ = '\0';
            rp++;
        }
    }

    mess->len = (int)cp - (int)msg;

    return(mess);
}

/*
 * Resolve name into data records
 */

nsmsg_p
_resolve(name, class, type, patience)
    char *name;
    int class, type;
    retransXretry_t patience;
{
    static char qbuf[PACKETSZ], abuf[PACKETSZ];
    register int n;
#ifdef __alpha
    register int res_options = _res.options;
#else
    register long res_options = _res.options;
#endif
    register int res_retrans = _res.retrans;
    register int res_retry = _res.retry;

#ifdef DEBUG
    if (_res.options & RES_DEBUG)
        printf("_resolve: class = %d, type = %d\n", class, type);
#endif

    if (class < 0 || type < 0) {
        errno = EINVAL;
        return((nsmsg_t *)NULL);
    }

    _res.options |= RES_IGNTC;

    n = res_mkquery(QUERY, name, class, type, (char *)0, 0, NULL, qbuf, PACKETSZ);
    if (n < 0) {
        errno = EMSGSIZE;
        return((nsmsg_t *)NULL);
    }

    _res.retrans = (patience.retrans ? patience.retrans : DEF_RETRANS);
    _res.retry = (patience.retry ? patience.retry : DEF_RETRY);

    n = res_send(qbuf, n, abuf, PACKETSZ);

    _res.options = res_options;
    _res.retrans = res_retrans;
    _res.retry = res_retry;

    if (n < 0) {
        errno = ECONNREFUSED;
        return((nsmsg_t *)NULL);
    }

    return(res_scan(abuf));
}


/*
 * Skip over a compressed domain name. Return the size or -1.
 */
static
dn_skip(comp_dn)
	char *comp_dn;
{
	register char *cp;
	register int n;

	cp = comp_dn;
	while (n = *cp++) {
		/*
		 * check for indirection
		 */
		switch (n & INDIR_MASK) {
		case 0:		/* normal case, n == len */
			cp += n;
			continue;
		default:	/* illegal type */
			return (-1);
		case INDIR_MASK:	/* indirection */
			cp++;
		}
		break;
	}
	return (cp - comp_dn);
}
