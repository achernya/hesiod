/* This file contains definitions for use by the Hesiod name service and
 * applications.
 *
 * For copying and distribution information, see the file <mit-copyright.h>.
 *
 * Original version by Steve Dyer, IBM/Project Athena.
 *
 *	$Id: hesiod.h.sed,v 1.7 1991-06-02 14:03:56 probe Exp $
 */

/* Configuration information. */

#define HESIOD_CONF	"CONFDIR/hesiod.conf"	/* Configuration file. */
#define DEF_RHS		".Athena.MIT.EDU"	/* Defaults if HESIOD_CONF */
#define DEF_LHS		".ns"			/*    file is not present. */

/* Error codes. */

#define	HES_ER_UNINIT	-1	/* uninitialized */
#define	HES_ER_OK	0	/* no error */
#define	HES_ER_NOTFOUND	1	/* Hesiod name not found by server */
#define HES_ER_CONFIG	2	/* local problem (no config file?) */
#define HES_ER_NET	3	/* network problem */

/* Declaration of routines */

char *hes_to_bind();
char **hes_resolve();
int hes_error();

/* For use in getting post-office information. */

struct hes_postoffice {
	char	*po_type;
	char	*po_host;
	char	*po_name;
};

/* Other routines */

struct hes_postoffice *hes_getmailhost();
struct servent *hes_getservbyname();
struct passwd *hes_getpwnam();
