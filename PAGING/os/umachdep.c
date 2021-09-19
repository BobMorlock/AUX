/*	@(#)umachdep.c	UniPlus VVV.2.1.14	*/

#ifdef HOWFAR
extern int	T_hardflt;
extern int	T_hardsegflt;
extern int	T_umachdep;
#ifdef AUTOCONFIG
int	T_mmuinit=0;
#else
extern int	T_mmuinit;
#endif AUTOCONFIG
extern int	T_dophys;
#endif HOWFAR

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/param.h"
#include "sys/uconfig.h"
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/sysmacros.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/map.h"
#include "sys/reg.h"
#include "sys/psl.h"
#include "sys/utsname.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/var.h"
#include "sys/acct.h"
#include "sys/file.h"
#include "sys/vnode.h"
#include "sys/debug.h"
#include "setjmp.h"
#include "sys/uio.h"
#include "sys/pmmu.h"
#include "sys/buserr.h"
#include "compat.h"
#endif lint

#define LOW	(USTART&0xFFFF)		/* Low user starting address */
#define HIGH	((USTART>>16)&0xFFFF)	/* High user starting address */

#ifdef	MMB
ste_t *rootbl = 0;		/* function code table */
ste_t *mmbtbla = 0;		/* table A for MMB implementation only */
#else	MMB
extern ste_t sup_rp;           	/* long descriptor to load into srp */
				/* 	(supv root ptr) now in local.s */
#endif MMB
#ifdef PMMU
extern ste_t cpu_rp;           	/* long descriptor to load into crp */
				/*	(cpu root ptr) now in local.s */
#endif PMMU
ste_t *kstbl = 0;		/* kernel segment table */
/* int cpuversion;		/* cpu version number */

extern pte_t *copyptbl;
extern int runtime;

/*
 * Icode is the bootstrap program used to exec init.
 */
short icode[] = {
				/*	 .=USTART		*/
	0x2E7C,	HIGH, LOW+0x400,/*	 movl	#USTART+0x400,sp*/
	0x227C,	HIGH, LOW+0x26,	/*	 movl	#envp,a1	*/
	0x223C,	HIGH, LOW+0x22,	/*	 movl	#argp,d1	*/
	0x207C,	HIGH, LOW+0x2A,	/*	 movl	#name,a0	*/
	0x42A7,			/*	 clrl	sp@-		*/
	0x303C,	0x3B,		/*	 movw	#59.,d0		*/
	0x4E40,	 		/*	 trap	#0		*/
	0x60FE,			/*	 bra	.		*/
	HIGH,	LOW+0x2A,	/* argp: .long	name		*/
	0,	0,		/* envp: .long	0		*/
	0x2F65,	0x7463,	0x2F69,	/* name: .asciz	"/etc/init"	*/
	0x6E69,	0x7400
};
int szicode = sizeof(icode);

/*
 * mmualloc - allocate space for static translation tables and kernel udot,
 *	then clear tables and udot
 *	note: mch.s clears from edata to ptob(btop(tblstart))
 */
mmualloc(tblstart)
caddr_t tblstart;		/* _end */
{
	register int cp, click;

	cp = (int)ptround(tblstart);
#ifdef MMB
	rootbl = (ste_t *)cp;
	cp += NFC * (sizeof(ste_t));		/* size of root table */
	cp = (int)ptround(cp);

	mmbtbla = (ste_t *)cp;
	cp += NTBLA * (sizeof(ste_t));	/* size of table A */
	cp = (int)ptround(cp);
#endif MMB
	kstbl = (ste_t *)cp;
#ifdef	MMB
	cp += NSTBL * (sizeof(ste_t));		/* size of seg table */
#else	MMB
	cp += MAXSEG * (sizeof(ste_t));         /* size of kernel seg table */
#endif	MMB
	cp = (int)ptround(cp);
#ifdef MMB
	AUTO_TRACE(T_mmuinit,("rootbl=0x%x, ", rootbl));
	AUTO_TRACE(T_mmuinit,("mmbtbla=0x%x, ", mmbtbla));
#endif MMB
	click = btop(cp);
	cp = ptob(click);		/* round to next click after tables */
	for (; click >= btop((int)tblstart); click--)/* clear tables and udot */
		clear((caddr_t)ptob(click), ptob(1));
	return(cp);			/* start of udot */
}

