/*	@(#)page.c	UniPlus VVV.2.1.10	*/

#ifdef HOWFAR
extern int	T_page;
extern int T_swapalloc;
#endif HOWFAR
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
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/vnode.h"
#include "sys/var.h"
#include "sys/vfs.h"
#include "svfs/mount.h"
#include "sys/buf.h"
#include "sys/map.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/swap.h"
#include "sys/tuneable.h"
#include "sys/debug.h"
#include "svfs/inode.h"
#include "netinet/in.h"
#endif lint

char	mem_lock;
int	firstfree, maxfree, freemem;
extern int	invsatb();
extern int	clratb();


/*	Lock memory allocation.
 */

#ifndef memlock

memlock()
{
	register int s;

	s = splhi();
	u.u_procp->p_flag |= SLOCK; /* SLOCK NOT USED ON V3 */
	while (mem_lock) {
		mem_lock |= 2;
{/*debug*/
#ifdef DEBUG
printf("ASA:mem_lock=BUSY "); debug();
#endif
		sleep(&mem_lock, PMEM);
	}
/*debug*/}
	mem_lock = 1;
	splx(s);
}

#endif



/*	Unlock memory allocation.
 */

#ifndef memunlock

memunlock()
{
	ASSERT(mem_lock);
	if (mem_lock&2)
		wakeup(&mem_lock);
	mem_lock = 0;
	u.u_procp->p_flag &= ~SLOCK; /* SLOCK NOT USED ON V3 */
}

#endif

/*
 * Allocate pages and fill in page table
 *	rp		-> region pages are being added to.
 *	base		-> address of page table
 *	size		-> # of pages needed
 *	validate	-> Mark pages valid if set.
 * returns:
 *	0	Memory allocated immediately.
 *	1	Had to unlock region and go to sleep before
 *		memory was obtained.  After awakening, the
 *		page was valid or pfree'd so no page was
 *		allocated.
 *
 * Called with mem_lock set and returns the same way.
 */

ptmemall(rp, base, size, validate)
reg_t		*rp;
register pte_t	*base;
register int	validate;
{
	register struct pfdat	*pfd;
	register int		i;

	/*	Check for illegal size.
	 */

	ASSERT(size > 0);
	ASSERT(mem_lock);
TRACE(T_page, ("ptmemall(0x%x, 0x%x, 0x%x, 0x%x, 0x%x)\n", rp, base, size, validate,caller()));
	ASSERT(rp->r_lock);

	if(memreserve(rp, size)  &&  base->pgm[0].pg_v)
	{
		freemem += size;
		return(1);
	}
	ASSERT(mem_lock);
	ASSERT(rp->r_lock);
	/*
	 * Take pages from head of queue
	 */

	pfd = phead.pf_next;
	i = 0;
	while(i < size){
		register pfd_t	*pfd_next;

		ASSERT (pfd != &phead);
		ASSERT(pfd->pf_flags&P_QUEUE);
		ASSERT(pfd->pf_use == 0);


		/* Delink page from free queue and set up pfd
		 */

		pfd_next = pfd->pf_next;
		pfd->pf_prev->pf_next = pfd_next;
		pfd_next->pf_prev = pfd->pf_prev;
		pfd->pf_next = NULL;
		pfd->pf_prev = NULL;
		if (pfd->pf_flags&P_HASH)
			(void)premove(pfd);
		pfd->pf_blkno = BLKNULL;
		pfd->pf_use = 1;
		pfd->pf_flags = 0;
		pfd->pf_rawcnt = 0;
		rp->r_nvalid++;

		/*
		 * Insert in page table
		 */

#ifdef MMB
		{ register n, ppfn = pfitopf(pfd - pfdat);
		for (n = 0; n < (1 << PPTOPSHFT); n++)
			base->pgm[n].pg_pfn = ppfn++;
		}
#else MMB
		base->pgm[0].pg_pfn = pfitopf(pfd - pfdat);
#endif MMB
		pg_clrmod(base);

		pg_clrprot(base);
		if(base->pgm[0].pg_cw) {
			pg_setprot(base, PG_RO);
		} else {
			pg_setprot(base, PG_RW);
		}

		if(validate)
			pg_setvalid(base);
		
		i++;
		base++;
		pfd = pfd_next;
	}
	return(0);
}


