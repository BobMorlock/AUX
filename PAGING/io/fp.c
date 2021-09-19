/*	@(#)fp.c	UniPlus VVV.2.1.1	*/

/*
 * MC68881 Floating-Point Coprocessor
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#ifdef lint
#include <sys/sysinclude.h>
#include <sys/fpioctl.h>
#else lint
#include <sys/param.h>
#include <sys/uconfig.h>
#include <sys/types.h>
#include <sys/mmu.h>
#include <sys/seg.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/sysmacros.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/buf.h>
#include <sys/time.h>
#include <sys/proc.h>
#include <sys/conf.h>
#include <signal.h>
#include <sys/user.h>
#include <sys/systm.h>
#include <sys/ioctl.h>
#include <sys/uioctl.h>
#include <sys/fpioctl.h>
#endif lint

/*ARGSUSED*/
fpioctl(dev, cmd, addr, flag)
dev_t dev;
caddr_t addr;
{
	register struct user *up;

	up = &u;
	switch (cmd) {
#ifdef mc68881		/* MC68881 floating-point coprocessor */
		case FPIOCEXC:	/* return reason for last 881 SIGFPE */
			if (fubyte(addr) == -1) {
				up->u_error = EFAULT;
				return;
			}
			/* u_fpexc is saved in trap.c */
			if (subyte(addr, up->u_fpexc) == -1) {
				up->u_error = EFAULT;
				return;
			}
			break;
#endif mc68881
		default:
			up->u_error = EINVAL;
	}
}

#ifdef mc68881		/* MC68881 floating-point coprocessor */
/*
 *	Save the internal state and programmer's model of the 68881.
 *	This is called from psig(), core(), swtch(), or procdup().
 */
fpsave()
{
	/* warning: asm calls depend on the order of declaration */
	register char *reg;		/* a2 */
	register char *istate;		/* a3 */
	extern short fp881;		/* is there an MC68881? */

	if (fp881 && u.u_fpsaved == 0) {
		istate = &u.u_fpstate[0];
		/* halt processing and save internal state of coprocessor */
		asm("	short	0xf313		# fsave (a3)");
		/* check for non-null saved state (version # byte) */
		if (*((short *)istate)) {
			/* save system registers CONTROL/STATUS/IADDR */
			reg = (char *)&u.u_fpsysreg[0];
			asm("	short	0xf212		# fmovem (a2)");
			asm("	short	0xbc00		# save system regs");
			/* save data registers FP0,FP1,...,FP7 */
			reg = &u.u_fpdreg[0][0];
			asm("	short	0xf212		# fmovem (a2)");
			asm("	short	0xf0ff		# save data registers");
#ifdef lint
			*reg = *istate;
#endif lint
		}
		u.u_fpsaved = 1;
	}
}

/*
 *	Restore the internal state and programmer's model of the 68881.
 *	This is called from trap() or syscall().
 */
fprest()
{
	/* warning: asm calls depend on the order of declaration */
	register char *reg;		/* a2 */
	register char *istate;		/* a3 */

	istate = &u.u_fpstate[0];
	/* check for non-null saved state (version # byte) */
	if (*((short *)istate)) {
		/* restore system registers CONTROL/STATUS/IADDR */
		reg = (char *)&u.u_fpsysreg[0];
		asm("	short	0xf212		# fmovem (a2)");
		asm("	short	0x9c00		# restore system registers");
		/* restore data registers FP0,FP1,...,FP7 */
		reg = &u.u_fpdreg[0][0];
		asm("	short	0xf212		# fmovem (a2)");
		asm("	short	0xd0ff		# restore data registers");
#ifdef lint
		*reg = *istate;
#endif lint
	}
	/* load internal state of coprocessor */
	asm("	short	0xf353		# frestore (a3)");
}
#endif mc68881