/*
 * mmuinit - initialize mmu
 */
mmuinit()
{
	register ste_t *tbl;

	oem_mmuinit();
#ifdef MMB
#ifdef PMMUMMB
	wtrte(cpu_rp, RT_VALID, 8, (long)rootbl);
	AUTO_TRACE(T_mmuinit,("cpu_rp(0x%x) = 0x%x 0x%x\n", &cpu_rp,
			cpu_rp.segf.ld_msadr, cpu_rp.segf.ld_lsadr));
#endif PMMUMMB
	/* set up two user entries in root table - user accesses go through a
	   table A with four entries - located at mmbtbla[32] */
	wtrte(rootbl[FCUSERD], RT_KURW, 4, (long)&mmbtbla[TAUINDEX]);
	wtrte(rootbl[FCUSERP], RT_KURW, 4, (long)&mmbtbla[TAUINDEX]);
	/* set up two supervisor entries in root table - kernel can access
	   anything in table A including the user's entries */
	wtrte(rootbl[FCSUPVD], RT_KRW, NTBLA, (long)&mmbtbla[TAKINDEX]);
	wtrte(rootbl[FCSUPVP], RT_KRW, NTBLA, (long)&mmbtbla[TAKINDEX]);
#else MMB		/* PMMU */
	wtrte(cpu_rp, RT_VALID, NSTBL, (long)&u.u_stbl[0]);
	wtrte(sup_rp, (RT_VALID|STSG), MAXSEG, (long)&kstbl[0]);
	AUTO_TRACE(T_mmuinit,("cpu_rp(0x%x) = 0x%x 0x%x\n", &cpu_rp,
			cpu_rp.segf.ld_msadr, cpu_rp.segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("sup_rp(0x%x) = 0x%x 0x%x\n", &sup_rp,
			sup_rp.segf.ld_msadr, sup_rp.segf.ld_lsadr));
#endif MMB

#ifdef MMB
	/* set up mmbtbla - this is the only table A for the MMB implementation
	   and is used only because the MMB must have three levels of tables */
	/* set up kernel entry in table A */
	wtrte(mmbtbla[TAKINDEX], RT_KRW, NSTBL, (long)&kstbl[0]);
	/* invalidate table A entries from kernel to end
	   (including user entries) */
	for (tbl = &mmbtbla[TAKINDEX+1]; tbl < &mmbtbla[NTBLA]; tbl++)
		tbl->segm.ld_dt = RT_IVD;	/* desc type = invalid */
	/* Set up kernel interrupt entry in table A */
	/* Needed because of 68020 bug having faulty user and system code in */
	/* instruction stream cache */
	wtrte(mmbtbla[TAKLASTINDEX], RT_KRW, NSTBL, (long)&kstbl[0]);
	/* set up I/O entries in table A (last entry) */
	/* Note that the next four wtrte's are equivalent to the previous one */
	wtrte(mmbtbla[anum(PHYSIOMAP)], RT_KRW, NSTBL, (long)&kstbl[0]);
	AUTO_TRACE(T_mmuinit,("rootbl[FCUSERD] = 0x%x 0x%x  ",
		rootbl[FCUSERD].segf.ld_msadr, rootbl[FCUSERD].segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("rootbl[FCUSERP] = 0x%x 0x%x\n",
		rootbl[FCUSERP].segf.ld_msadr, rootbl[FCUSERP].segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("rootbl[FCSUPVD] = 0x%x 0x%x  ",
		rootbl[FCSUPVD].segf.ld_msadr, rootbl[FCSUPVD].segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("rootbl[FCSUPVP] = 0x%x 0x%x\n",
		rootbl[FCSUPVP].segf.ld_msadr, rootbl[FCSUPVP].segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("mmbtbla[TAKINDEX]=0x%x 0x%x\n",
		mmbtbla[TAKINDEX].segf.ld_msadr,
		mmbtbla[TAKINDEX].segf.ld_lsadr));
	AUTO_TRACE(T_mmuinit,("rootbl[FCUSERD]   (%x): ", &rootbl[FCUSERD]));
	AUTO_TRACE(T_mmuinit,("rootbl[FCUSERP]   (%x): ", &rootbl[FCUSERP]));
	AUTO_TRACE(T_mmuinit,("rootbl[FCSUPVD]   (%x): ", &rootbl[FCSUPVD]));
	AUTO_TRACE(T_mmuinit,("rootbl[FCSUPVP]   (%x): ", &rootbl[FCSUPVP]));
	AUTO_TRACE(T_mmuinit,("mmbtbla[TAKINDEX]    (%x): ", &mmbtbla[TAKINDEX]));
#endif MMB
}

/*
 * iocheck - check for an I/O device at given address
 */
iocheck(addr)
caddr_t addr;
{
	register int *saved_jb;
	register int i;
	jmp_buf jb;

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		i = *addr;
		i = 1;
	} else
		i = 0;
	nofault = saved_jb;
	return(i);
}

/*
 * busaddr - Save the info from a bus or address error.
 */
/* VARARGS */
busaddr(frame)
struct {
	long regs[4];	/* d0,d1,a0,a1 */
	int baddr;	/* bsr return address */
	short fcode;	/* fault code */
	int aaddr;	/* fault address */
	short ireg;	/* instruction register */
} frame;
{
	u.u_fcode = frame.fcode;
	u.u_aaddr = frame.aaddr;
	u.u_ireg = frame.ireg;
}

/*
 * dophys - machine dependent set up portion of the phys system call
 */
dophys(phnum, laddr, bcount, phaddr)
unsigned phnum, laddr, phaddr;
register unsigned bcount;
{
	register struct phys *ph;
	register struct user *up;
	register struct region *rp;
	register struct pregion *prp;
	preg_t *vtopreg();

	up = &u;
	TRACE(T_dophys, ("dophys(%d, 0x%x, 0x%x, 0x%x)\n", phnum, laddr, bcount, phaddr));
	if (phnum >= v.v_phys) {
		TRACE(T_dophys, ("dophys: phnum out of range\n"));
		goto bad;
	}
	if(soff(laddr)) {
		TRACE(T_dophys, ("dophys: laddr 0x%x not on segment boundary\n",
									laddr));
		goto bad;
	}

	ph = &up->u_phys[(short)phnum];
	ph->u_phladdr = 0;
	ph->u_phsize = 0;
	ph->u_phpaddr = 0;
/* if a region is already phys'ed in, then detach it */
	if (ph->u_phprp != NULL) {
		TRACE(T_dophys, ("dophys: detaching old prp 0x%x\n", ph->u_phprp));
		reglock(ph->u_phprp->p_reg);
		pinvalidate(ph->u_phprp);
		detachreg(ph->u_phprp, up);
		ph->u_phprp = NULL;
	}
/* bcount is zero means we were to clear the mapping and return */
	if (bcount == 0) {
		TRACE(T_dophys, ("dophys: bcount == 0 done\n"));
		return;
	}
/* allocate attach and validate a phys region */
	if((rp = allocreg((struct vnode *)NULL, RT_PHYSCALL)) == NULL){
		TRACE(T_dophys, ("dophys: allocreg failed\n"));
		goto bad;
	}
	/*
	 * Is the attach address already in use
	 */
	if (vtopreg(up->u_procp, (caddr_t)laddr) != (preg_t *)NULL) {
		freereg(rp);
		TRACE(T_dophys, ("dophys: vaddr in use\n"));
		goto bad;
	}
	rp->r_flags |= RG_NOSWAP;
	TRACE(T_dophys, ("dophys: allocreg rp=0x%x\n", rp));
/* attachreg checks segment alignment of laddr */
	if ((prp = attachreg(rp, &u, (caddr_t)laddr, PT_PHYSCALL, SEG_RW))==NULL) {
		freereg(rp);
		TRACE(T_dophys, ("dophys: attachreg failed\n"));
		goto bad;
	}
	TRACE(T_dophys, ("dophys: attachreg prp=0x%x\n", prp));
/* growreg fills in zeroed page tables */
	if((growreg(prp, (int)btop(bcount), DBD_NONE)) < 0) {
		detachreg(prp, &u);
		TRACE(T_dophys, ("dophys: growreg failed\n"));
		goto bad;
	}
	regrele(rp);
	ph->u_phprp = prp;
/* pvalidate maps ptes to phys area (phaddr) */
	pvalidate(rp, (int)btop(bcount), (int)btop(phaddr));
	ph->u_phladdr = laddr;
	ph->u_phsize = btop(bcount);
	ph->u_phpaddr = phaddr;
	return;
bad:
	up->u_error = EINVAL;
}

/* 
 * pvalidate loads the zero filled pts with actual values
 */
pvalidate(rp, pg, phaddr)
register struct region *rp;
{
	register pte_t		**rlist;
	register pte_t		*pt;

	rlist = rp->r_list;
	pt = *rlist;
	while (pg--) {
		if (pt > &(rlist[0][NPGPT])) {
			if (*(++rlist) == NULL) {
				printf("pvalidate(%x,%x,%x) failed\n",
							rp, pg, phaddr);
				break;
			}
			pt = *rlist;
		}
		wtpte(pt->pgi, PG_V|PG_RW, phaddr++);
		pg_setlock(pt);
		pt++;
	}
	clratb(USRATB);
}

/* 
 * pinvalidate clears the ptes for this region so detachreg won't pfree them
 */
pinvalidate(prp)
register struct pregion *prp;
{
	register pte_t		**rlist;
	register pte_t		*pt;

	rlist = prp->p_reg->r_list;
	for (;pt = *rlist; rlist++)
		for (;pt <= &(rlist[0][NPGPT]);pt++)
			pg_zero(pt);
	clratb(USRATB);
}

/*
 * addupc - Take a profile sample.
 */
addupc(pc, p, incr)
unsigned pc;
register struct {
	short	*pr_base;
	unsigned pr_size;
	unsigned pr_off;
	unsigned pr_scale;
} *p;
{
	union {
		int w_form;		/* this is 32 bits on 68000 */
		short s_form[2];
	} word;
	register short *slot;

	slot = &p->pr_base[((((pc - p->pr_off) * p->pr_scale) >> 16) + 1)>>1];
	if ((caddr_t)slot >= (caddr_t)(p->pr_base) &&
	    (caddr_t)slot < (caddr_t)((unsigned)p->pr_base + p->pr_size)) {
		if ((word.w_form = fuword((caddr_t)slot)) == -1)
			u.u_prof.pr_scale = 0;	/* turn off */
		else {
			word.s_form[0] += (short)incr;
			(void) suword((caddr_t)slot, word.w_form);
		}
	}
}

#ifdef notdef
/*
 * dump the present contents of the stack
 */
dumpstack(ret)
{
	register unsigned short *ip;

	ip = (unsigned short *)&ret;
	printf("\n%x  ", ip);
	while (ip < (unsigned short *)((int)&u+ptob(v.v_usize))) {
		if (((int)ip & 31) == 0)
			printf("\n%x  ", ip);
		printf(" %x", *ip++);
	}
	printf("\n");
	if (ret != 0)
		panic("**** ABORTING ****");
}
#endif notdef

/* ARGSUSED */
dumpmm(f)
{
}

/* 
 * hardflt(): pagein routine
 * Is this a hard bus error? Or can we recover with pfault/vfault?
 * Should only be called for USER mode bus errors. Can be called by the
 * kernel on behalf of the user
 */
hardflt(regs)
struct lbuserr regs;
{
	register struct lbuserr *ap = &regs;
	register caddr_t vaddr;
	register caddr_t dfaddr = (caddr_t)NULL;
	int retval;
#ifdef mc68881
	extern short fp881;		/* is there an MC68881? */
#endif mc68881
#define supv_mode (ap->ber_regs[RPS] & RPS_SUP)

TRACE(T_hardflt,("hardflt: vaddr=0x%x ssw=0x%x\n",ap->ber_faddr,ap->ber_sstat));

#ifdef FLOAT
	if (u.u_fpinuse && !supv_mode) {
		savfp();
		u.u_fpsaved = 1;
	}
#else
#ifdef mc68881
	if (!supv_mode) {
		u.u_fpsaved = 0;
		fpsave();
	}
#endif mc68881
#endif FLOAT

	/*
	 * DATA FAULT: If the DF bit is set in the ssw then fault in
	 * data page at the data cycle fault address (ber_faddr)
	 */
	if (ap->ber_ssw.df) {
		vaddr = (caddr_t)ap->ber_faddr;
		if (supv_mode) {
#ifdef UMVSEG
			if (btots((int)vaddr) == UMOVESEG)
				vaddr = (caddr_t)stob(btots((int)u_faddr))
						+ soff(vaddr);
#else UMVSEG
			if((int)vaddr >= USEROFF &&
				(int)vaddr < USEROFF + tblatova(TAUNUM))
				vaddr -= USEROFF;
#endif UMVSEG
			else {
				retval = error("hardflt: KDFAULT\n");
				goto out;
			}
		} else {
			/*
			 * Was the fault due to user write protection at 
			 * the segment level (i.e. not from copy-on-write)?
			 */
			if ((ap->ber_ssw.rw == 0) && 
			    isreadonly(u.u_stbl[snum(vaddr)])) {
				retval = 
				    error("hardflt:USER WP FAULT: vaddr=0x%x\n",
					    (int)vaddr);
				goto out;
			}
		}
		if (hardsegflt(vaddr) < 0) {
			retval = error("hardflt: %s DFAULT: vaddr=0x%x\n", 
				    (int)(supv_mode ? "KERNEL" : "USER"),
				    (int)vaddr);
			goto out;
		}
		dfaddr = vaddr;
	} else if (supv_mode) {
		retval = error("hardflt: KIFAULT\n");
		goto out;
	}

	/*
	 * INSTRUCTION FAULT: faults on instruction prefetch in 
	 * the pipeline occur because the instruction has been paged out. 
	 * If the FC or FB bit is set we need to fault in the page 
	 * at the appropriate PC. If we faulted in a text page set 
	 * the appropriate rerun bit in the ssw.
	 */
	if (ap->ber_ssw.fc || ap->ber_ssw.fb) {
		if ((ap->ber_format & FMT_FMASK) == FMT_LONG)
			vaddr = (caddr_t)ap->ber_baddr;
		else
			vaddr = (caddr_t)(ap->ber_regs[PC] + 4);
		if (ap->ber_ssw.fb) 
			ap->ber_ssw.rb = 1;
		else if (ap->ber_ssw.fc) {
			ap->ber_ssw.rc = 1;
			vaddr -= 2;
		}
		if (!dfaddr || (btop((int)dfaddr) != btop((int)vaddr)))
			if (hardsegflt(vaddr) < 0) {
				retval =  error("hardflt: IFAULT: vaddr=0x%x\n",
							(int)vaddr);
				goto out;
			}
	} 

	if ((runtime & RT_A45J) && !(ap->ber_intr1.prr)) {
		register unsigned long *dib;

		/*
		 * XC68020 - A45J ERRATA, 4 April 1985
		 * check for pending data cycle completion during 
		 * RTE where we must complete a data transfer for 
		 * the processor before fixing up the pending rerun
		 */
		if ((ap->ber_format&FMT_FMASK) == FMT_LONG)
			dib = &ap->ber_dib;
		else
			dib = NULL;	/* no dib on short frame */

		if(ap->ber_ssw.df) {
			if (ap->ber_ssw.rw) {
				if ((cyclefix((int)ap->ber_ssw.rw,
				    (int)ap->ber_ssw.siz, (int)ap->ber_faddr, 
				    (char *)dib)) != -1)
					ap->ber_ssw.df = 0;
				else {
					retval = -1;
					goto out;
				}
			} else {
				if ((cyclefix((int)ap->ber_ssw.rw,
				    (int)ap->ber_ssw.siz, (int)ap->ber_faddr,
				    (char *)&ap->ber_dob)) != -1)
					ap->ber_ssw.df = 0;
				else {
					retval = -1;
					goto out;
				}
			}
		}

		/* now fix up pending rerun */

		if (ap->ber_intr1.rw) {
			if ((cyclefix((int)ap->ber_intr1.rw,
			    (int)ap->ber_intr1.siz, (int)ap->ber_intr2,
			    (char *)dib)) != -1)
				ap->ber_intr1.prr = 1;
			else {
				retval = -1;
				goto out;
			}
		} else {
			if ((cyclefix((int)ap->ber_intr1.rw, 
			    (int)ap->ber_intr1.siz, (int)ap->ber_intr2,
			    (char *)&ap->ber_dob)) != -1)
				ap->ber_intr1.prr = 1;
			else {
				retval = -1;
				goto out;
			}
		}
	}

	/* Successful pagein completed.  If this was a user 
	 * page fault then check for pending signals before 
	 * returning to user mode.
	 */
	if (!supv_mode) {
		register struct proc *p = u.u_procp;
		int *save_ar0;

		/* The following code implements 
		 *	if (p->p_sig && issig()) psig();
		 */
		save_ar0 = u.u_ar0;
		u.u_ar0 =  ap->ber_regs;
		if (ap->ber_format == FMT_LONG)
			u.u_traptype = TRAPLONG;
		else if (ap->ber_format == FMT_SHORT)
			u.u_traptype = TRAPSHORT;
		else
			u.u_traptype = TRAPNORM;
		if (p->p_sig) {
			SPLHI();
			if (issig()) {
				SPL0();
				psig(ap->ber_regs[PC]);
			} else
				SPL0();
		}
		u.u_ar0 = save_ar0;
	}
	retval = 0;

out:
#ifdef FLOAT
	if (u.u_fpinuse && u.u_fpsaved && !supv_mode)
		restfp();
#else
#ifdef mc68881
	if (fp881 && u.u_fpsaved && !supv_mode)
		fprest();
#endif mc68881
#endif FLOAT
	return(retval);
}
#undef supv_mode

cyclefix(rw,siz,vaddr,db)
register int rw, siz, vaddr;
register char *db;
{
	register int i;
	register int rval;

	/* calculate number of bytes to move */
	if (siz == 0)
		siz = 4;
	db += (4 - siz);
	if (rw) {
		if(db == NULL) {
			printf("cyclefix: short stackframe buffer error \n");
			return(-1);
		}
		for (i = 0; i <siz;i++) {
			if ((rval = fubyte((caddr_t)vaddr++)) == -1)
				return (error("cyclefix read: vaddr=0x%x\n",
									vaddr));
			*db++ = (char) rval;
		}
	} else {
		for (i = 0; i <siz;i++)
			if ((rval = subyte((caddr_t)vaddr++, *db++)) == -1)
				return (error("cyclefix write: vaddr=0x%x\n",
									vaddr));
	}
	return(0);
}

hardsegflt(vaddr)
register caddr_t vaddr;
{
	register pte_t *ptp, *ptep, *vptep;
	register uint vdbdtype;
	pte_t *phystosvir();

	/* 
	 * bus error beyond the end of user virtual address space
	 */
	if ((unsigned)vaddr >= (unsigned)USRSTACK)
		return(error("hardsegflt: 0x%x >= 0x%x\n", (int)vaddr, USRSTACK));

	ptp = (pte_t *)steaddr(u.u_stbl[snum(vaddr)]);
	if(ptp != (pte_t *)NULL) {
		ptep = ptp + pnum(vaddr);
		vptep = phystosvir((char *)ptep);
		vdbdtype = dbdget(vptep)->dbd_type;
	}
	if ((ptp == (pte_t *)NULL) || (vptep == (pte_t *)NULL)
	    || ((vptep->pgi[0].pg_pte == 0) && (vdbdtype == DBD_NONE))) {

TRACE(T_hardsegflt,
("hardsegflt: call grow(vaddr=0x%x) ptep=0x%x vptep=0x%x\n",vaddr,ptep,vptep));

		if (!grow((unsigned)vaddr))
			return(error("hardsegflt: grow %x fail\n", (int)vaddr));
		ptp = (pte_t *)steaddr(u.u_stbl[snum(vaddr)]);
		if(ptp != (pte_t *)NULL) {
			ptep = ptp + pnum(vaddr);
			vptep = phystosvir((char *)ptep);
			vdbdtype = dbdget(vptep)->dbd_type;
		}
		if ((ptp == (pte_t *)NULL) || (vptep == (pte_t *)NULL)
	    	|| ((vptep->pgi[0].pg_pte == 0) && (vdbdtype == DBD_NONE))){
			return(error("hardsegflt: grow %x botch\n", (int)vaddr));
		}
	}

TRACE(T_hardsegflt,
("hardsegflt: vaddr=0x%x ptep=0x%x vptpep=0x%x=0x%x\n",vaddr,ptep,vptep,
vptep->pgi[0].pg_pte));

	if (!vptep->pgm[0].pg_v) {
		if (vfault(vaddr, ptep) <= 0)
			return(error("hardsegflt: vfault botch\n"));
	} else
	if (vptep->pgm[0].pg_cw) {
		if (pfault(vaddr, ptep) == 0)
			return(error("hardsegflt: pfault botch\n"));
	} else
		return(error("hardsegflt: page is valid\n"));
	return(0);
}

/* VARARGS */
error(s1,s2,s3,s4,s5,s6,s7,s8,s9)
char *s1;
{
#ifdef lint
	printf("%s %d %d %d %d %d %d %d %d", 
		s1,s2,s3,s4,s5,s6,s7,s8,s9);
#endif lint
	TRACE(s1,(s1,s2,s3,s4,s5,s6,s7,s8,s9));
	return (-1);
}

#ifdef FLOAT		/* sky floating point board */
fpresume()
{
	/*
	 * save the floating point info per user
	 */
	if (u.u_fpinuse && u.u_fpsaved==0) {
		savfp();
		u.u_fpsaved = 1;
	}
}
#endif FLOAT

ldustbl()
{
#ifndef MMB
	register ste_t *a2;
#endif MMB
	register i;
	register ste_t *upstbl;

#ifdef UMVSEG
 	/*
 	 * restore the COPYSEG address per user
 	 */
 	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
#endif UMVSEG

 	/*
 	 * restore the user page tables in the beginning of the new udot
 	 */
	upstbl=(ste_t *)(((long)(u.u_procp->p_uptbl[0].pgi[0].pg_pte))&~(0xFF));
#ifdef	MMB
	for (i=TAUINDEX; i < TAUINDEX+TAUNUM; i++,upstbl+=MAXSEG)
		wtrte(mmbtbla[i], RT_KURW, NSTBL, (long)upstbl);
#else	MMB
	wtrte(cpu_rp, RT_VALID, NSTBL, (long)upstbl);
	a2 = &cpu_rp;
	/*      pmove   a2@,crp  */
	asm("short      0xF012");
	asm("short      0x4C00");
#endif	MMB
	TRACE(T_umachdep,("ldustbl addr=0x%x\n",  (long)upstbl));
}

#ifdef	HOWFAR
char	*signames[] = {
	"signal 0",
	"SIGHUP",
	"SIGINT",
	"SIGQUIT",
	"SIGILL",
	"SIGTRAP",
	"SIGIOT",
	"SIGEMT",
	"SIGFPE",
	"SIGKILL",
	"SIGBUS",
	"SIGSEGV",
	"SIGSYS",
	"SIGPIPE",
	"SIGALRM",
	"SIGTERM",
	"SIGUSR1",
	"SIGUSR2",
	"SIGCLD",
	"SIGPWR",
	"SIGTSTP",
	"SIGTTIN",
	"SIGTTOU",
	"SIGSTOP",
	"SIGXCPU",
	"SIGXFSZ",
	"SIGVTALRM",
	"SIGPROF",
	"SIGWINCH",
	"SIGCONT",
	"Signal 30",
	"Signal 31",
	"Signal 32"
};

char	*
signame(n)
{
	if (n >= 0 && n < sizeof(signames) / sizeof(signames[0]))
		return (signames[n]);
	else
		return ("unknown (signal out of range)");
}
#endif HOWFAR
