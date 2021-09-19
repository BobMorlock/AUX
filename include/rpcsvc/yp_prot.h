/*	@(#)yp_prot.h 1.1 85/05/30 SMI	*/

/*
 * yp_prot.h
 * yp - yellow pages server and binder protocols.
 *
 * This file contains symbols and structures defining the
 * rpc protocol between the yellowpages clients and the yellowpages
 * servers.  The servers are the yp database servers, and the yp binders.
 */

/*
 * The following procedures are supported by the protocol:
 *
 * YPPROC_NULL() returns ()
 * 	takes nothing, returns nothing.  This indicates that the yp server is
 *	alive.
 *
 * YPPROC_DOMAIN (char *) returns (bool_t)
 *	TRUE indicates that the responding yp server does serve the named 
 *	domain; FALSE indicates no support.
 *	
 * YPPROC_DOMAIN_NONACK (char *) returns (TRUE) if the yp server does serve
 *	the named domain, otherwise does not return.  Used in the broadcast
 *	case.
 *	
 * YPPROC_MATCH (yprequest of YPREQ_KEY type) returns (ypresponse of
 *	YPRESP_VAL type).  Returns the right-hand value for a passed left-hand
 *	key, within a named map and domain.  The match key token must be exact.
 *
 * YPPROC_FIRST (yprequest of YPREQ_NOKEY type) returns (ypresponse of
 *	YPRESP_KEY_VAL type).  Returns the first key-value pair from a named
 *	domain and map.
 *
 * YPPROC_NEXT (yprequest of YPREQ_KEY type) returns (ypresponse of
 *	YPRESP_KEY_VAL type).   Returns the key-value pair following a passed
 *	key-value pair within a named domain and map.
 *
 * YPPROC_PUSH (yprequest of YPREQ_NOKEY type) returns nothing.  Instructs a
 *	yp server which is the owner of a passed map within a passed domain to
 *	inform his peers within the domain that there is a new copy of
 *	the map.  If the server is the map's master, he will emit a YPPROC_GET
 *	request to each reachable peer in the domain; if he is not the map's
 *	master, he will do nothing.  This is an administrative interface to 
 *	the yp system.
 *
 * YPPROC_PULL (yprequest of YPREQ_NOKEY type) returns nothing.  Instructs a
 *	yp server to get the latest version of a named map within a named
 *	domain.  If he is not the map's master he will attempt to get a later
 *	copy of the map than he now has, trying first from the map's master,
 *	and, that failing, from any reachable peer.  If he is the map's master,
 *	or if he does not serve the passed domain, he will do nothing.  This
 *	is an administrative interface to the yp system.
 *
 * YPPROC_POLL (yprequest of YPREQ_NOKEY type) returns (ypresponse of
 *	YPRESP_MAP_PARMS type).  Instructs a yp server to return a map's
 *	parameters (order number and master) insofar as he now knows them.
 *	This is time-efficient but perhaps not the most correct information
 *	potentially available to him:  he uses his internal data structures,
 *	rather than doing the file I/O needed to check the order number and 
 *	master server values in his current map files.  This is primarily a
 *	server-to-server interface.
 *
 * YPPROC_GET (yprequest of YPREQ_MAP_PARMS type) returns nothing.  Instructs
 *	a yp server to get the latest version of a named map within a named
 *	domain.  This is the server-to-server version of the administrative
 *	request YPPROC_PULL.
 */
 
struct ypmap_parms {
	char *domain;			/* Null string means not available */
	char *map;			/* Null string means not available */
	unsigned long int ordernum;	/* 0 means not available */
	char *owner;			/* Null string means not available */
};


enum ypreqtype {YPREQ_KEY = 1, YPREQ_NOKEY = 2, YPREQ_MAP_PARMS = 3};
struct ypreq_key {
	char *domain;
	char *map;
	datum keydat;
};
struct ypreq_nokey {
	char *domain;
	char *map;
};
struct yprequest {
	enum ypreqtype yp_reqtype;
	union {
		struct ypreq_key yp_req_keytype;
		struct ypreq_nokey yp_req_nokeytype;
		struct ypmap_parms yp_req_map_parmstype;
	}yp_reqbody;
};
/*
 * Request types for yp functions which don't use simple "predefined" xdr
 * types, and structure member alias names
 */
#define YPMATCH_REQTYPE YPREQ_KEY
#define ypmatch_req_domain yp_reqbody.yp_req_keytype.domain
#define ypmatch_req_map yp_reqbody.yp_req_keytype.map
#define ypmatch_req_keydat yp_reqbody.yp_req_keytype.keydat
#define ypmatch_req_keyptr yp_reqbody.yp_req_keytype.keydat.dptr
#define ypmatch_req_keysize yp_reqbody.yp_req_keytype.keydat.dsize

#define YPFIRST_REQTYPE YPREQ_NOKEY
#define ypfirst_req_domain yp_reqbody.yp_req_nokeytype.domain
#define ypfirst_req_map yp_reqbody.yp_req_nokeytype.map

