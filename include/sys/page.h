/*	@(#)page.h	UniPlus V.2.1.8	*/
/*	Page table entry dependent constants.
 */

#ifndef LOCORE	/* also defined in mch.s */
#define	NBPP		PAGESIZE	/* Number of bytes per page */
#endif LOCORE
#define	BPPSHFT		PAGESHIFT	/* LOG2(NBPP) if exact */
#define NDPP		(1<<DPPSHFT)	/* Number of disk blocks per page */
#define DPPSHFT		(PAGESHIFT-BPDSHFT)/* Shift for disk blocks per page */
#define NPGPT		(1<<NPGPTSHFT)	/* Nbr of pages per page table (seg) */
#define NPGPTSHFT	PTBLINDEX	/* Shift for NPGPT */
#define NPPGPT		(1<<NPPGPTSHFT)	/* Nbr of phys pages per page table */
#define NPPGPTSHFT	PPTBLINDEX	/* Shift for NPPGPT */

#define PG_PROTSHFT	0		/* for compatibility with ccv */

/* This is needed to translate between physical and logical pages. */
#define	PPTOPSHFT	(PPTBLINDEX-PTBLINDEX)

#define PNUMSHFT	PAGESHIFT	/* Shift for page number from address */
#define PNUMMASK	((1<<PTBLINDEX)-1)/* Mask for page number within seg */
#define POFFMASK	(PAGESIZE-1)	/* Mask for offset into page */
#define PPOFFMASK	(PPAGESIZE-1)	/* Mask for offset into phys page */
	/* Mask for page frame number after shift */
#define PGFNMASK	((1<<(PTBLINDEX+STBLINDEX))-1)

#define	NPTPP		(1<<NPTPPSHFT)	/* Nbr of page tables per page */
#define	NPTPPSHFT	(PAGESHIFT-PTSZSHFT)/* Shift for NPTPP */

/* This assumes the pte is 4 bytes long, which is correct for both
 * the MMB and PMMU.  If some other MMU is being used change PTSZSHFT
 * accordingly.
 */
#define PTSIZE		(1<<PTSZSHFT)
#define	PTSZSHFT	(PPTBLINDEX+2)

/*	Get virtual page number (within segment) from system virtual address. */
/*	For the PMMU ppnum() is identical to pnum().
 *	For the MMB ppnum() is the physical page table index (bits 10-17)
 *	and pnum() combines the table A and segment tables indices (bits 13-17).
 */
#define	pnum(X)		(((uint)(X) >> PNUMSHFT) & PNUMMASK)
#define	ppnum(X)	(((uint)(X) >> PPAGESHIFT) & ((1<<PPTBLINDEX)-1))

/*	Page offset.  */

#define	poff(X)		((uint)(X) & POFFMASK)

/*	Round up page table address */

#ifdef MMB
#define ptround(p)	((int *) (((int)(p) + PTSIZE-1) & ~(PTSIZE-1)))
#else MMB
#define PTBOUND	16
#define ptround(p)	((int *) (((int)(p) + PTBOUND-1) & ~(PTBOUND-1)))
#endif MMB
/* For the PMMU/MMB we don't need optround. */
#define optround(p)	ptround(p)

/*	Round down page table address */

#define ptalign(p)	((int *) ((int)(p) & ~(PTSIZE-1)))

/*
 * Page Table
 */

#ifndef LOCORE
typedef union ptbl {
	int page[NPPGPT];
} ptbl_t;

#endif LOCORE
#ifdef ORIG3B20
union ptbl *getptbl();		/* page table allocator */
union ptbl *kdma();		/* kernel dma access routine */
union ptbl *userdma();		/* user dma access routine */
#endif ORIG3B20

/*
 *	Page Macros
 */

/*	Get page number from system virtual address.  */

#define	svtop(X)	(((int)(X) >> PNUMSHFT) & PGFNMASK)

/*	Get system virtual address from page number.  */

#define	ptosv(X)	((int)(X) << PNUMSHFT)

/*	Get page table entry for a system virtual address.  */

#define	svtopte(X)	((pte_t *)segpptbl(kstbl[btots(((long)(X)))])+pnum(((long)(X))))

/*	Get page table entry for a system virtual page.  */

#define	ptopte(X)	((pte_t *)segpptbl(kstbl[ptots((long)(X))])+(((long)(X))&PNUMMASK))

