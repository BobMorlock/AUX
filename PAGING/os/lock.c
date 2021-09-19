/*	@(#)lock.c	UniPlus VVV.2.1.1	*/

#ifdef lint
#include "sys/sysinclude.h"
#include "sys/lock.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/lock.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#endif lint

lock()
{
	struct a {
		long oper;
	};

	if (!suser())
		return;
	switch(((struct a *)u.u_ap)->oper) {
	case TXTLOCK:
		if((u.u_lock & (PROCLOCK|TXTLOCK)) || textlock() == 0)
			goto bad;
		break;
	case PROCLOCK:
		if(u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK))
			goto bad;
		if(u.u_exdata.ux_mag != 0407  &&  textlock() == 0)
			goto bad;
		(void)datalock();
		proclock();
		break;
	case DATLOCK:
		if ((u.u_lock&(PROCLOCK|DATLOCK))  ||  datalock() == 0)
			goto bad;
		break;
	case UNLOCK:
		if (punlock() == 0)
			goto bad;
		break;

	default:
bad:
		u.u_error = EINVAL;
	}
}

textlock()
{
	register preg_t	*prp;
	register reg_t	*rp;


	prp = findpreg(u.u_procp, PT_TEXT);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	reglock(rp);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);
	u.u_lock |= TXTLOCK;
	return(1);
}
		
tunlock()
{
	register preg_t	*prp;
	register reg_t	*rp;

	prp = findpreg(u.u_procp, PT_TEXT);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
	u.u_lock &= ~TXTLOCK;
	return(1);
}

datalock()
{
	register preg_t	*prp;
	register reg_t	*rp;


	prp = findpreg(u.u_procp, PT_DATA);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	reglock(rp);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);
	prp = findpreg(u.u_procp, PT_STACK);
	if(prp == NULL) {
		reglock(rp);
		rp->r_flags &= ~RG_NOSWAP;
		regrele(rp);
		return(0);
	}
	rp = prp->p_reg;
	reglock(rp);
	rp->r_flags |= RG_NOSWAP;
	regrele(rp);
	u.u_lock |= DATLOCK;
	return(1);
}
		
dunlock()
{
	register preg_t	*prp;
	register reg_t	*rp;


			
	prp = findpreg(u.u_procp, PT_DATA);
	if(prp == NULL)
		return(0);
	rp = prp->p_reg;
	reglock(rp);
	rp->r_flags &= ~RG_NOSWAP;
	regrele(rp);
	u.u_lock &= ~DATLOCK;
	return(1);
}

proclock()
{
	u.u_procp->p_flag |= SSYS;
	u.u_lock |= PROCLOCK;
}

punlock()
{
	if ((u.u_lock&(PROCLOCK|TXTLOCK|DATLOCK)) == 0)
		return(0);
	u.u_procp->p_flag &= ~SSYS;
	u.u_lock &= ~PROCLOCK;
	(void) tunlock();
	(void) dunlock();
	return(1);
}

/* <@(#)lock.c	6.2> */
