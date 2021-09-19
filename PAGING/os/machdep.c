/*	@(#)machdep.c	UniPlus VVV.2.1.7	*/

#ifdef HOWFAR
extern int T_machdep;
extern int T_sendsig;
#endif HOWFAR

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/param.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/reg.h"
#include "sys/acct.h"
#include "sys/map.h"
#include "sys/file.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/vnode.h"
#include "sys/debug.h"

#include "sys/var.h"
#include "sys/psl.h"
#include "sys/pmmu.h"
#include "sys/buserr.h"

#include "sys/pathname.h"
#include "sys/uio.h"
#include "sys/trace.h"
#include <sys/clock.h>
#include <compat.h>
#endif lint

#define	mask(s)	(1<<((s)-1))

int	tick, tickadj;
static	badstack();

clkset(oldtime)
time_t	oldtime;
{
	time.tv_sec = oldtime;
	tick	= 1000000 / v.v_hz;
	tickadj	= 1000000 / v.v_hz / 10;
}

		
static	struct sigframe {
		int	sf_signum;		/* 0x0 */
		int	sf_code;		/* 0x4 */
		struct	sigcontext *sf_scp;	/* 0x8 */
		int	(*sf_handler)();	/* 0xC */
		int	sf_kind;	/* 0x10: u_traptype */
		int	sf_regd0;	/* 0x14: previous value of D0 */
	/* from here to the end of the structure is saved by user code */
		int	sf_user[3];	/* 0x18: registers saved by user code */
					/* 0x24 */
	};
/*
 * Send a signal to a process - simulate an interrupt.
 */
/*ARGSUSED*/
sendsig(hdlr, signo, arg)
int (*hdlr)();
{
	register int	*regs;
	register struct user *up;
	register struct proc *p;
	struct sigcontext lscp, *uscp;
	struct sigframe lfp, *ufp;
	int oonstack;

#ifdef	HOWFAR
	extern char	*signame();

	TRACE(T_sendsig,
		("<%x,%s,%x> ", hdlr, signame(signo), arg));
#endif	HOWFAR
	up = &u;
	p = up->u_procp;
	regs	= up->u_ar0;

    if ((p->p_compatflags & COMPAT_BSDSIGNALS) == 0) {
	if (p->p_flag & SCOFF) {
		/*
		 *	We infer from the fact that the binary was in
		 *	Common Object File Format that the signal interface
		 *	is the new one.
		 */
		register int	*usp;

		usp	= (int *)regs[SP];
		(void) grow((unsigned) (usp - 5));
		/* simulate an interrupt on the user's stack */
		(void) suword((caddr_t) --usp, regs[PC]);
		(void) suword((caddr_t) --usp,
			(regs[RPS] & 0xffff) | (up->u_traptype << 16));
		(void) suword((caddr_t) --usp, arg);
		(void) suword((caddr_t) --usp, signo);
		regs[SP]	= (int) usp;
	} else {
		register unsigned long	usp;
		short	ps;

		usp	= regs[SP] - 6;
		ps	= (short) regs[RPS];
		(void) grow((unsigned) (usp - 3));
		(void) subyte((caddr_t) usp, ps >> 8);	/* high byte of ps */
		(void) subyte((caddr_t) (usp + 1), ps);	/* low byte of ps */
		(void) suword((caddr_t) (usp + 2), regs[PC]);
		regs[SP]	= (int) usp;
		regs[RPS]	&= ~PS_T;
	}
	regs[PC]	= (int) hdlr;
	if ((cputype == 68020) || (cputype == 68010)) {	 
		/* 
		 *	make sure looks like short format - clai 
		 *	020 still needs to store actual format somewhere - trip
		 */
		((struct buserr *)regs)->ber_format = 0;
		up->u_traptype = 0;
	}
    } else {
	oonstack = up->u_onstack;
	uscp = (struct sigcontext *)regs[SP] - 1;
	/* Once switching to the signal stack, we never switch off.
	 */
	if (!up->u_onstack && (up->u_sigonstack & mask(signo))) {
		ufp = (struct sigframe *)up->u_sigsp - 1;
		up->u_onstack = 1;
	} else
		ufp = (struct sigframe *)uscp - 1;
	/*
	 * Must build signal handler context on stack to be returned to
	 * so that rei instruction in sigcode will pop ps and pc
	 * off correct stack.  The remainder of the signal state
	 * used in calling the handler must be placed on the stack
	 * on which the handler is to operate so that the calls
	 * in sigcode will save the registers and such correctly.
	 */
	if (!oonstack) 
		grow((unsigned)ufp);
	if (!up->u_onstack)
		grow((unsigned)uscp);
	lfp.sf_signum = signo;
	if (signo == SIGILL || signo == SIGFPE) {
		lfp.sf_code = up->u_code;
		up->u_code = 0;
	} else
		lfp.sf_code = 0;
	lfp.sf_scp = uscp;
	lfp.sf_handler = hdlr;
	lfp.sf_regd0 = regs[R0];
	lfp.sf_kind = up->u_traptype;

	/* sigcontext goes on previous stack */
	lscp.sc_onstack = oonstack;
	lscp.sc_mask = arg;
	/* setup rei */
	lscp.sc_sp = regs[SP];
	lscp.sc_pc = regs[PC];
	lscp.sc_ps = regs[RPS];
	regs[SP] = (int)ufp;
	regs[RPS] &= ~PS_T;
	regs[PC] = (int)up->u_sigcode;
	if (copyout(&lfp, ufp, sizeof(lfp) - sizeof(lfp.sf_user)))
		goto bad;
	if (copyout(&lscp, uscp, sizeof(lscp)))
		goto bad;
	if ((cputype == 68020) || (cputype == 68010)) {
		((struct buserr *)regs)->ber_format = 0;
		up->u_traptype = 0;
	}
	return;

bad:
	badstack();
	return;
    }
}