/*	Get the system virtual address corresponding to a physical
 *	address.
 */

#ifdef MMB

/*	Get the system virtual address corresponding to a physical address */

#define svirtophys(X)	(ptob(svtopfi(X)) + poff(X))

/*	Get page frame number (for pte) given the pfdat index */

#define pfitopf(X)	((X) << PPTOPSHFT)

/*	Get pfdat index given the page frame number from pte */

#define pftopfi(X)	((X) >> PPTOPSHFT)

/*	Get physical address given the page frame number */

#define pfitop(X)	(X)

/* 	Get pfdat index given system virtual address */

#define svtopfi(X)	(pftopfi((svtopte(X)->pgm[0].pg_pfn)))

/*	Get uptvaddrs index given physical address of user page table */

#define phystoupti(X)	(btotp((X)-(int)uptbase))

#else MMB

/*	Get the system virtual address corresponding to a physical address */

#define svirtophys(X)	(ptob(svtopte(X)->pgm[0].pg_pfn) + poff(X))

/*	Get page frame number (for pte) given the pfdat index */

#define pfitopf(X)	(X)

/*	Get pfdat index given the page frame number from pte */

#define pftopfi(X)	(X)

/*	Get physical address given the page frame number */

#define pfitop(X)	(X)

/* 	Get pfdat index given system virtual address */

#define svtopfi(X)	(pftopfi((svtopte(X)->pgm[0].pg_pfn)))

/*	Get uptvaddrs index given physical address of user page table */

#define phystoupti(X)	(btotp((X)-(int)uptbase))

#endif MMB

#ifndef LOCORE	/* for mch.s */
extern union pte	*kptbl;
extern caddr_t		uptbase;
extern caddr_t		*uptvaddrs;
extern int		nptalloced;
extern int		nptfree;

#ifdef MMB
/*
 * - MMB "page table entry" is 8 longs (to map 8 1K physical pages)
 * - pg_ndref software bit is in a different position than on PMMU
 *
 *   --------------------------------------------------------------------
 *   |               page address               |sl|cw|g|ci|nr|m|r|wp|dt|
 *   --------------------------------------------------------------------
 *                        22                      1  1 1  1  1 1 1  1  2
 *                                               SW SW      SW
 */
typedef union pte {
	struct {
#ifdef lint
		unsigned int
#else
		unsigned long
#endif lint
			pg_pfn	: 22,	/* Physical page frame number.	*/
			pg_lock	: 1,	/* Lock in core.	(SW)	*/
			pg_cw	: 1,	/* Copy on write.	(SW)	*/
			pg_gate	: 1,	/* Page may contain gates.	*/
			pg_ci	: 1,	/* Cache inhibit.		*/
			pg_tcl	: 1,	/* Translation cache lock.	*/
#ifdef PMMUMMB
#define	pg_ndref pg_ci			/* Needs reference.	(SW)	*/
#else PMMUMMB
#define	pg_ndref pg_tcl			/* Needs reference.	(SW)	*/
#endif PMMUMMB
			pg_mod	: 1,	/* Page has been modified flag.	*/
			pg_ref	: 1,	/* Page has been referenced.	*/
			pg_prot	: 1,	/* Write protect.		*/
			pg_v	: 2;	/* Descriptor type (valid bit).	*/
	} pgm[1 << PPTOPSHFT];
	/* 
	 * For fast mask operations when needed
	 */
	struct {
		long	pg_pte;
	} pgi[1 << PPTOPSHFT];
} pte_t;
/*
 *	In some cases (such as mapping I/O) we need to access the 1K page.
 */
typedef union ppte {
	struct {
#ifdef lint
		unsigned int
#else
		unsigned long
#endif lint
			pg_pfn	: 22,	/* Physical page frame number.	*/
			pg_lock	: 1,	/* Lock in core.	(SW)	*/
			pg_cw	: 1,	/* Copy on write.	(SW)	*/
			pg_gate	: 1,	/* Page may contain gates.	*/
			pg_ci	: 1,	/* Cache inhibit.		*/
#ifdef PMMUMMB
#define	pg_ndref pg_ci			/* Needs reference.	(SW)	*/
#else PMMUMMB
#define	pg_ndref pg_tcl			/* Needs reference.	(SW)	*/
#endif PMMUMMB
			pg_tcl	: 1,	/* Translation cache lock.	*/
			pg_mod	: 1,	/* Page has been modified flag.	*/
			pg_ref	: 1,	/* Page has been referenced.	*/
			pg_prot	: 1,	/* Write protect.		*/
			pg_v	: 2;	/* Descriptor type (valid bit).	*/
	} pgm;
	struct {
		long	pg_pte;
	} pgi;
} ppte_t;
#else MMB		/* PMMU */
/*
 *   --------------------------------------------------------------------
 *   |               page address             |nr|sl|cw|g|ci|l|m|r|wp|dt|
 *   --------------------------------------------------------------------
 *                        21                    1  1  1 1  1 1 1 1  1  2
 *                                             SW SW SW
 */