/*
 * Shred page table and update accounting for swapped
 * and resident pages.
 *	rp	-> ptr to the region structure.
 *	pt	-> ptr to the first pte to free.
 *	dbd	-> ptr to disk block descriptor.
 *	size	-> nbr of pages to free.
 *
 * Called with mem_lock set and returns the same way.
 */

pfree(rp, pt, dbd, size)
reg_t		*rp;
register pte_t	*pt;
register dbd_t	*dbd;
int		size;
{
	register struct pfdat	*pfd;
	register int		k;

	ASSERT(mem_lock);

	/* 
	 * Zap page table entries
	 */

	for (k = 0; k < size; k++, pt++) {
		if (pt->pgm[0].pg_v) {
			pfd = &pfdat[pftopfi(pt->pgm[0].pg_pfn)];

			/* Free pages that aren't being used
			 * by anyone else
			 */
			if(--pfd->pf_use == 0){

				/* Pages that are associated with disk
				 * go to end of queue in hopes that they
				 * will be reused.  All others go to
				 * head of queue so they will be reused
				 * quickly.
				 */

				if(dbd == NULL || dbd->dbd_type == DBD_NONE){
					/*
					 * put at head 
					 */
					pfd->pf_next = phead.pf_next;
					pfd->pf_prev = &phead;
					phead.pf_next = pfd;
					pfd->pf_next->pf_prev = pfd;
				} else {
					/*
					 * put at tail 
					 */
					pfd->pf_prev = phead.pf_prev;
					pfd->pf_next = &phead;
					phead.pf_prev = pfd;
					pfd->pf_prev->pf_next = pfd;
				}
				pfd->pf_flags |= P_QUEUE;
				freemem++;
			}

			rp->r_nvalid--;
		}
		TRACE(T_swapalloc,
			("pfree: size=0x%x pt=0x%x dbd=0x%x dbd_type=0x%x\n", 
			size, pt, dbd, dbd?dbd->dbd_type:0));
		if(dbd  &&  dbd->dbd_type == DBD_SWAP){
			if (swfree1(dbd) == 0)
				(void) pbremove(rp, dbd);
		}

		/*
		 * Change to zero pte's.
		 */

		pg_zero(pt);
		if(dbd)
			dbd++->dbd_type = DBD_NONE;
	}


}




/*
 * Device number
 *	vp	-> vnode pointer
 * returns:
 *	dev	-> device number
 */
u_int
effdev(vp)
register struct vnode *vp;
{
	register int mode;

	if (vp->v_op == &svfs_vnodeops) {
		/* local file system */
		mode = VTOI(vp)->i_mode & IFMT;
		if (mode == IFREG || mode == IFDIR) 
			return(VTOI(vp)->i_dev);
		return(VTOI(vp)->i_rdev);
	}
	return((u_int)vp);
}

/*
 * Find page by looking on hash chain
 *	dbd	-> Ptr to disk block descriptor being sought.
 * returns:
 *	0	-> can't find it
 *	pfd	-> ptr to pfdat entry
 */

struct pfdat *
pagefind(rp, dbd)
register reg_t	*rp;
register dbd_t	*dbd;
{
	register u_int		dev;
	register daddr_t	blkno;
	register pfd_t		*pfd;

	/*	Hash on block and look for match.
	 */

	ASSERT(mem_lock);
	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];
	}
	pfd = phash[(blkno>>DPPSHFT)&phashmask].pf_hchain;

	for( ; pfd != NULL ; pfd = pfd->pf_hchain) {
		if((pfd->pf_blkno == blkno) && (pfd->pf_dev == dev)){
			if (pfd->pf_flags & P_BAD)
				continue;
			return(pfd);
		}
	}
	return(0);
}

/*
 * Insert page on hash chain
 *	dbd	-> ptr to disk block descriptor.
 *	pfd	-> ptr to pfdat entry.
 * returns:
 *	none
 */