/*	badstack -- kill off a process with a bad stack.
 *	Process has trashed its stack; give it an illegal
 *	instruction to halt it in its tracks.
 */

static
badstack()
{
	int	signo;
	register struct proc *p;

	p = u.u_procp;
	u.u_signal[SIGILL-1] = SIG_DFL;
	signo = mask(SIGILL);
	p->p_sigignore &= ~signo;
	p->p_sigcatch &= ~signo;
	p->p_sigmask &= ~signo;
	psignal(p, SIGILL);
}

/*
 * Routine to cleanup state after a signal
 * has been taken.  Reset signal mask and
 * stack state from context left by sendsig (above).
 * Pop these values in preparation for rei which
 * follows return from this routine.
 */
sigcleanup()
{
	struct sigframe lfp;
	struct sigcontext lscp;
	register struct user *up;
	register int	*regs;

	up = &u;
	regs = up->u_ar0;
	if ( (up->u_procp->p_compatflags & COMPAT_BSDSIGNALS) == 0)
		return(EINVAL);
	if(copyin(regs[SP], &lfp, sizeof(lfp) - sizeof(lfp.sf_user))) {
		badstack();
		return(EFAULT);
	}
	if (copyin(lfp.sf_scp, &lscp, sizeof(lscp))) {
		badstack();
		return(EFAULT);
	}
	up->u_onstack = lscp.sc_onstack & 01;
	up->u_procp->p_sigmask =
	    lscp.sc_mask &~ (mask(SIGKILL)|mask(SIGCONT)|mask(SIGSTOP));
	regs[R0] = lfp.sf_regd0;
	regs[SP] = lscp.sc_sp;
	regs[PC] = lscp.sc_pc;
	regs[RPS] = lscp.sc_ps & ~ (PS_SUP | PS_IPL /* | 0x1000 */);
	if (cputype == 68010) {
		if (lfp.sf_kind & TRAPLONG) 
			((struct buserr *)regs)->ber_format = 0x8;
	}
	else if (cputype == 68020) {
		if (lfp.sf_kind & TRAPLONG) { 
			((struct buserr *)regs)->ber_format = FMT_LONG;
		}
		else if (lfp.sf_kind & TRAPSHORT) {
			((struct buserr *)regs)->ber_format = FMT_SHORT;
		}
	}
	up->u_traptype = lfp.sf_kind;
	if (lfp.sf_kind & TRAPADDR) /* addr error */
		psignal(up->u_procp, SIGBUS);
	else if (lfp.sf_kind & TRAPBUS) /* bus error */
		psignal(up->u_procp, SIGSEGV);
	else {			/* no error */
		up->u_traptype = 0;
	}
	return (0);
}

/*
 * Clear registers on exec
 */