typedef union pte {
	struct {
		unsigned long
			pg_pfn	: 19,	/* Physical page frame number.	*/
			pg_xxx1	: 2,	/* dummy entry			*/
			pg_ndref: 1,	/* Needs reference.	(SW)	*/
			pg_lock	: 1,	/* Lock in core.	(SW)	*/
			pg_cw	: 1,	/* Copy on write.	(SW)	*/
			pg_gate	: 1,	/* Page may contain gates.	*/
			pg_ci	: 1,	/* Cache inhibit.		*/
			pg_tcl	: 1,	/* Translation cache lock.	*/
			pg_mod	: 1,	/* Page has been modified flag.	*/
			pg_ref	: 1,	/* Page has been referenced.	*/
			pg_prot	: 1,	/* Write protect.		*/
			pg_v	: 2;	/* Descriptor type (valid bit).	*/
	} pgm[1];
	/* 
	 * For fast mask operations when needed
	 */
	struct {
		long	pg_pte;
	} pgi[1];
} pte_t;
typedef pte_t	ppte_t;
#endif MMB
#endif LOCORE	/* for mch.s */

/* Page Table Entry
 */

#ifdef MMB
/* KAREN: PG_ADDR is for a 1K page, not 8K.  Watch how it's used. */
#define PG_ADDR		0xFFFFFC00	/* physical page address */
# ifdef PMMUMMB
#define PG_NDREF	0x00000040	/* need reference bit (software) */
# else PMMUMMB
#define PG_NDREF	0x00000020	/* need reference bit (software) */
# endif PMMUMMB
#define	PGADDRSHFT	10		/* shift for pfn to position in pte */
#else MMB	/* PMMU */
#define PG_ADDR		0xFFFFE000	/* physical page address */
#define PG_NDREF	0x00000400	/* need reference bit (software) */
#define	PGADDRSHFT	13		/* shift for pfn to position in pte */
#endif MMB
#define PG_LOCK		0x00000200	/* page lock bit (software) */
#define PG_COPYW	0x00000100	/* copy on write bit (software) */
#define PG_G		0x00000080	/* gate bit (page may contain gates) */
#define PG_CI		0x00000040	/* cache inhibit bit */
#define PG_L		0x00000020	/* lock in translation cache */
#define PG_M		0x00000010	/* modified bit (page written to) */
#define PG_REF		0x00000008	/* reference bit (page accessed)*/
#define PG_PROT		0x00000004	/* CPU protection field */
#define PG_PERM		0x00000004	/* write protect bit (supv or user) */
#define PG_VFIELD	0x00000003	/* page descriptor field */
/* use this bit as the "valid" bit - desc. field is 2 bits but high bit is
 * zero in both encodings that are used (0x0 for invalid, 0x1 for valid) */
#define	PG_V		0x00000001	/* page valid bit */

/* There is only one protection bit in a short (32-bit) page descriptor.
 * PG_NOACC actually allows reading, just not writing.  There is no
 * distinction between supervisor and user accesses at this level. */
#define	PG_NOACC	0x00000004	/* "no" permission */
#define PG_ALL		0x00000000	/* all permissions */
#define PG_RO		PG_NOACC
#define PG_RW		PG_ALL
#define PG_WRT		PG_ALL

/*
 * Various macros for code compatibility
 */
#define npteget(pt)	(pt)
#define pteget(pt)
#define pteset(pt)

/* Form page table entry from modes and page frame number.
 * Note that the page number bits are in the same position within
 * both the virtual address and the pte for the PMMU.  This will
 * not be the case for all MMUs.
 */
#ifdef MMB
/* On the MMB we use 8 1K physical pages to make up 1 8K page that
 * a single pte_t represents.  These macros handle this distinction
 * between the MMB and PMMU.
 */