pinsert(rp, dbd, pfd)
register reg_t	*rp;
register dbd_t	*dbd;
register pfd_t	*pfd;
{
	register u_int	dev;
	register int	blkno;

	/* Check page range, see if already on chain
	 */

	ASSERT(mem_lock);
	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];

		/*
		 *	If blkno is zero, then we can't hash the page.
		 *	This happens for the last data page of a stripped
		 *	a.out that is an odd number of blocks long.
		 */

		if(blkno == 0)
			return;
	}

	ASSERT(pfd->pf_hchain == NULL);
	ASSERT(pfd->pf_use > 0);

	/*
	 * insert newcomers at tail of bucket
	 */

	{
		register struct pfdat *pfd1, *p;

		for(p = &phash[(blkno>>DPPSHFT)&phashmask] ; 
				pfd1 = p->pf_hchain ; p = pfd1) {
			if((pfd1->pf_blkno == blkno) &&
			   (pfd1->pf_dev == dev)){
#if OSDEBUG == YES
				printf("swapdev %x %x %x\n",swapdev,
				     pfd1->pf_dev,dev);
				printf("blkno %x %x\n",blkno,pfd1->pf_blkno);
				printf("swpi %x %x\n", pfd1->pf_swpi,
					pfd->pf_swpi);
				printf("pfd %x %x\n",pfd,pfd1);
				printf("use %x %x\n",pfd->pf_use,pfd1->pf_use);
				printf("flags %x %x\n",pfd->pf_flags,
				     pfd1->pf_flags);
#endif
				panic("pinsert dup");
			}
		}
		p->pf_hchain = pfd;
		pfd->pf_hchain = pfd1;
	}

	/*	Set up the pfdat.  Note that only swap pages are
	 *	put on the hash list for now.
	 */

	pfd->pf_dev = dev;
	if(dbd->dbd_type == DBD_SWAP){
		pfd->pf_swpi = dbd->dbd_swpi;
		pfd->pf_flags |= P_SWAP;
	} else {
		pfd->pf_flags &= ~P_SWAP;
	}
	pfd->pf_blkno = blkno;
	pfd->pf_flags |= P_HASH;
}


/*
 * remove page from hash chain
 *	pfd	-> page frame pointer
 * returns:
 *	0	Entry not found.
 *	1	Entry found and removed.
 */
premove(pfd)
register struct pfdat *pfd;
{
	register struct pfdat *pfd1, *p;
	int	rval;

	ASSERT(mem_lock);

	rval = 0;
	for(p = &phash[(pfd->pf_blkno>>DPPSHFT)&phashmask] ; 
			pfd1 = p->pf_hchain ; p = pfd1) {
		if (pfd1 == pfd) {
			p->pf_hchain = pfd->pf_hchain;
			rval = 1;
			break;
		}
	}
	/*
	 * Disassociate page from disk and
	 * remove from hash table
	 */
	pfd->pf_blkno = BLKNULL;
	pfd->pf_hchain = NULL;
	pfd->pf_flags &= ~P_HASH;
	pfd->pf_dev = 0;
	return(rval);
}

/*
 * Allocate system virtual address space but
 * don't allocate any physical pages (this is
 * done later with sptfill().  Expects a size
 * in pages, and returns a virtual address.
 */
caddr_t
sptreserve(size)
register int size;
{
	register sp;

TRACE(T_page, ("malloc(0x%x, 0x%x)\n", sptmap, size));
	if ((sp = malloc(sptmap, size))  ==  0) {
		printf("No kernel virtual space\n");
#if OSDEBUG == YES
		printf("size=%d, mode=%d, base=%d\n",size, mode, base);
#endif
		return(NULL);
	}
TRACE(T_page, ("malloc pages at 0x%x\n", ptopte(sp)));
	return((caddr_t)ptosv(sp));
}

