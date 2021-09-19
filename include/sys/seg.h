/* @(#)seg.h	6.3 */
/*	Segment table dependent constants.
 */

	/* Shift for segment number from address. */
#define SNUMSHFT	(PAGESHIFT+PTBLINDEX)
	/* Mask for segment number after shift. */
#define SNUMMASK	((1<<STBLINDEX)-1)
	/* Mask for offset into segment. */
#define SOFFMASK	((1<<SNUMSHFT)-1)
	/* Shift for pages per segment. */
#define PPSSHFT		NPGPTSHFT	/* Nbr of pages per page table (seg) */

#ifndef LOCORE
#define SEGSIZE		(1<<SNUMSHFT)	/* bytes per full segment */
#else LOCORE
#define SEGSIZE		0x80000		/* bytes per full segment */
#endif LOCORE

/*
 *	Long Descriptors (Pointer Table Entries)
 *
 *   ----------------------------------------------------------------------
 *   |lu| limit |ral|wal|sg|s| 0000 |u|wp| dt | page table address |unused|
 *   ----------------------------------------------------------------------
 *     1    15    3   3   1 1    4   1  1   2            28            4
 */
#ifndef LOCORE
typedef union ldesc {
	struct {
		unsigned short
			ld_lu:1,	/* is ld_limit lower/upper limit? */
			ld_limit:15;	/* min/max for index into next table */
		unsigned short
			ld_stat:14,	/* status */
			ld_dt:2;	/* descriptor type */
		unsigned long
			ld_addr;	/* table address */
	} segm;
	struct {
		unsigned long ld_msadr;	/* most sig longword */
		unsigned long ld_lsadr;	/* least sig longword */
	} segf;
} ste_t;
#endif LOCORE

/* lower/upper values in segf.ld_msadr (valid for DT4B or DT8B descriptors) */
#define	UPPLIMIT	0x00000000	/* ld_limit is upper limit of index */
#define	LOWLIMIT	0x80000000	/* ld_limit is lower limit of index */

/* desc type values in segf.ld_msadr (long descriptor type encodings) */
#define	DTLINV		0x00000000	/* invalid table */
#ifndef MMB
#define	DTPD		0x00000001	/* ptr to contiguous valid pages */
#endif MMB
#define	DT4B		0x00000002	/* ptr to base of table of short desc */
#define	DT8B		0x00000003	/* ptr to base of table of long desc */

/* status values in segf.ld_msadr (valid for DT4B or DT8B descriptors) */
#define	STRAL		0x0000E000	/* read access level */
#define	STRUS		0x0000E000	/* RAL value for user or supv access */
#define	STRS		0x00000000	/* RAL value for supv access only */
#define	STWAL		0x00001C00	/* write access level */
#define	STWUS		0x00001C00	/* WAL value for user or supv access */
#define	STWS		0x00000000	/* WAL value for supv access only */
#define	STSG		0x00000200	/* shared globally */
#define	STS		0x00000100	/* supervisor space references only */
#define	STUSED		0x00000008	/* used (never cleared by MMC) */
#define	STWPROT		0x00000004	/* write protected at segment level */

/*
 *	Segment Table Entry
 *	All are in segf.ld_msadr except for S_ADDR which is in segf.ld_lsadr.
 */
#define	S_PAGES		0x7FFF0000	/* highest/lowest equipped page */
#define S_PAGESSHFT	16		/* shift to get S_PAGES */
#define	S_PERM		0x0000FFFF	/* segment permissions (status field) */
#define	S_VFIELD	0x00000003	/* ste valid field (descriptor type) */
/* use this bit as the "valid" bit - desc. field is 2 bits but low bit is
 * zero in both encodings that are used (0x0 for invalid, 0x2 for valid) */
#define	S_VALID		DT4B		/* segment table entry valid */
#define	S_KWRITE	(S_VALID|STWS)	/* supv only write access */
#define	S_KREAD		(S_VALID|STRS)	/* supv only read access */
#define	S_KRW		(S_VALID|STRS|STWS)/* supv only read/write access */
#define	S_KUWRITE	(S_VALID|STWUS)	/* user or supv write access */
#define	S_KUREAD	(S_VALID|STRUS)	/* user or supv read access */
#define	S_KURW		(S_VALID|STRUS|STWUS)/* user or supv r/w access */
#define	S_IVD		DTLINV		/* invalid STE */
#define S_ADDR		0xFFFFFFF0	/* page table physical address */

#define SEG_RO		(S_KUREAD|STWPROT)
#define SEG_RW		S_KURW
#define isreadonly(ste)		(((ste).segf.ld_msadr & STWAL) != STWUS)

/*
 *	Macros to write segment table entries.
 */
/* write a normal segment table entry (uses upper limit of size-1) */
#define	wtste(ste, mode, size, pt)					\
	{	(ste).segf.ld_msadr = (unsigned long)(UPPLIMIT | (mode)	\
			| ((((size)<<PPTOPSHFT) - 1) << S_PAGESSHFT));	\
		(ste).segf.ld_lsadr = (unsigned long)(pt);		\
	}