#define	mkppte(mode,pfn)	(mode | ((pfn) << PPAGESHIFT))
#define	wtpte(pgi,mode,pfn)				\
	{	register ppfn = (pfn)<<PPTOPSHFT;	\
		(pgi)[0].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[1].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[2].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[3].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[4].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[5].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[6].pg_pte = mkppte(mode,ppfn++);	\
		(pgi)[7].pg_pte = mkppte(mode,ppfn);	\
	}
/* For mapping I/O it is necessary to write a physical pte
 * (using ppnum(vaddr) to calculate the physical page number)
 */
#define	wtppte(pgi,mode,pfn)				\
		(pgi).pg_pte = mkppte(mode,pfn)
#else MMB	/* PMMU */
#define	mkpte(mode,pfn)		(mode | ((pfn) << PNUMSHFT))
#define	wtpte(pgi,mode,pfn)				\
		(pgi)[0].pg_pte = mkpte(mode,pfn)
#define	wtppte(pgi,mode,pfn)	wtpte(pgi,mode,pfn)
#endif MMB

#ifdef MMB
/* On the MMB we use 8 1K physical pages to make up 1 8K page that
 * a single pte_t represents.  These macros handle this distinction
 * between the MMB and PMMU.
 */
#define pg_clr(P,mask)					\
	{	(P)->pgi[0].pg_pte &= ~(mask);		\
		(P)->pgi[1].pg_pte &= ~(mask);		\
		(P)->pgi[2].pg_pte &= ~(mask);		\
		(P)->pgi[3].pg_pte &= ~(mask);		\
		(P)->pgi[4].pg_pte &= ~(mask);		\
		(P)->pgi[5].pg_pte &= ~(mask);		\
		(P)->pgi[6].pg_pte &= ~(mask);		\
		(P)->pgi[7].pg_pte &= ~(mask);		\
	}
#define pg_set(P,mask)					\
	{	(P)->pgi[0].pg_pte |= (mask);		\
		(P)->pgi[1].pg_pte |= (mask);		\
		(P)->pgi[2].pg_pte |= (mask);		\
		(P)->pgi[3].pg_pte |= (mask);		\
		(P)->pgi[4].pg_pte |= (mask);		\
		(P)->pgi[5].pg_pte |= (mask);		\
		(P)->pgi[6].pg_pte |= (mask);		\
		(P)->pgi[7].pg_pte |= (mask);		\
	}
#else MMB		/* PMMU */
#define pg_clr(P,mask)	(P)->pgi[0].pg_pte &= ~(mask)
#define pg_set(P,mask)	(P)->pgi[0].pg_pte |= (mask)
#endif MMB

/*	Set a new value into the protection field.			*/
/* BOBJ what if page already has protection, should clear prot before or'ing? */
#define pg_clrprot(P)	pg_clr(P, PG_PROT)
#define pg_setprot(P,V)	pg_set(P,V)

#define pg_setvalid(P)	pg_set(P, PG_V)
#define pg_clrvalid(P)	pg_clr(P, PG_V)

#define pg_setndref(P)	pg_set(P, PG_NDREF)
#define pg_clrndref(P)	pg_clr(P, PG_NDREF)

#define pg_setlock(P)	pg_set(P, PG_LOCK)
#define pg_clrlock(P)	pg_clr(P, PG_LOCK)

#define pg_setmod(P)	pg_set(P, PG_M)
#define pg_clrmod(P)	pg_clr(P, PG_M)

#define pg_setcw(P)	pg_set(P, PG_COPYW)
#define pg_clrcw(P)	pg_clr(P, PG_COPYW)

#define pg_setref(P)	pg_set(P, PG_REF)
#define pg_clrref(P)	pg_clr(P, PG_REF)

#ifdef MMB
/* On the MMB we use 8 1K physical pages to make up 1 8K page that
 * a single pte_t represents.  This macro handles this distinction
 * between the MMB and PMMU.
 */
#define pg_chkmod(P)					\
	(	(P)->pgm[0].pg_mod ? 1 : (		\
		(P)->pgm[1].pg_mod ? 1 : (		\
		(P)->pgm[2].pg_mod ? 1 : (		\
		(P)->pgm[3].pg_mod ? 1 : (		\
		(P)->pgm[4].pg_mod ? 1 : (		\
		(P)->pgm[5].pg_mod ? 1 : (		\
		(P)->pgm[6].pg_mod ? 1 : (		\
		(P)->pgm[7].pg_mod ? 1 : 0)))))))	\
	)
