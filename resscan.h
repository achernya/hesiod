/* This file contains definitions for the name resolver scanning routines
 * used by the Hesiod library.
 *
 * For copying and distribution information, see the file <mit-copyright.h>.
 *
 * Original version by Steve Dyer, IBM/Project Athena, and Sam Hsu,
 * DEC/Project Athena.
 *
 *	$Author: cfields $
 *	$Athena: resscan.h,v 1.3 88/08/07 21:53:09 treese Locked $
 *	$Header: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/resscan.h,v 1.5 1995-06-25 01:36:21 cfields Exp $
 *	$Source: /afs/dev.mit.edu/source/repository/athena/lib/hesiod/resscan.h,v $
 *	$Log: not supported by cvs2svn $
 * Revision 1.4  1988/08/07  23:17:42  treese
 * Second-public-distribution
 *
 * Revision 1.3  88/08/07  21:53:09  treese
 * First public distribution
 * 
 * Revision 1.2  88/06/12  00:53:14  treese
 * Cleaned up to work with Saber.
 * First public distribution.
 * 
 * Revision 1.2  88/06/05  19:51:32  treese
 * Cleaned up for public distribution
 * 
 */

typedef struct rr {
    short type;			     /* RR type */
    short class;		     /* RR class */
    int dlen;			     /* len of data section */
    char *data;			     /* pointer to data */
} rr_t, *rr_p;

typedef struct nsmsg {
    int len;			     /* sizeof(msg) */
    int ns_off;			     /* offset to name server RRs */
    int ar_off;			     /* offset to additional RRs */
    int count;			     /* total number of RRs */
    HEADER *hd;			     /* message header */
    rr_t rr;			     /* vector of (stripped-down) RR descriptors */
} nsmsg_t, *nsmsg_p;

typedef struct retransXretry {
    short retrans;
    short retry;
} retransXretry_t, *retransXretry_p;


#define RES_INITCHECK() if(!(_res.options&RES_INIT))res_init();

extern char *p_cdname(), *p_rr(), *p_type(), *p_class();
extern struct nsmsg *res_scan(), *resolve(), *_resolve(); 
