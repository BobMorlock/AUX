/*	@(#)probe.c	UniPlus VVV.2.1.3	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/file.h"
#include "sys/vnode.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/pfdat.h"
#include "sys/proc.h"
#include "sys/debug.h"
#endif lint

/*
 * Calculate number of pages transfer touchs
 */
#define len(base, count)	\
	btop(base + count) - btotp(base)

/*
 * Calulate starting user PTE address for transfer
 */
#define upt(base)	\
	(pte_t *) segvptbl(&u.u_stbl[snum(base)]) + pnum(base)
union ptbl *segvptbl();

/*
 * Check user read/write access
 * If rw has B_PHYS set, the user PTEs will be faulted
 * in and locked down.  Returns 0 on failure, 1 on success
 */
useracc(base, count, rw)
register base, count;
{
	register	i;
	register int	x;
	register pte_t	*pt;
	register reg_t  *rp;


	
	/*
	 * If physio, two checks are made:
	 *	1) Base must be word aligned
	 *	2) Transfer must be contained in one segment
	 */

	if (rw & B_PHYS && ((base & 1)
	 || snum(base) != snum(base + count - 1)))
		return(0);

	/*
	 * Check page permissions and existence
	 */

	rp = findreg(u.u_procp, (caddr_t)base);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);

	for(i = len(base, count); --i >= 0; base += ptob(1)) {
#ifndef lint
		pt = (pte_t *)upt(base);
#else
		pt = (pte_t *)0;
#endif
		x = fubyte((caddr_t)base);
		if(x == -1)
			goto out;
		if(rw & B_READ)
			if(subyte((caddr_t)base, x) == -1)
				goto out;
		if (rw & B_PHYS){
			memlock();
			pg_setlock(pt);
			pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt++;
			memunlock();
		}
	}
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
	return(1);
out:
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
	return(0);
}


/*
 * Terminate user physio
 */
undma(base, count, rw)
register int base, rw;
{
	register pte_t *pt;
	register npgs;
	register reg_t *rp;


	/*
	 * Unlock PTEs, set the reference bit.
	 * Set the modify bit if B_READ
	 */
	rp = findreg(u.u_procp, (caddr_t)base);
	for (npgs = len(base, count); --npgs >= 0; base += ptob(1)) {
#ifndef lint
		pt = (pte_t *)upt(base);
#else
		pt = (pte_t *)0;
#endif
		ASSERT(npteget(pt)->pgm[0].pg_lock);
		ASSERT(pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt > 0);
		memlock();
		if(--pfdat[pftopfi(pt->pgm[0].pg_pfn)].pf_rawcnt == 0)
			pg_clrlock(pt);
		memunlock();
		pg_setref(pt);
		if (rw == B_READ) 
			pg_setmod(pt);
	}
	regrele(rp);
}


/* <@(#)probe.c	1.2> */