#define pg_chkref(P)					\
	(	(P)->pgm[0].pg_ref ? 1 : (		\
		(P)->pgm[1].pg_ref ? 1 : (		\
		(P)->pgm[2].pg_ref ? 1 : (		\
		(P)->pgm[3].pg_ref ? 1 : (		\
		(P)->pgm[4].pg_ref ? 1 : (		\
		(P)->pgm[5].pg_ref ? 1 : (		\
		(P)->pgm[6].pg_ref ? 1 : (		\
		(P)->pgm[7].pg_ref ? 1 : 0)))))))	\
	)
#define pg_chkndref(P)	 ((P)->pgm[0].pg_ndref)

#define pg_chktune(P, V) 				\
	(	(P)->pgi[0].pg_pte&(V) ? 1 : (		\
		(P)->pgi[1].pg_pte&(V) ? 1 : (		\
		(P)->pgi[2].pg_pte&(V) ? 1 : (		\
		(P)->pgi[3].pg_pte&(V) ? 1 : (		\
		(P)->pgi[4].pg_pte&(V) ? 1 : (		\
		(P)->pgi[5].pg_pte&(V) ? 1 : (		\
		(P)->pgi[6].pg_pte&(V) ? 1 : (		\
		(P)->pgi[7].pg_pte&(V) ? 1 : 0)))))))	\
	)
#define pg_zero(P)				\
	{	(P)->pgi[0].pg_pte = 0;		\
		(P)->pgi[1].pg_pte = 0;		\
		(P)->pgi[2].pg_pte = 0;		\
		(P)->pgi[3].pg_pte = 0;		\
		(P)->pgi[4].pg_pte = 0;		\
		(P)->pgi[5].pg_pte = 0;		\
		(P)->pgi[6].pg_pte = 0;		\
		(P)->pgi[7].pg_pte = 0;		\
	}
#define ppg_zero(P)	(P)->pgi.pg_pte = 0
#else MMB		/* PMMU */
#define pg_chkmod(P)	((P)->pgm[0].pg_mod) 
#define pg_chkref(P)	((P)->pgm[0].pg_ref) 
#define pg_chkndref(P)	((P)->pgm[0].pg_ndref)
#define pg_chktune(P,V)	((P)->pgi[0].pg_pte&(V))
#define pg_zero(P)	(P)->pgi[0].pg_pte = 0
#define ppg_zero(P)	pg_zero(P)
#endif MMB

/* Find disk block descriptor table corresponding to page table. */
/* #define dbdget(pt)	((dbd_t *)(((uint *)pt) + NPPGPT)) */
#define dbdget(pt)	(((dbd_t *)pt) + NPGPT)

#ifndef LOCORE
extern int *nendp;
#endif LOCORE

/*	The following macro takes a page table entry and modifies
**	it as required in order to be able to do dma.
*/

#ifndef ORIG3B20	/* no dma */
#define	pg_dmaok(P)	((P) & (PG_ADDR | PG_ALL))
#ifdef MMB
#define	wtdmaok(dpgi,spgi)			\
	{	(dpgi)[0].pg_pte = pg_dmaok(spgi[0].pg_pte);	\
		(dpgi)[1].pg_pte = pg_dmaok(spgi[1].pg_pte);	\
		(dpgi)[2].pg_pte = pg_dmaok(spgi[2].pg_pte);	\
		(dpgi)[3].pg_pte = pg_dmaok(spgi[3].pg_pte);	\
		(dpgi)[4].pg_pte = pg_dmaok(spgi[4].pg_pte);	\
		(dpgi)[5].pg_pte = pg_dmaok(spgi[5].pg_pte);	\
		(dpgi)[6].pg_pte = pg_dmaok(spgi[6].pg_pte);	\
		(dpgi)[7].pg_pte = pg_dmaok(spgi[7].pg_pte);	\
	}
#else MMB	/* PMMU */
#define	wtdmaok(dpgi,spgi)			\
		(dpgi)[0].pg_pte = pg_dmaok(spgi.pg_pte);
#endif MMB
#else ORIG3B20
#define	pg_dmaok(P)	(((P) & (PG_ADDR | PG_ALL)) | PG_DMAR | PG_DMAW)
#endif ORIG3B20
