/*	@(#)mmu.h	UniPlus V.2.1.8	*/

#if !(defined(MMB) || defined(PMMU))
#define MMB	/* for now, this is the default choice - if you have a 
		   different MMU, then take a local copy of this file */
#endif MMB

/*
 * Motorola PMMU
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 *
 * Definitions for memory management.
 *
 * A 2-level scheme is used, dubbed "segment" and "page" levels.
 * The virtual address is used as:
 *
 * ---------------------------------------------------------------------------
 * | unused | segment table index |  page table index  |  offset into page   |
 * ---------------------------------------------------------------------------
 * <-- ? --><----- STBLINDEX ----><---- PTBLINDEX ----><----- PAGESHIFT ----->
 *     where (STBLINDEX + PTBLINDEX + PAGESHIFT) <= 32
 *
 * Change only STBLINDEX, PTBLINDEX and PAGESHIFT.
 */

/* Don't change any of these values if you are not using a PMMU. */
#define	PAGESHIFT	PS8K
#ifdef MMB
#define	MMBTBLAINDEX	2		/* third level for MMB only */
#define	STBLINDEX	9		/* MMBTBLAINDEX + PSTBLINDEX */
#define	PTBLINDEX	5
#define	PPAGESHIFT	10		/* 1K physical page size */
#define	PSTBLINDEX	7		/* 0 - 127 */
#define	PPTBLINDEX	8		/* 0 - 255 */
#else MMB	/* PMMU */
#define	STBLINDEX	9
/*#define	PTBLINDEX	10*/
#define	PTBLINDEX	9
#define	PPAGESHIFT	PAGESHIFT
#define	PSTBLINDEX	STBLINDEX
#define	PPTBLINDEX	PTBLINDEX
#endif MMB

/* values for page size field of translation control register
 * (only these values are supported)
 * This also happens to be the same as PAGESHIFT. */
#define	PS2K		11
#define	PS4K		12
#define	PS8K		13
#define	PS16K		14
#define	PS32K		15

/* maximum number of entries in tables at each level
 * (physical not logical) */
#ifdef MMB
#define	NTBLA		(1<<7)		/* max. number entries in MMB table A */
#endif MMB
#define	NSTBL		(1<<PSTBLINDEX)	/* max. number entries in seg table */
#define	NPTBL		(1<<PPTBLINDEX)	/* max. number entries in page table */

#define SEGSHIFT	(PAGESHIFT+PTBLINDEX)		/* shift of seg num */
#define	SEGMASK		(((1<<STBLINDEX)-1)<<SEGSHIFT)	/* seg in virt addr */
#define	PAGEMASK	(((1<<PTBLINDEX)-1)<<PAGESHIFT)	/* page in virt addr */
#define	PPAGEMASK	(((1<<PPTBLINDEX)-1)<<PPAGESHIFT)/* phys page */
#define	PAGESIZE	(1<<PAGESHIFT)			/* page (click) size */
#define	PPAGESIZE	(1<<PPAGESHIFT)			/* phys page size */
#define ADDRMASK	(SEGMASK|PAGEMASK|POFFMASK)	/* valid address bits */

#define	vtoseg(v) 	(((v)>>SEGSHIFT) & (SEGMASK>>SEGSHIFT))
#define	vtoppage(v) 	(((v)>>PPAGESHIFT) & (PPAGEMASK>>PPAGESHIFT))

	/* size of the current process */
#define procsize(p)	((p)->p_size)

	/* macros to eliminate sep I/D */
#define fuiword(x)	fuword(x)
#define suiword(x,y)	suword(x,y)

#ifdef PMMU
#  ifdef PMMUMMB
#define flush_atc	/* inline atc flush -- won't blow stack away */	\
	/*	pflusha		# flush all atc entries */		\
	asm("short	0xF000");					\
	asm("short	0x2400");
#  else PMMUMMB
#define flush_atc	/* inline atc flush -- won't blow stack away */	\
	/*	pflusha		# flush all atc entries */	\
	asm("short	0xF000");					\
	asm("short	0x2400");	/* ZZZ for now */
#  endif PMMUMMB
#else PMMU
#define flush_atc	/* inline atc flush -- won't blow stack away */	\
	asm("	mov.l	&0x7,%d0");	/* setup system mode write */	\
	 /*    movecl	d0,dfc  */					\
	asm("	short	0x4e7b");					\
	asm("	short	0x0001");					\
	asm("	mov.l	rootbl,%d0");					\
	asm("	mov.l	mmuaddr,%a0");	/* write arg to root pointer */	\
	 /*    movesl	d0,a0@  */					\
	asm("	short	0x0e90");					\
	asm("	short	0x0800")
#endif PMMU

#ifdef MMB
#define nlduptbl(nuptbl)			\
	asm("	mov.l	nuptbl,%a0");		\
	asm("	mov.l	uptbl,%a1");		\
	asm("	mov.l	&8,%d0");		\
	 /*  LS%89:		*/		\
	asm("	mov.l	(%a0)+,(%a1)+");	\
	asm("	sub.l	&1,%d0");		\
	 /*    bne	LS%89	*/		\
	asm("	short	0x6600");		\
	asm("	short	0xFFFA")
#else MMB
#define nlduptbl(nuptbl)			\
	asm("	mov.l	nuptbl,%a0");		\
	asm("	mov.l	uptbl,%a1");		\
	asm("	mov.l	&1,%d0");		\
	 /*  LS%89:		*/		\
	asm("	mov.l	(%a0)+,(%a1)+");	\
	asm("	sub.l	&1,%d0");		\
	 /*    bne	LS%89	*/		\
	asm("	short	0x6600");		\
	asm("	short	0xFFFA")
#endif MMB

/* runtime options */
#define RT_M20CACHE	01	/* enable 68020 chip cache */
#define RT_BCACHE	02	/* enable VM04 board cache */
#define RT_MMB		010	/* MMB modify/reference bit errata */
#define RT_VBMON	020	/* VM04 VERSAbus DMA monitor errata */
#define	RT_A23G		0100	/* M68020 A23G errata */
#define	RT_A45J		0200	/* M68020 A45J errata */
#define	RT_A92E		0400	/* M68020 A92E errata */
#define	RT_20CCHEBUG    (RT_A92E|RT_A23G) /* M68020 instruction cache bug */