#define YPNEXT_REQTYPE YPREQ_KEY
#define ypnext_req_domain yp_reqbody.yp_req_keytype.domain
#define ypnext_req_map yp_reqbody.yp_req_keytype.map
#define ypnext_req_keydat yp_reqbody.yp_req_keytype.keydat
#define ypnext_req_keyptr yp_reqbody.yp_req_keytype.keydat.dptr
#define ypnext_req_keysize yp_reqbody.yp_req_keytype.keydat.dsize

#define YPPUSH_REQTYPE YPREQ_NOKEY
#define yppush_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppush_req_map yp_reqbody.yp_req_nokeytype.map

#define YPPULL_REQTYPE YPREQ_NOKEY
#define yppull_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppull_req_map yp_reqbody.yp_req_nokeytype.map

#define YPPOLL_REQTYPE YPREQ_NOKEY
#define yppoll_req_domain yp_reqbody.yp_req_nokeytype.domain
#define yppoll_req_map yp_reqbody.yp_req_nokeytype.map

#define YPGET_REQTYPE YPREQ_MAP_PARMS
#define ypget_req_domain yp_reqbody.yp_req_map_parmstype.domain
#define ypget_req_map yp_reqbody.yp_req_map_parmstype.map
#define ypget_req_ordernum yp_reqbody.yp_req_map_parmstype.ordernum
#define ypget_req_owner yp_reqbody.yp_req_map_parmstype.owner

enum ypresptype {YPRESP_VAL = 1, YPRESP_KEY_VAL = 2, YPRESP_MAP_PARMS = 3};
struct ypresp_val {
	long unsigned status;
	datum valdat;
};
struct ypresp_key_val {
	long unsigned status;
	datum valdat;
	datum keydat;
};
struct ypresponse {
	enum ypresptype yp_resptype;
	union {
		struct ypresp_val yp_resp_valtype;
		struct ypresp_key_val yp_resp_key_valtype;
		struct ypmap_parms yp_resp_map_parmstype;
	} yp_respbody;
};
/*
 * Response types for yp functions which don't use simple "predefined" xdr
 * types, and structure member alias names
 */
#define YPMATCH_RESPTYPE YPRESP_VAL
#define ypmatch_resp_status yp_respbody.yp_resp_valtype.status
#define ypmatch_resp_valdat yp_respbody.yp_resp_valtype.valdat
#define ypmatch_resp_valptr yp_respbody.yp_resp_valtype.valdat.dptr
#define ypmatch_resp_valsize yp_respbody.yp_resp_valtype.valdat.dsize

#define YPFIRST_RESPTYPE YPRESP_KEY_VAL
#define ypfirst_resp_status yp_respbody.yp_resp_key_valtype.status
#define ypfirst_resp_keydat yp_respbody.yp_resp_key_valtype.keydat
#define ypfirst_resp_keyptr yp_respbody.yp_resp_key_valtype.keydat.dptr
#define ypfirst_resp_keysize yp_respbody.yp_resp_key_valtype.keydat.dsize
#define ypfirst_resp_valdat yp_respbody.yp_resp_key_valtype.valdat
#define ypfirst_resp_valptr yp_respbody.yp_resp_key_valtype.valdat.dptr
#define ypfirst_resp_valsize yp_respbody.yp_resp_key_valtype.valdat.dsize

#define YPNEXT_RESPTYPE YPRESP_KEY_VAL
#define ypnext_resp_status yp_respbody.yp_resp_key_valtype.status
#define ypnext_resp_keydat yp_respbody.yp_resp_key_valtype.keydat
#define ypnext_resp_keyptr yp_respbody.yp_resp_key_valtype.keydat.dptr
#define ypnext_resp_keysize yp_respbody.yp_resp_key_valtype.keydat.dsize
#define ypnext_resp_valdat yp_respbody.yp_resp_key_valtype.valdat
#define ypnext_resp_valptr yp_respbody.yp_resp_key_valtype.valdat.dptr
#define ypnext_resp_valsize yp_respbody.yp_resp_key_valtype.valdat.dsize

#define YPPOLL_RESPTYPE YPRESP_MAP_PARMS
#define yppoll_resp_domain yp_respbody.yp_resp_map_parmstype.domain
#define yppoll_resp_map yp_respbody.yp_resp_map_parmstype.map
#define yppoll_resp_ordernum yp_respbody.yp_resp_map_parmstype.ordernum
#define yppoll_resp_owner yp_respbody.yp_resp_map_parmstype.owner

/* Program and version symbols, magic numbers */

