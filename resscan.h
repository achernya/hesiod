/*
 * resolver scanning utilities
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

extern struct state _res;
extern char *p_cdname(), *p_rr(), *p_type(), *p_class();
extern struct nsmsg *res_scan(), *resolve(), *_resolve(); 
