/*	@(#)usys4.c	UniPlus VVV.2.1.2	*/


#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/dir.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/region.h"
#include "sys/proc.h"
#endif lint
/*
 * phys - Set up a physical address in user's address space.
 */
phys()
{
	register struct a {
		unsigned phnum;		/* phys segment number */
		unsigned laddr;		/* logical address */
		unsigned bcount;	/* byte count */
		unsigned phaddr;	/* physical address */
	} *uap;

	if (!suser()) return;
	uap = (struct a *)u.u_ap;
	dophys(uap->phnum, uap->laddr, uap->bcount, uap->phaddr);
}

/*
 * reboot the system
 */
reboot()
{
	register i;

	if (!suser())
		return;
	sync();
	for (i = 0; i < 1000000; i++)
		;
	doboot();
}
