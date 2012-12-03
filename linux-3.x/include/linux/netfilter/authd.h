#ifndef _NETFILTER_AUTHD_H
#define _NETFILTER_AUTHD_H

/* This one appears to be unused for the moment.
 */
#define SO_AUTHD		86


/* Auth information setting socket options */
enum authd_ops {
	AUTHD_OP_EXTIRPATE,		/* Extirpate unrequited data */
	AUTHD_OP_SADDR,			/* Set real source address */
	AUTHD_OP_DADDR,			/* Set real destination address */
	AUTHD_OP_URL,			/* Set target URL */
	AUTHD_OP_GROUPS,		/* Group list */
	AUTHD_OP_TSCATEGORIES,		/* TS categroy list */
	AUTHD_OP_TSREPUTATION,		/* TS site reputation */
	AUTHD_OP_DEV,			/* Incoming device */
};

#endif