setregs(pnp)
struct pathname *pnp;
{
	register struct user *up;
	register char *cp;
	register i, (**rp)(), (**fptr)();
	extern (*execfunc[])();
	extern int compatflags;

	up = &u;
	up->u_procp->p_compatflags = compatflags;
	for (rp = &up->u_signal[0]; rp < &up->u_signal[NSIG]; rp++)
		if (((int)(*rp) & 1) == 0)
			*rp = 0;
	up->u_procp->p_sigcatch = 0;
	up->u_sigcode = 0;
	for (cp = &regloc[0]; cp < &regloc[15]; )
		up->u_ar0[*cp++] = 0;
	up->u_ar0[PC] = up->u_exdata.ux_entloc & ~01;
#ifndef lint	/* "constant in conditional context" when HOWFAR turned on */
	TRACE(T_machdep,("New pc = 0x%x\n", up->u_ar0[PC]));
#endif lint
	up->u_eosys = REALLYRETURN;
	for (i=0; i<NOFILE; i++)
		if ((up->u_pofile[i]&EXCLOSE) && up->u_ofile[i] != NULL) {
			closef(up->u_ofile[i]);
			up->u_ofile[i] = NULL;
		}
	for (i = 0; i < USERSIZ; i++)
		up->u_user[i] = 0;
	for (fptr = execfunc; *fptr; fptr++)
		(**fptr)();
	/*
	 * Remember file name for accounting.
	 */
	up->u_acflag &= ~AFORK;
	i = MIN(COMMSIZ, pnp->pn_pathlen);
	bcopy((caddr_t)pnp->pn_path, (caddr_t)up->u_comm, i);
	if (i < COMMSIZ)
		up->u_comm[i] = 0;
}

/*
 * dump out the core of a process
 */
coredump(vp)
register struct vnode *vp;
{
	register struct user *up;
	register preg_t	*prp;
	register proc_t *pp;
	register int	gap;
	int offset = 0;

	up = &u;
	/*	Put the region sizes into the u-block for the
	 *	dump.
	 */
	
	pp = up->u_procp;

	if(prp = findpreg(pp, PT_TEXT))
		up->u_tsize = prp->p_reg->r_pgsz;
	else
		up->u_tsize = 0;
	
	/*	In the following, we do not want to write
	**	out the gap but just the actual data.  The
	**	caluclation mirrors that in loadreg and
	**	mapreg which allocates the gap and the
	**	actual space separately.  We have to watch
	**	out for the case where the entire data region
	**	was given away by a brk(0).
	*/

	if(prp = findpreg(pp, PT_DATA)){
		up->u_dsize = prp->p_reg->r_pgsz;
		gap = btotp((caddr_t)up->u_exdata.ux_datorg - prp->p_regva);
		if(up->u_dsize > gap)
			up->u_dsize -= gap;
		else
			up->u_dsize = 0;
	} else {
		up->u_dsize = 0;
	}

	if(prp = findpreg(pp, PT_STACK)){
		up->u_ssize = prp->p_reg->r_pgsz;
	} else {
		up->u_ssize = 0;
	}

	/*	Check the sizes against the current ulimit and
	**	don't write a file bigger than ulimit.  If we
	**	can't write everything, we would prefer to
	**	write the stack and not the data rather than
	**	the other way around.
	*/

	if(USIZE + up->u_dsize + up->u_ssize > dtop(up->u_limit)){
		up->u_dsize = 0;
		if(USIZE + up->u_ssize > dtop(up->u_limit))
			up->u_ssize = 0;
	}

	/*	Write the u-block to the dump file.
	 */

	/*
	 * make register pointer relative for adb
	 */
	up->u_ar0 = (int *)((int)up->u_ar0 - (int)up);
	u.u_error = vn_rdwr(UIO_WRITE, vp,
	    (caddr_t)up,
	    ptob(v.v_usize),
	    0, UIOSEG_KERNEL, IO_UNIT, (int *)0);
	up->u_ar0 = (int *)((int)up->u_ar0 + (int)up);
	offset += ptob(v.v_usize);

	/*	Write the data and stack to the dump file.
	 */
	
	if(up->u_dsize){
		if (u.u_error == 0) {
			u.u_error = vn_rdwr(UIO_WRITE, vp,
			    (caddr_t)up->u_exdata.ux_datorg,
			    ptob(up->u_dsize) - 
					poff((caddr_t)up->u_exdata.ux_datorg),
			    offset, UIOSEG_USER, IO_UNIT, (int *)0);
		}
TRACE(T_machdep, ("coredump data: base=0x%x count=0x%x offset=0x%x vp=0x%x\n",
up->u_exdata.ux_datorg, ptob(up->u_dsize) - poff((caddr_t)up->u_exdata.ux_datorg),
offset, vp));

		offset += ptob(up->u_dsize);
	}
	if(up->u_ssize){
		if (u.u_error == 0)
			u.u_error = vn_rdwr(UIO_WRITE, vp,
				(caddr_t)(USRSTACK-ptob(up->u_ssize)),
				ptob(up->u_ssize),
			    offset, UIOSEG_USER, IO_UNIT, (int *)0);
TRACE(T_machdep, ("coredump stack: base=0x%x count=0x%x offset=0x%x vp=0x%x\n",
up->u_exdata.ux_datorg, ptob(up->u_ssize), offset, vp));
	}
TRACE(T_machdep, ("coredump returning\n"));
}


