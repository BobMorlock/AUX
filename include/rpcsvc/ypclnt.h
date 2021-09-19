/*	@(#)ypclnt.h 1.1 85/05/30 SMI	*/

/*
 * ypclnt.h
 * This defines the symbols used in the c language
 * interface to the yp client functions.  A description of this interface
 * can be read in ypclnt(3N).
 */

/*
 * Failure reason codes.  The success condition is indicated by a functional
 * value of "0".
 */
#define YPERR_BADARGS 1			/* Args to function are bad */
#define YPERR_RPC 2			/* RPC failure */
#define YPERR_DOMAIN 3			/* Can't bind to a server which serves
					 *   this domain. */
#define YPERR_MAP 4			/* No such map in server's domain */
#define YPERR_KEY 5			/* No such key in map */
#define YPERR_YPERR 6			/* Internal yp server or client
					 *   interface error */
#define YPERR_RESRC 7			/* Local resource allocation failure */
#define YPERR_NOMORE 8			/* No more records in map database */
#define YPERR_PMAP 9			/* Can't communicate with portmapper */
#define YPERR_YPBIND 10			/* Can't communicate with ypbind */
#define YPERR_YPSERV 11			/* Can't communicate with ypserv */
#define YPERR_NODOM 12			/* Local domain name not set */


/*
 * External yp client function references. 
 */
extern int yp_bind();
extern void yp_unbind();
extern int yp_get_default_domain ();
extern int yp_match ();
extern int yp_first ();
extern int ypclnt_first ();
extern int yp_next();
extern int ypclnt_next();
extern char *yperr_string();

