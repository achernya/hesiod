/* $Id: hesiod.h,v 1.1 1996-11-07 02:28:09 ghudson Exp $ */

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

/* Original version by Steve Dyer, IBM/Project Athena. */

/* This file is part of the Hesiod library.  It declares routines,
 * constants, and structures visible to users of the library.
 */

#ifndef HES__HESIOD_H
#define HES__HESIOD_H

/* Configuration information. */

#define DEF_RHS		".Athena.MIT.EDU"	/* Defaults if HESIOD_CONF */
#define DEF_LHS		".ns"			/*    file is not present. */

/* Error codes. */

#define	HES_ER_OK	0	/* No error */
#define	HES_ER_UNINIT	1	/* Uninitialized */
#define	HES_ER_NOTFOUND	2	/* Hesiod name not found by server */
#define HES_ER_CONFIG	3	/* Local problem (no config file?) */
#define HES_ER_NET	4	/* Network problem */
#define HES_ER_RANGE	5	/* Return buffer not large enough */
#define HES_ER_NOMEM	6	/* Insufficient memory is available */
#define HES_ER_INVAL	7	/* Invalid response from hesiod server */

#define HES_MAX_ERRLEN	128	/* Max buffer space needed for error strings */

/* For use in getting post-office information. */

struct hes_postoffice {
  char *po_type;
  char *po_host;
  char *po_name;
};

struct passwd;
struct servent;

/* Thread-safe routines. */
int hes_init(void);
int hes_to_bind_r(const char *name, const char *type, char *buffer,
		  int buflen);
int hes_resolve_r(const char *name, const char *type, char **retvec,
		  int retveclen);
int hes_strerror_r(int errval, char *buf, int bufsize);
int hes_getpwnam_r(const char *name, struct passwd *entry, char *buf,
		   int bufsize, struct passwd **result);
int hes_getpwuid_r(int uid, struct passwd *entry, char *buf, int bufsize,
		   struct passwd **result);
int hes_getmailhost_r(const char *user, struct hes_postoffice *ret,
		      char *linebuf, int bufsize);
int hes_getservbyname_r(const char *name, const char *proto,
			struct servent *result, char *buffer, int buflen);

/* Non-thread-safe routines. */
char *hes_to_bind(const char *name, const char *type);
char **hes_resolve(const char *name, const char *type);
int hes_error(void);
const char *hes_strerror(int errval);
struct passwd *hes_getpwnam(const char *nam);
struct passwd *hes_getpwuid(int uid);
struct hes_postoffice *hes_getmailhost(const char *user);
struct servent *hes_getservbyname(const char *name, const char *proto);

#endif /* HES__HESIOD_H */
