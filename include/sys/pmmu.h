/*	@(#)pmmu.h	UniPlus VVV.2.1.1	*/
/*
 * MC68851 PMMU (Paged Memory Management Unit)
 *	with notes for the MMB (M68KVMMB Memory Management Board) subset
 *
 * (C) 1985 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 *	MMC: MC68461 Memory Management Controller
 *	(root pointer and translation control registers)
 */
/* function codes - indices into root table (rootbl) */
#define	FCUSERD		1		/* user data space */
#define	FCUSERP		2		/* user program space */
#define	FCSUPVD		5		/* supervisor data space */
#define	FCSUPVP		6		/* supervisor program space */
#define	FCCPU		7		/* CPU space */
#define	NFC		8		/* number of function codes */

/* translation control register (TC) */
struct mmctc {
	unsigned short	tc_e:1,		/* enable */
			tc_zero:5,	/* zeroes */
			tc_sre:1,	/* supervisor root pointer enable */
			tc_fcl:1,	/* function code lookup */
			tc_ps:4,	/* page size */
			tc_is:4;	/* initial shift */
	unsigned short	tc_ti;		/* table index fields */
};

/* mmc_e: enable/disable address translation - cleared on reset */
#define	TC_EOFF		0x0
#define	TC_EON		0x1

/* mmc_sre: enable/disable supervisor root pointer in 68851 (only disable
 * is supported on MMB) */
#define	TC_SREOFF	0x0

/* mmc_fcl: enable/disable function code lookup (only enable is supported
 * on MMB) */
#define	TC_FCLON	0x1

/* mmc_ps: page size (only 1K bytes is supported on MMB) */
#define	TC_PS1K		10
#define	TC_PS2K		PS2K
#define	TC_PS4K		PS4K
#define	TC_PS8K		PS8K
#define	TC_PS16K	PS16K
#define	TC_PS32K	PS32K

/* mmc_is: number of bits ignored during table searching (only 32-bit and
 * 24-bit logical addresses are supported on MMB - table index fields above
 * are fixed by value of mmc_is) */
#define	TC_IS32		0x0
#define	TC_IS24		0x8

/* mmc_ti: 4 table index fields TIA, TIB, TIC, TID - number of bits of
 * logical address used for indexing at each level of lookup
 * These are the values as fixed on the MMB for 32-bit and 24-bit addresses. */
#define	TC_TIA32	0x7000
#define	TC_TIB32	0x0700
#define	TC_TIC32	0x0080
#define	TC_TID32	0x0000
#define	TC_TI32		(TC_TIA32|TC_TIB32|TC_TIC32|TC_TID32)

#define	TC_TIA24	0x6000
#define	TC_TIB24	0x0800
#define	TC_TIC24	0x0000
#define	TC_TID24	0x0000
#define	TC_TI24		(TC_TIA24|TC_TIB24|TC_TIC24|TC_TID24)

/* a page (click) is 8 physical MMU pages */
/*#define	ptopp(x)	((x)<<3)
/*#define	pptop(x)	((x)>>3)
/*#define	pptob(x)	((x)<<10)
/*#define	btopp(x)	(((x)+1023)>>10)
 */

/* maximum memory mapped by one entry in each table as fixed on MMB */
#define	MAXTBLA		(1<<25)
/*#define	MAXTBLB		(1<<18)
/*#define	MAXTBLC		(1<<10)
 */

#ifdef LATER
/* root pointer (RP) - pointer to base of first translation table
 * (only upper 28 bits supported on MMC - must be on 16 byte boundary) */
extern struct ldesc *rootbl;	/* function code table */
#endif LATER

/* mmbtbla is the extra pointer table for the MMB implementation. */
#ifdef MMB
extern ste_t *mmbtbla;		/* addr of table A */
#define	TAKINDEX	0	/* kernel index into table A */
#define	TAUINDEX	32	/* user index into table A */
#define	TAKLASTINDEX	127	/* kernel index into last segment of table A */
#define	tblatova(x)	(MAXTBLA*(x))
#define TAUNUM		4
#define USEROFF		(TAUINDEX << 25)
#define ANUMSHFT	(PPAGESHIFT+PPTBLINDEX+PSTBLINDEX)
#define ANUMMASK	((1 << (32-ANUMSHFT))-1)
#define	anum(X)		(((int)(X) >> ANUMSHFT) & ANUMMASK)	/* bits 25-31 */
#endif MMB

#ifdef LATER
/* table B */
extern struct ldesc utblbp[NTBLB];	/* user translation table B */
extern struct ldesc *ktblb;	/* addr of kernel's table B (for 1 to 1 map and
					udot for current process) */
extern int nktblb;		/* number of entries in ktblb for 1-1 map */
#define	TBKINDEX	0	/* first kernel index into table B */

/* table C's (page tables) */
extern long *tblc;		/* addr of table C (page table) */
extern long *kutblc;		/* map of udot for current process */
extern long *ktblc;		/* map physical memory 1 to 1 for kernel */
extern long *iotblc;		/* map I/O space */
/*#define	TCIO1INDEX	192	/* I/O map for 0xFFFB0000-0xFFFB03FF
				   (SIO, timer & VM04 status/control) */
/*#define	TCIO2INDEX	252	/* I/O map for 0xFFFFF000-0xFFFFF3FF
				   (SCSI) */
/*#define	TCIO3INDEX	128	/* I/O map for 0xFFFA0000-0xFFFA03FF
				   (ROM restart address) */
/*#define	TCIO4INDEX	255	/* I/O map for 0xFFFFFC00-0xFFFFFFFF
				   (VM13 status/control reg.) */
#endif LATER
