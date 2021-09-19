/*	@(#)linea.c	UniPlus VVV.2.1.4	*/
/*
 *	This routine is the 'high level' a-line trap handler ... see 
 *		*ivec.s for the rest of this code.
 */

#include <sys/param.h>
#include <sys/types.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/reg.h>

#ifdef PMMU
long
lineA(ap)
register struct args *ap;
{
	register caddr_t *ptr;

	ptr = (caddr_t *) u.u_user[0];	/* We already know that u.u_user[0] */
					/* is non-zero */
	ptr[1] = ap->a_pc;
	ap->a_pc = ptr[0];
}
#endif PMMU

