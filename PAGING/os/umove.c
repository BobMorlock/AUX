/*	@(#)umove.c	UniPlus VVV.2.1.3	*/

#ifdef HOWFAR
extern int	T_subyte;
#endif HOWFAR

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/page.h"
#include "sys/systm.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#include "sys/debug.h"
#include "setjmp.h"
#include "sys/pmmu.h"
#include "sys/trace.h"
#include "sys/var.h"
#endif lint

/* This file "os/umove.c" corresponds with the 3B20's file "ml/move.s". */

extern pte_t *copyptbl;
extern pte_t *copypte;
pte_t *phystosvir();

/*
 * fuword - Fetch word from user space.
 */
fuword(addr)
caddr_t addr;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(int)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fuword: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(int)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fuword: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		val = *(int *)(addr+USEROFF);
#else	UMVSEG
		if (soff(addr) >= (stob(1) - 2)) {
			val = (*(short *)(stob(UMOVESEG) + soff(addr)));
			u_ste = (int)steaddr(u.u_stbl[snum(addr)+1]);
			wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
			clratb(SYSATB);
			val = val << 16 | (*(short *)(stob(UMOVESEG)));
		} else
			val = (*(int *)(stob(UMOVESEG) + soff(addr)));
#endif	UMVSEG
	} else
		val = -1;
	nofault = saved_jb;
#ifdef	UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * fubyte - Fetch byte from user space.
 */
fubyte(addr)
register caddr_t addr;
{
	register unsigned int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fubyte: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("fubyte: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		val = *(char *)(addr+USEROFF) & 0xFF;
#else	UMVSEG
		val = (*(char *)(stob(UMOVESEG) + soff(addr)))&0xFF;
#endif	UMVSEG
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * suword - Store word into user space.
 */
suword(addr, word)
caddr_t addr;
int word;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(word)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("suword: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(word)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("suword: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		*(int *)(addr+USEROFF) = word;
#else	UMVSEG
		if (soff(addr) >= (stob(1) - 2)) {
			*(short *)(stob(UMOVESEG) + soff(addr)) =
				(word >> 16) &0xFFFF;
			u_ste = (int)steaddr(u.u_stbl[snum(addr)+1]);
			wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
			clratb(SYSATB);
			*(short *)(stob(UMOVESEG)) = word & 0xFFFF;
		} else
			*(int *)(stob(UMOVESEG) + soff(addr)) = word;
#endif UMVSEG
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * subyte - Store byte into user space.
 */
subyte(addr, byte)
register caddr_t addr;
char byte;
{
	int val;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int ofaddr;
	register int oste;

	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("subyte: to address 0x%x failed\n",addr));
		return(-1);
	}
	ofaddr = u_faddr;
	u_faddr = (int)addr;
	oste = u_ste;
	u_ste = (int)steaddr(u.u_stbl[snum(addr)]);
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else
	if (addr < (caddr_t)v.v_ustart ||
	    (addr+sizeof(char)) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("subyte: to address 0x%x failed\n",addr));
		return(-1);
	}
#endif UMVSEG

	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
#ifndef	UMVSEG
		*(char *)(addr+USEROFF) = byte & 0xFF;
#else UMVSEG
		*(char *)(stob(UMOVESEG) + soff(addr)) = byte&0xFF;
#endif UMVSEG
		val = 0;
	} else
		val = -1;
	nofault = saved_jb;
#ifdef UMVSEG
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#endif UMVSEG
	return(val);
}

/*
 * copyout - Move bytes out of the system into user's address space.
 */