/*
 * Allocate pages for a reserved section of
 * of system virtual address space.
 *	return values:
 *		 0	normal return
 *		-1	nosleep is set and we would have 
 *			  had to sleep (no memory was allocated)
 * NOTE: 
 * 	This routine is intended to allow filling in a virtual 
 *	array at interrupt time (watch out - memory allocation 
 *	at interrupt time).  If called from an interrupt routine
 *	sptfill must be called with nosleep set, and the calling 
 *	routine must check for possible failure.  If nosleep is 
 *	not set, sptfill will not fail and need not be checked.
 */
sptfill(svaddr, size, mode, pbase, nosleep)
caddr_t svaddr;
register int size, mode;
int pbase, nosleep;
{
	register int sp, i;
	register pte_t *p;
	extern int kmemory;

	/*
	 * If we need to allocate memory and we're not allowed
	 * to sleep, make sure we won't run into any locks.
	 */
	if ((pbase == 0) && nosleep && 
	    (freemem < size || isreglock(&sysreg)))
		return(-1);

	/*
	 * Allocate and fill in pages
	 */
	sp = svtop(svaddr);
	if (pbase  ==  0){
		reglock(&sysreg);
		memlock();
		if(ptmemall(&sysreg, ptopte(sp), size, 1))
			panic("sptalloc - ptmemall failed");
		kmemory += size;
		memunlock();
		regrele(&sysreg);
	}

	/*
	 * Setup page table entries
	 */
	for (i = 0; i < size; i++) {
		if (pbase > 0) {
			p = (pte_t *)ptopte(sp + i);		
			pg_zero(p);
			wtpte(p->pgi, mode, pbase++);
		} else {
#ifdef MMB
			register j;
			
			p = (pte_t *)ptopte(sp + i);		
			for (j = 0; j < (1 << PPTOPSHFT); j++)
				p->pgi[j].pg_pte |= mode;
#else MMB
				((pte_t *)ptopte(sp+i))->pgi[0].pg_pte |= mode;
#endif MMB
		}
		invsatb(SYSATB, (caddr_t)(ptosv(sp + i)), 1);
	}
	return(0);
}

/*
 * Allocate system virtual address space and
 * allocate or link  pages.
 */
sptalloc(size, mode, base)
register int size, mode, base;
{
	register caddr_t svaddr;
	extern int kmemory;
	extern int kt_ppmem;

TRACE(T_page, ("sptalloc(0x%x, 0x%x, 0x%x)\n", size, mode, base));

 	if (base == 0 && kt_ppmem && kmemory > tune.t_ppmem) {
		if (!(suser() && kmemory <= kt_ppmem)) {
			printf("No kernel space\n");
			return(NULL);
		}
	}

	/*
	 * Allocate system virtual address space
	 */
	if ((svaddr = sptreserve(size)) == NULL)
		return(NULL);
		
	/*
	 * Allocate and fill in pages
	 */
	(void) sptfill(svaddr, size, mode, base, 0);

	return((int)svaddr);
}

sptfree(vaddr, size, flag)
register int size;
{
	register i, sp;

	sp = svtop(vaddr);
TRACE(T_page,("sptfree:vaddr=%x sp=%x\n",vaddr,sp));
	if (flag){
		reglock(&sysreg);
		memlock();
		pfree(&sysreg, ptopte(sp), (dbd_t *)NULL, size);
		kmemory -= size;
		memunlock();
		regrele(&sysreg);
	}
	for (i = 0; i < size; i++) {
		pg_zero(ptopte(sp+i));
	}
	mfree(sptmap, size, sp);
}

/*
 * Initialize memory map
 *	first	-> first free page #
 *	last	-> last free page #
 * returns:
 *	none
 */
meminit(first, last)
register int first;
{
	register struct pfdat *pfd;
	register int i;

	firstfree = first;
	maxfree = last;
	freemem = (last - first);
	maxmem = freemem;
	/*
	 * Setup queue of pfdat structures.
	 * One for each page of available memory.
	 */
	pfd = &pfdat[first];
	phead.pf_next = &phead;
	phead.pf_prev = &phead;
	/*
	 * Add pages to queue, high memory at end of queue
	 * Pages added to queue FIFO
	 */
	for (i = freemem; --i >= 0; pfd++) {
		pfd->pf_next = &phead;
		pfd->pf_prev = phead.pf_prev;
		phead.pf_prev->pf_next = pfd;
		phead.pf_prev = pfd;
		pfd->pf_flags = P_QUEUE;
	}
}