/*	Clear an entire atb.
 */

/*ARGSUSED*/
clratb(flags)
int	flags;
{
	/* UNFORTUNATELY clratb / invsatb must invalidate the whole cache
		by writing the root pointer currently
	 */
	fl_atc();
}

/*	Invalidate a single atb.
 */

/*ARGSUSED*/
invsatb(flags, vaddr, count)
int	flags;
caddr_t vaddr;
int	count;
{
	if (flags == SYSATB)
		fl_sysatc();
	else
		fl_usratc();
}

/*
 * ovbcopy(f, t, l)
 *	copy from one buffer to another that are possibly overlapped
 *	this is like bcopy except that f and t may be overlapped in
 *	any manner, so you may need to copy from the rear rather than
 *	from the front
 * parameters
 *	char *f;	from address
 *	char *t;	to address
 *	int l;		length
 */
ovbcopy(f, t, l)
register char *f, *t;
register int l;
{
	if (f != t)
		if (f < t) {
			f += l;
			t += l;
			while (l-- > 0)
				*--t = *--f;
		} else {
			while (l-- > 0)
				*t++ = *f++;
		}
}

#ifdef TRACE
int	nvualarm;

vtrace()
{
	register struct a {
		int	request;
		int	value;
	} *uap;
	int vdoualarm();

	uap = (struct a *)u.u_ap;
	switch (uap->request) {

	case VTR_DISABLE:		/* disable a trace point */
	case VTR_ENABLE:		/* enable a trace point */
		if (uap->value < 0 || uap->value >= TR_NFLAGS)
			u.u_error = EINVAL;
		else {
			u.u_rval1 = traceflags[uap->value];
			traceflags[uap->value] = uap->request;
		}
		break;

	case VTR_VALUE:		/* return a trace point setting */
		if (uap->value < 0 || uap->value >= TR_NFLAGS)
			u.u_error = EINVAL;
		else
			u.u_rval1 = traceflags[uap->value];
		break;

	case VTR_UALARM:	/* set a real-time ualarm, less than 1 min */
		if (uap->value <= 0 || uap->value > 60 * v.v_hz ||
		    nvualarm > 5)
			u.u_error = EINVAL;
		else {
			nvualarm++;
			timeout(vdoualarm, (caddr_t)u.u_procp->p_pid,
			    uap->value);
		}
		break;

	case VTR_STAMP:
		trace(TR_STAMP, uap->value, u.u_procp->p_pid);
		break;
	}
}

vdoualarm(arg)
	int arg;
{
	register struct proc *p;

	p = pfind(arg);
	if (p)
		psignal(p, SIGURG);
	nvualarm--;
}

/*VARARGS*/
trace1(args)
	int args;
{
	register int nargs;
	register int x;
	register int *argp, *tracep;

	nargs = 4;
	x = tracex % TRCSIZ;
	if (x + nargs >= TRCSIZ) {
		tracex += (TRCSIZ - x);
		x = 0;
	}
	argp = &args;
	tracep = &tracebuf[x];
	tracex += nargs;
	*tracep++ = (time.tv_sec%1000)*1000 + (time.tv_usec/1000);
	nargs--;
	do
		*tracep++ = *argp++;
	while (--nargs > 0);
}
#endif TRACE
/* <@(#)machdep.c	6.4> */