copyout(from, to, nbytes)
register caddr_t from, to;
unsigned nbytes;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;

	if (to < (caddr_t)v.v_ustart ||
	    (to+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyout: to address 0x%x failed\n",to));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes != 0) {
		u_faddr = (int)to;
		u_ste = (int)steaddr(u.u_stbl[snum(to)]);
TRACE(T_subyte,("copyout: wtste into kstbl[UMOVESEG] = 0x%x\n",u_ste));
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)to & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			blt((caddr_t)(stob(UMOVESEG) + soff(to)), from, count);
		} else {
			val = EFAULT;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		from + = count;
		to + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else	UMVSEG
	if (to < (caddr_t)v.v_ustart ||
	    (to+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyout: to address 0x%x failed\n",to));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt((caddr_t)(to+USEROFF), from, nbytes);
	} else {
		val = EFAULT;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}

/*
 * copyin - Move bytes into the system space out of user's address space.
 */
copyin(from, to, nbytes)
register caddr_t from, to;
unsigned nbytes;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;

	if (from < (caddr_t)v.v_ustart ||
	    (from+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyin: to address 0x%x failed\n",from));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes != 0) {
		u_faddr = (int)from;
		u_ste = (int)steaddr(u.u_stbl[snum(from)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)from & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			blt(to, (caddr_t)(stob(UMOVESEG) + soff(from)), count);
		} else {
			val = EFAULT;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		from + = count;
		to + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else	UMVSEG
	if (from < (caddr_t)v.v_ustart ||
	    (from+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("copyin: to address 0x%x failed\n",from));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		blt(to, (caddr_t)(from+USEROFF), nbytes);
	} else {
		val = EFAULT;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}

/*
 * bcopyin - Move bytes into the system space out of user's address space.
 *	copy only until a null or nbytes
 *	return number of bytes
 */
bcopyin(from, to, nbytes)
register caddr_t from, to;
register unsigned nbytes;
{
	register int val;
	jmp_buf jb;int *saved_jb;
#ifdef	UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;
	register int retval = 0;

	if (from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("bcopyin: to address 0x%x failed\n",from));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	val = 0;
	while (nbytes != 0) {
		u_faddr = (int)from;
		u_ste = (int)steaddr(u.u_stbl[snum(from)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)from & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			for (; val < count;) {
				if (from >= (caddr_t)v.v_uend) {
					TRACE(T_subyte,("bcopyin(2): to address 0x%x failed\n",from));
					nofault = saved_jb;
					retval = -1;
					goto out;
				}
				*to = *((caddr_t)(stob(UMOVESEG) + soff(from++)));	/* this can cause a fault */
				val++;
				if (*to++ == 0) {
					nofault = saved_jb;
					retval = val;
					goto out;
				}
			}
		} else {
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
	}
out:
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
	return( retval ? retval : (val == nbytes ? 0 : -1) );
#else	UMVSEG
	if (from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("bcopyin: to address 0x%x failed\n",from));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		from += USEROFF;
		for (val = 0; val < nbytes; ) {
			if ((from-USEROFF) >= (caddr_t)v.v_uend) {
				TRACE(T_subyte,("bcopyin(2): to address 0x%x failed\n",from));
				nofault = saved_jb;
				return(-1);
			}
			*to = *from++;	/* this can cause a fault */
			val++;
			if (*to++ == 0) {
				nofault = saved_jb;
				return(val);
			}
		}
	}
	nofault = saved_jb;
	if (val == nbytes)
		return(0);
	return(-1);
#endif	UMVSEG
}

/*
 * wcopyin - Move words into the system space out of user's address space.
 *	copy only until a null or nbytes
 *	return number of bytes
 */
wcopyin(from, to, nbytes)
register int *from, *to;
register unsigned nbytes;
{
	register int val;
	jmp_buf jb;int *saved_jb;
#ifdef	UMVSEG
	register int off, count;
	register int ofaddr;
	register int oste;
	register char *fromcp;
	register int retval = 0;

	if ((caddr_t)from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("wcopyin(1): to address 0x%x failed\n",from));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	val = 0;
	fromcp = (char *) from;
	while (nbytes != 0) {
		u_faddr = (int)from;
		u_ste = (int)steaddr(u.u_stbl[snum(from)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)from & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			for (; val < count;) {
				if ((fromcp+sizeof(int)) > (caddr_t)v.v_uend) {
					TRACE(T_subyte,("wcopyin(2): to address 0x%x failed\n",from));
					nofault = saved_jb;
					retval = -1;
					goto out;
				}
				ASSERT(count >= sizeof(int));
				from = (int *) (stob(UMOVESEG) + soff(fromcp));
				*to = *from++;	/* this can cause a fault */
				fromcp += sizeof(int);
				val += sizeof(int);
				if (*to++ == 0) {
					nofault = saved_jb;
					retval = val;
					goto out;
				}
			}
		} else {
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
	}
out:
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
	return( retval ? retval : (val == nbytes ? 0 : -1) );
#else	UMVSEG
	if ((caddr_t)from < (caddr_t)v.v_ustart) {
		TRACE(T_subyte,("wcopyin(1): to address 0x%x failed\n",from));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		from = (int *)((int)from + USEROFF);
		for (val = 0; val < nbytes; ) {
			if (((caddr_t)(from+1)-USEROFF) > (caddr_t)v.v_uend) {
				TRACE(T_subyte,("wcopyin(2): to address 0x%x failed\n",from));
				nofault = saved_jb;
				return(-1);
			}
			*to = *from++;	/* this can cause a fault */
			val += sizeof(int);
			if (*to++ == 0) {
				nofault = saved_jb;
				return(val);
			}
		}
	}
	nofault = saved_jb;
	if (val == nbytes)
		return(0);
	return(-1);
#endif	UMVSEG
}

/*
 * copypage - Copy one logical page given the page frame number for
 * the source and destination.
 */
copypage(from, to)
int from, to;
{
	/* convert page frame number to pfdat index */
	from = pftopfi(from);
	to = pftopfi(to);
	wtpte(copypte->pgi, PG_V|PG_RW, from);	/* load kernel vaddrs */
	wtpte((copypte+1)->pgi, PG_V|PG_RW, to);
	invsatb(SYSATB, stob(COPYSEG), 2);		/* clear old vals */
	blt512((caddr_t)stob(COPYSEG)+NBPP, (caddr_t)stob(COPYSEG), ptob(1)>>9);
}

/*
 * clearpage - Clear one physical page given its page frame number
 */
clearpage(pfn)
int pfn;
{
	/* convert page frame number to pfdat index */
	pfn = pftopfi(pfn);
	wtpte(copypte->pgi, PG_V|PG_RW, pfn);	/* load kernel vaddrs */
	invsatb(SYSATB, stob(COPYSEG), 1);		/* clear old vals */
	clear((caddr_t)stob(COPYSEG), ptob(1));
}

/* uclear: clear a section of user space
 */
uclear(uva, nbytes)
register caddr_t uva;
{
	int val = 0;
	jmp_buf jb;int *saved_jb;
#ifdef UMVSEG
	register int inc;
	register caddr_t addr;
	register int off, count;
	register int ofaddr;
	register int oste;

	if (uva < (caddr_t)v.v_ustart ||
	    (uva+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("uclear: to address 0x%x failed\n",uva));
		return(-1);
	}
	ofaddr = u_faddr;
	oste = u_ste;
	while (nbytes > 0) {
		u_faddr = (int)uva;
		u_ste = (int)steaddr(u.u_stbl[snum(uva)]);
		wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
		clratb(SYSATB);
		off = (int)uva & SOFFMASK;
		count = (int)(nbytes < stob(1) ? nbytes : stob(1));
		if ((off + count) > stob(1))
			count = stob(1) - off;
		saved_jb = nofault;
		if (!setjmp(jb)) {
			nofault = jb;
			addr = (caddr_t)(stob(UMOVESEG) + soff(uva));
			for (inc=count; inc > 0; inc--, addr++)
				*addr = (char)0;
		} else {
			val = -1;
			nofault = saved_jb;
			break;
		}
		nofault = saved_jb;
		nbytes -= count;
		uva + = count;
	}
	u_faddr = (int)ofaddr;
	u_ste = oste;
	wtste(kstbl[UMOVESEG], SEG_RW, NPGPT, u_ste);
	clratb(SYSATB);
#else UMVSEG
	if (uva < (caddr_t)v.v_ustart ||
	    (uva+nbytes) > (caddr_t)v.v_uend) {
		TRACE(T_subyte,("uclear: to address 0x%x failed\n",uva));
		return(-1);
	}
	saved_jb = nofault;
	if (!setjmp(jb)) {
		nofault = jb;
		clear(uva+USEROFF,nbytes);
	} else {
		val = -1;
	}
	nofault = saved_jb;
#endif	UMVSEG
	return(val);
}