/* write a stack segment table entry (uses lower limit of maxindex-size) */
#define	wtsste(ste, mode, size, pt) 					\
	{	(ste).segf.ld_msadr = (unsigned long)(LOWLIMIT | (mode)	\
			| ((NPPGPT-((size)<<PPTOPSHFT))<<S_PAGESSHFT)); \
		(ste).segf.ld_lsadr = (unsigned long)(pt);		\
	}
#ifndef MMB
/* write an early terminating segment table entry (maps contiguous area)  */
#define	wtlpte(ste, mode, size, paddr)					\
	{	(ste).segf.ld_msadr = (unsigned long)(UPPLIMIT | (mode)	\
			| ((((size)<<PPTOPSHFT) - 1) << S_PAGESSHFT));	\
		(ste).segm.ld_dt = DTPD; 				\
		(ste).segf.ld_lsadr = (unsigned long)(paddr);		\
	}
#endif MMB

/* Get the address of the page table from the segment table entry.  The
 * upper 28 bits of the ld_lsadr field define the base of the next table. */
#define	steaddr(ste)	((unsigned)((ste).segf.ld_lsadr) & S_ADDR)
#define	steprot(ste)	((unsigned)((ste).segf.ld_msadr) & S_PERM)

/*
 *	Root Table Entry
 *	All are in segf.ld_msadr except for RT_ADDR which is in segf.ld_lsadr.
 *	NOTE:	These values are also used for mmbtbla, the extra pointer
 *		table for the MMB implementation.
 */
#define	RT_PAGES	0x7FFF0000	/* highest/lowest equipped page */
#define RT_PAGESSHFT	16		/* shift to get RT_PAGES */
#define	RT_PERM		0x0000FFFF	/* status field */
#define	RT_VFIELD	0x00000003	/* descriptor type */
#define	RT_VALID	DT8B		/* ptr to pointer table */
#define	RT_KWRITE	(RT_VALID|STWS)	/* supv only write access */
#define	RT_KREAD	(RT_VALID|STRS)	/* supv only read access */
#define	RT_KRW		(RT_VALID|STRS|STWS)/* supv only read/write access */
#define	RT_KUWRITE	(RT_VALID|STWUS)/* user or supv write access */
#define	RT_KUREAD	(RT_VALID|STRUS)/* user or supv read access */
#define	RT_KURW		(RT_VALID|STRUS|STWUS)/* user or supv r/w access */
#define	RT_IVD		DTLINV		/* invalid root table entry */
#define RT_ADDR		0xFFFFFFF0	/* page table physical address */

/*
 *	Macro to write root table entries.  This macro is also used for
 *	writing mmbtbla entries.  The size field is not translated from
 *	logical to physical pages, but otherwise wtrte is the same as wtste.	
 */
/* write a root table entry (uses upper limit of size-1) */
#define	wtrte(rte, mode, size, pt)					\
	{	(rte).segf.ld_msadr = UPPLIMIT | (mode)			\
			| (((size) - 1) << RT_PAGESSHFT);		\
		(rte).segf.ld_lsadr = pt;				\
	}
#ifdef PMMU
/*
 *	Macro to write a root pointer entry to be loaded into
 *	the PMMU's crp, srp, or drp.
 */
#define wtrpe(rpe, pt)							\
	{	(rpe).segf.ld_msadr = UPPLIMIT | RT_VALID |		\
			((8 - 1) << RT_PAGESSHFT);			\
		(rpe).segf.ld_lsadr = pt;				\
	}
#endif PMMU

/*	Get segment number from byte address.  */
/*	For the PMMU psnum() is identical to snum().
 *	For the MMB psnum() is the physical segment table index (bits 18-24)
 *	and snum() combines the table A and segment tables indices (bits 18-26).
 */
#define	snum(X)		(((int)(X) >> SNUMSHFT) & SNUMMASK)
#define	psnum(X)	(((int)(X) >> (PPAGESHIFT+PPTBLINDEX)) & ((1<<PSTBLINDEX)-1))

/*	Get offset into the segment from a byte address.  */

#define soff(x)   ((uint)(x) & SOFFMASK)

/*	Physical page table address = segpptbl(segment table entry)	*/

#define	segpptbl(s)	((union ptbl *)((s).segf.ld_lsadr & S_ADDR))

/*	Segment size of normal non-stack segment = segsz(segment table entry) */

#define	segsz(s)	(((s).segf.ld_msadr&S_VALID) ?		\
	( (((s).segf.ld_msadr&S_PAGES)>>S_PAGESSHFT) + 1) >> PPTOPSHFT : 0)

/*	Segment size of stack segment = segsz(segment table entry) */

#define	segssz(s)	(((s).segf.ld_msadr&S_VALID) ?		\
	(NPPGPT - (((s).segf.ld_msadr&S_PAGES)>>S_PAGESSHFT) ) >> PPTOPSHFT : 0)

#ifndef LOCORE
extern ste_t	*kstbl;		/* kernel segment table */
#endif LOCORE

#include "sys/useg.h"
