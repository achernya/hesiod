#define	HES_ER_UNINIT	-1	/* uninitialized */
#define	HES_ER_OK	0	/* no error */
#define	HES_ER_NOTFOUND	1	/* Hesiod name not found by server */
#define HES_ER_CONFIG	2	/* local problem (no config file?) */
#define HES_ER_NET	3	/* network problem */

#define HESIOD_CONF	"/etc/hesiod.conf"
#define DEF_LHS		".ns"	/* defaults if not HESIOD_CONF is present */
#define DEF_RHS		".Athena.MIT.EDU"

/* definition of routines */
char *hes_to_bind();
char **hes_resolve();

struct hes_postoffice {
	char	*po_type;
	char	*po_host;
	char	*po_name;
};

struct hes_postoffice *hes_getmailhost();