#define YPPROG			((u_long)100004)
#define YPVERS			((u_long)1)
#define YPVERS_ORIG		((u_long)1)
#define YPMAXRECORD		((u_long)1024)
#define YPMAXDOMAIN		((u_long)64)
#define YPMAXMAP		((u_long)64)
#define YPSYMBOL_PREFIX "YP_"
#define YPSYMBOL_PREFIX_LENGTH (sizeof(YPSYMBOL_PREFIX) - 1)
#define YPMAXPEER		((u_long)256)	/* Maximum peer name length
						 *   (unenforcable) */

/* Procedure symbols */

#define YPPROC_NULL		((u_long)0)
#define YPPROC_DOMAIN		((u_long)1)
#define YPPROC_DOMAIN_NONACK	((u_long)2)
#define YPPROC_MATCH		((u_long)3)
#define YPPROC_FIRST		((u_long)4)
#define YPPROC_NEXT		((u_long)5)
#define YPPROC_POLL		((u_long)6)
#define YPPROC_PUSH		((u_long)7)
#define YPPROC_PULL		((u_long)8)
#define YPPROC_GET		((u_long)9)

/* Return status values */

#define YP_TRUE	 ((long)1)		/* General purpose success code */
#define YP_NOMORE ((long)2)		/* No more entries in map */
#define YP_FALSE ((long)0)		/* General purpose failure code */
#define YP_NOMAP ((long)-1)		/* No such map in domain */
#define YP_NODOM ((long)-2)		/* Domain not supported */
#define YP_NOKEY ((long)-3)		/* No such key in map */
#define YP_BADOP ((long)-4)		/* Invalid operation */
#define YP_BADDB ((long)-5)		/* Server data base is bad */
#define YP_YPERR ((long)-6)		/* YP server error */
#define YP_BADARGS ((long)-7)		/* Request arguments bad */

/*
 * Domain binding data structure, used by ypclnt package and ypserv modules.
 * Users of the ypclnt package (or of this protocol) don't HAVE to know about
 * it, but it must be available to users because _yp_dobind is a public
 * interface.
 */

struct dom_binding {
	struct dom_binding *dom_pnext;
	char dom_domain[YPMAXDOMAIN + 1];
	struct sockaddr_in dom_server_addr;
	unsigned short int dom_server_port;
	int dom_socket;
	CLIENT *dom_client;
	unsigned short int dom_local_port;
};

#ifndef BOOL_DEFINED
typedef unsigned int bool;
#define BOOL_DEFINED
#endif

extern bool xdr_datum();
extern bool xdr_ypdomain_wrap_string();
extern bool xdr_ypmap_wrap_string();
extern bool xdr_ypreq_key();
extern bool xdr_ypreq_nokey();
extern bool xdr_yprequest();
extern bool xdr_ypresp_val();
extern bool xdr_ypresp_key_val();
extern bool xdr_ypresponse();
extern bool xdr_ypbind_resp();
extern bool xdr_ypbind_setdom();
extern bool xdr_yp_inaddr();
extern bool xdr_ypmap_parms();
extern bool xdr_ypowner_wrap_string();

/*
 *		Protocol between clients and yp binder servers
 */

/*
 * The following procedures are supported by the protocol:
 *
 * YPBINDPROC_NULL() returns ()
 * 	takes nothing, returns nothing
 *
 * YPBINDPROC_DOMAIN takes (char *) returns (struct ypbind_resp)
 *
 * YPBINDPROC_SETDOM takes (struct ypbind_setdom) returns nothing
 */
 
/* Program and version symbols, magic numbers */

#define YPBINDPROG		((u_long)100007)
#define YPBINDVERS		((u_long)1)
#define YPBINDVERS_ORIG		((u_long)1)

/* Procedure symbols */

#define YPBINDPROC_NULL		((u_long)0)
#define YPBINDPROC_DOMAIN	((u_long)1)
#define YPBINDPROC_SETDOM	((u_long)2)
/*
 * Response structure and overall result status codes.  Success and failure
 * represent two separate response message types.
 */

enum ypbind_resptype {YPBIND_SUCC_VAL = 1, YPBIND_FAIL_VAL = 2};

struct ypbind_binding {
	struct in_addr ypbind_binding_addr;	/* In network order */
	unsigned short int ypbind_binding_port;	/* In network order */
};
struct ypbind_resp {
	enum ypbind_resptype ypbind_status;
	union {
		unsigned long ypbind_error;
		struct ypbind_binding ypbind_bindinfo;
	} ypbind_respbody;
};


/* Detailed failure reason codes for response field ypbind_error*/

#define YPBIND_ERR_ERR 1		/* Internal error */
#define YPBIND_ERR_NOSERV 2		/* No bound server for passed domain */
#define YPBIND_ERR_RESC 3		/* System resource allocation failure */

/*
 * Request data structure for ypbind "Set domain" procedure.
 */
struct ypbind_setdom {
	char ypsetdom_domain[YPMAXDOMAIN + 1];
	struct ypbind_binding ypsetdom_binding;
};
#define ypsetdom_addr ypsetdom_binding.ypbind_binding_addr
#define ypsetdom_port ypsetdom_binding.ypbind_binding_port