#ifdef NOTDEF
/*
 * flush all pages associated with a mount device
 *	mp	-> mount table entry
 * returns:
 *	none
 */
punmount(mp)
register struct mount *mp;
{
	register int i;
	register struct pfdat *pfd;

	bflush(mp->m_dev);
	memlock();
	for (i = firstfree,pfd = pfdat + i; i < maxfree; i++, pfd++) {
		if (mp->m_dev == pfd->pf_dev)
			if ((pfd->pf_flags & (P_HASH | P_SWAP))  == P_HASH)
				(void)premove(pfd);
	}
	memunlock();
}
#endif NOTDEF

/*
 * Find page by looking on hash chain
 *	dbd	Ptr to disk block descriptor for block to remove.
 * returns:
 *	0	-> can't find it
 *	i	-> page frame # (index into pfdat)
 */

pbremove(rp, dbd)
reg_t	*rp;
dbd_t	*dbd;
{
	register struct pfdat	*pfd;
	register struct pfdat	*p;
	register int		blkno;
	register u_int		dev;

	/*
	 * Hash on block and look for match
	 */

	ASSERT(mem_lock);

	if(dbd->dbd_type == DBD_SWAP){
		dev = swaptab[dbd->dbd_swpi].st_dev;
		blkno = dbd->dbd_blkno;
	} else {
		register struct vnode	*vp;

		/*	For pages on a file (rather than swap),
		 *	we use the first of the 2 or 4 block numbers
		 *	as the value to hash.
		 */

		vp = rp->r_vptr;
		ASSERT(vp != NULL);
		ASSERT(vp->v_map != NULL);
		dev = effdev(vp);

		/*	The following kludge is because of the
		 *	overlapping text and data block in a 413
		 *	object file.  We hash shared pages on the
		 *	first of the 2 or 4 blocks which make up
		 *	the page and private pages on the second
		 *	block.  This means that the block which
		 *	has the end of the text and the beginning
		 *	of the data will be in the hash twice,
		 *	once as text and once as data.  This is
		 *	necessary since the two cannot be shared.
		 */

		if(rp->r_type == RT_PRIVATE)
			blkno = vp->v_map[dbd->dbd_blkno + 1];
		else
			blkno = vp->v_map[dbd->dbd_blkno];
	}

	for (p = &phash[(blkno>>DPPSHFT)&phashmask] ; 
			pfd = p->pf_hchain; p = pfd) {
		if ((pfd->pf_blkno == blkno) && (pfd->pf_dev== dev)) {
			p->pf_hchain = pfd->pf_hchain;

			pfd->pf_blkno = BLKNULL;
			pfd->pf_hchain = NULL;
			pfd->pf_flags &= ~P_HASH;
			pfd->pf_dev = 0;
			return(1);
		}
	}

	return(0);

}


/*
 * Reserve size memory pages.  Returns with freemem
 * decremented by size.  Return values:
 *	0 - Memory available immediately
 *	1 - Had to sleep to get memory
 */
memreserve(rp, size)
register reg_t *rp;
{
	register struct proc *p;

	ASSERT(rp->r_lock);
	ASSERT(mem_lock);
TRACE(T_page, ("memreserve: rp=0x%x freemem=0x%x size=0x%x\n",rp,freemem,size));
	if (freemem >= size) {
		freemem -= size;
		return(0);
	}
	p = u.u_procp;
	while (freemem < size) {
		regrele(rp);
		memunlock();
		p->p_stat = SXBRK;
		(void)wakeup(&runout);
TRACE(T_page, ("memreserve: calling swtch()\n"));
		swtch();
		reglock(rp);
		memlock();
	}
	freemem -= size;
	return(1);
}
/* <@(#)page.c	1.5> */
