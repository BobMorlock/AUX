/*	@(#)param.h	UniPlus V.2.1.6	*/
/*
 * Machine dependent constants
 */

#define BPWSHFT		2	/* Shift for nbr of bytes per word. */
#define BPDSHFT		9	/* Shift for bytes per disk block. */
#define DOFFMASK	0x1FF	/* Mask for offset into disk block. */

#define MSWAPX  15              /* pseudo mount table index for swapdev */
#define NSEGP		(1<<STBLINDEX)/* Max nbr of segments per process. */
#define	USIZE		1	/* size of user block in pages. */
#define LOCSIZE 	1	/* size of local data area in pages */
#define	COPYSEGSZ	2	/* Size of copyseg in pages.	*/

#define	HZ		60	/* Ticks/second of the clock */

/*
 * fundamental variables
 * don't change
 */

#define NOFILE	32		/* max open files per process */
#define	MAXPID	30000		/* max process id */
#define	MAXUID	60000		/* max user id */
#define	MAXLINK	1000		/* max links */

#define	NGROUPS	8		/* max number groups */
#define	NOGROUP	-1		/* marker for empty group set member */

#define	CANBSIZ	256		/* max size of typewriter line	*/
#define	NCARGS	5120		/* # characters in exec arglist */

#define	SSIZE	1		/* initial stack size (in clicks) */
#define	SINCR	1		/* increment of stack (in clicks) */

#define PVFS	27
#define	PLOCK	35
/*
 * priorities
 */

#define	PMASK	0177
#define	PCATCH	0400
#define	PSWP	0
#define	PMEM	0
#define	PINOD	10
#define	PRIBIO	20
#define	PZERO	25
#define	PDELAY	(PZERO-1)
#define	NZERO	20
#define	PPIPE	26
#define	PWAIT	30
#define	PSLEP	39
#define	PUSER	60
#define	PIDLE	127
#define	PMAX	127

/*
 * fundamental constants of the implementation
 */

#define	NBPW	sizeof(int)	/* number of bytes in an integer */
#define NBBY    8               /* number of bits in a byte */

#define	NULL	0

#define	MAXBSIZE	SBUFSIZE
#define	DEV_BSIZE	512
#define	DEV_BSHIFT	9		/* log2(DEV_BSIZE) */
#define	DEV_BMASK	(DEV_BSIZE-1)
#define BLKDEV_IOSIZE	SBUFSIZE

#define	MAXNAMLEN	255
#define MAXPATHLEN	1024
#define MAXSYMLINKS	8

#define	CMASK	0		/* default mask for file creation */
#define	CDLIMIT	(1L<<24)	/* default max write address */
#define	NODEV	(dev_t)(-1)
#define	ROOTINO	((ino_t)2)	/* i number of all roots */
#define	NICINOD	100		/* number of superblock inodes */

#ifndef	FsTYPE
#define	FsTYPE	3
#endif

#if FsTYPE==1
	/* Original 512 byte file system */
#define	BSIZE	512		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	9		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	0777		/* BSIZE-1 */
#define	INOPB	8		/* inodes per block */
#define	INOSHIFT	3	/* LOG2(INOPB) if exact */
#define	NMASK	0177		/* NINDIR-1 */
#define	NSHIFT	7		/* LOG2(NINDIR) */

#define	FsBSIZE(fsp)	BSIZE
#define	FsBSHIFT(fsp)	BSHIFT
#define	FsNINDIR(fsp)	NINDIR
#define	FsBMASK(fsp)	BMASK
#define	FsBOFF(fsp, x)	((x)&BMASK)
#define	FsBNO(fsp, x)	((x)>>BSHIFT)
#define	FsINOPB(fsp)	INOPB
#define	FsLTOP(fsp, b)	b
#define	FsPTOL(fsp, b)	b
#define	FsNMASK(fsp)	NMASK
#define	FsNSHIFT(fsp)	NSHIFT
#define	FsITOD(fsp, x)	(((unsigned)x+(2*8-1))>>3)
#define	FsITOO(fsp, x)	(((unsigned)x+(2*8-1))&07)
#define	FsINOS(fsp, x)	((x&~07)+1)
#endif

#if FsTYPE==2
	/* New 1024 byte file system */
#define	BSIZE	1024		/* size of file system block (bytes) */
#define	SBUFSIZE	BSIZE	/* system buffer size */
#define	BSHIFT	10		/* LOG2(BSIZE) */
#define	NINDIR	(BSIZE/sizeof(daddr_t))
#define	BMASK	01777		/* BSIZE-1 */
#define	INOPB	16		/* inodes per block */
#define	INOSHIFT	4	/* LOG2(INOPB) if exact */
#define	NMASK	0377		/* NINDIR-1 */
#define	NSHIFT	8		/* LOG2(NINDIR) */
#define	FsBSIZE(fsp)	BSIZE
#define	FsBSHIFT(fsp)	BSHIFT
#define	FsNINDIR(fsp)	NINDIR
#define	FsBMASK(fsp)	BMASK
#define	FsBOFF(fsp, x)	((x)&BMASK)
#define	FsBNO(fsp, x)	((x)>>BSHIFT)
#define	FsINOPB(fsp)	INOPB
#define	FsLTOP(fsp, b)	((b)<<1)
#define	FsPTOL(fsp, b)	((b)>>1)
#define	FsNMASK(fsp)	NMASK
#define	FsNSHIFT(fsp)	NSHIFT
#define	FsITOD(fsp, x)	(((unsigned)x+(2*16-1))>>4)
#define	FsITOO(fsp, x)	(((unsigned)x+(2*16-1))&017)
#define	FsINOS(fsp, x)	(((x&~017)+1))
#endif

#if FsTYPE==3
#define	SBUFSIZE	1024	/* system buffer size */
#define	FsLRG(fsp)	((fsp)->s_type == Fs2b)
#define	FsBSIZE(fsp)	(FsLRG(fsp) ? 1024 : 512)
#define	FsBSHIFT(fsp)	(FsLRG(fsp) ? 10 : 9)
#define	FsNINDIR(fsp)	(FsLRG(fsp) ? 256 : 128)
#define	FsBMASK(fsp)	(FsLRG(fsp) ? 01777 : 0777)
#define	FsBOFF(fsp, x)	(FsLRG(fsp) ? ((x)&01777) : ((x)&0777))
#define	FsBNO(fsp, x)	(FsLRG(fsp) ? ((x)>>10) : ((x)>>9))
#define	FsINOPB(fsp)	(FsLRG(fsp) ? 16 : 8)
#define	FsLTOP(fsp, b)	(FsLRG(fsp) ? (b)<<1 : b)
#define	FsPTOL(fsp, b)	(FsLRG(fsp) ? (b)>>1 : b)
#define	FsNMASK(fsp)	(FsLRG(fsp) ? 0377 : 0177)
#define	FsNSHIFT(fsp)	(FsLRG(fsp) ? 8 : 7)
#define	FsITOD(fsp, x)	(daddr_t)(FsLRG(fsp) ? \
	((unsigned)x+(2*16-1))>>4 : ((unsigned)x+(2*8-1))>>3)
#define	FsITOO(fsp, x)	(daddr_t)(FsLRG(fsp) ? \
	((unsigned)x+(2*16-1))&017 : ((unsigned)x+(2*8-1))&07)
#define	FsINOS(fsp, x)	(FsLRG(fsp) ? \
	((x&~017)+1) : ((x&~07)+1))
#endif

#if FsTYPE==7
	/* 512, 1024, and 2048 bytes per block system */
#define	SBUFSIZE	2048	/* system buffer size */
#define	FsLRG(fsp)	((fsp)->s_type == Fs2b)
#define	FsXLRG(fsp)	((fsp)->s_type == Fs4b)
#define	FsBSIZE(fsp)	(FsXLRG(fsp) ? (DEV_BSIZE*4) : \
	(FsLRG(fsp) ? (DEV_BSIZE*2) : DEV_BSIZE))
#define	FsBSHIFT(fsp)	(FsXLRG(fsp) ? 11 : ((FsLRG(fsp) ? 10 : 9)))
#define	FsNINDIR(fsp)	(FsXLRG(fsp) ? 512 : ((FsLRG(fsp) ? 256 : 128)))
#define	FsBMASK(fsp)	(FsXLRG(fsp) ? 03777 : ((FsLRG(fsp) ? 01777 : 0777)))
#define	FsBOFF(fsp, x)	(FsXLRG(fsp) ? ((x)&03777) : \
	(FsLRG(fsp) ? ((x)&01777) : ((x)&0777)))
#define	FsBNO(fsp, x)	(FsXLRG(fsp) ? ((x)>>11) : \
	(FsLRG(fsp) ? ((x)>>10) : ((x)>>9)))
#define	FsINOPB(fsp)	(FsXLRG(fsp) ? 32 : (FsLRG(fsp) ? 16 : 8))
#define	FsLTOP(fsp, b)	(FsXLRG(fsp) ? (b)<<2 : (FsLRG(fsp) ? (b)<<1 : b))
#define	FsPTOL(fsp, b)	(FsXLRG(fsp) ? (b)>>2 : (FsLRG(fsp) ? (b)>>1 : b))
#define	FsNMASK(fsp)	(FsXLRG(fsp) ? 0777 : (FsLRG(fsp) ? 0377 : 0177))
#define	FsNSHIFT(fsp)	(FsXLRG(fsp) ? 9 : (FsLRG(fsp) ? 8 : 7))
#define	FsITOD(fsp, x)	(daddr_t)(FsXLRG(fsp) ? ((unsigned)x+(2*32-1))>>5 : \
	(FsLRG(fsp) ? ((unsigned)x+(2*16-1))>>4 : ((unsigned)x+(2*8-1))>>3))
#define	FsITOO(fsp, x)	(daddr_t)(FsXLRG(fsp) ? ((unsigned)x+(2*32-1))&037 : \
	(FsLRG(fsp) ? ((unsigned)x+(2*16-1))&017 : ((unsigned)x+(2*8-1))&07))
#define	FsINOS(fsp, x)	(FsXLRG(fsp) ? ((x&~037)+1) : \
	(FsLRG(fsp) ? ((x&~017)+1) : ((x&~07)+1)))
#endif

#if FsTYPE==11
	/* 512, 1024 and 4096 bytes per block system */
#define	SBUFSIZE	4096	/* system buffer size */
#define	FsLRG(fsp)	((fsp)->s_type == Fs2b)
#define	FsXXLRG(fsp)	((fsp)->s_type == Fs8b)
#define	FsBSIZE(fsp)	(FsXXLRG(fsp) ? (DEV_BSIZE*8) : \
			(FsLRG(fsp) ? (DEV_BSIZE*2) : DEV_BSIZE))
#define	FsBSHIFT(fsp)	(FsXXLRG(fsp) ? 12 : \
			(FsLRG(fsp) ? 10 : 9))
#define	FsNINDIR(fsp)	(FsXXLRG(fsp) ? 1024 : \
		 	(FsLRG(fsp) ? 256 : 128))
#define	FsBMASK(fsp)	(FsXXLRG(fsp) ? 07777 : \
			(FsLRG(fsp) ? 01777 : 0777))
#define	FsBOFF(fsp, x)	(FsXXLRG(fsp) ? ((x)&07777)  : \
			(FsLRG(fsp) ? ((x)&01777) : ((x)&0777)))
#define	FsBNO(fsp, x)	(FsXXLRG(fsp) ? ((x)>>12) : \
			(FsLRG(fsp) ? ((x)>>10) : ((x)>>9)))
#define	FsINOPB(fsp)	(FsXXLRG(fsp) ? 64 : \
			(FsLRG(fsp) ? 16 : 8))
#define	FsLTOP(fsp, b)	(FsXXLRG(fsp) ? (b)<<3 : \
			(FsLRG(fsp) ? (b)<<1 : b))
#define	FsPTOL(fsp, b)	(FsXXLRG(fsp) ? (b)>>3 : \
			(FsLRG(fsp) ? (b)>>1 : b))
#define	FsNMASK(fsp)	(FsXXLRG(fsp) ? 01777 : \
			(FsLRG(fsp) ? 0377 : 0177))
#define	FsNSHIFT(fsp)	(FsXXLRG(fsp) ? 10 : \
			(FsLRG(fsp) ? 8 : 7))
#define	FsITOD(fsp, x)	(daddr_t) \
			(FsXXLRG(fsp) ? ((unsigned)x+(2*64-1))>>6 : \
			(FsLRG(fsp) ? ((unsigned)x+(2*16-1))>>4 : \
			((unsigned)x+(2*8-1))>>3))
#define	FsITOO(fsp, x)	(daddr_t) \
			(FsXXLRG(fsp) ? ((unsigned)x+(2*64-1))&077 : \
			(FsLRG(fsp) ? ((unsigned)x+(2*16-1))&017 : \
			((unsigned)x+(2*8-1))&07))
#define	FsINOS(fsp, x)	(FsXXLRG(fsp) ? ((x&~077)+1) : \
			(FsLRG(fsp) ? ((x&~017)+1) : ((x&~07)+1)))
#endif

#define	FsBLKOFF(fsp,b)	((b) & FsBMASK(fsp))

#define	BBLOCK		((daddr_t)(0))
#define BBSIZE		(DEV_BSIZE)
#define	SBLOCK		((daddr_t)(BBLOCK + BBSIZE / DEV_BSIZE))
#define SBSIZE		(DEV_BSIZE)
/* added for compatibility with old utilities */
#define SUPERB		SBLOCK
#define SUPERBOFF	(SBLOCK * DEV_BSIZE)

#define	NICFREE	50		/* number of superblock free blocks */
#define	SUPBSIZE	512	/* number of bytes in the superblock */

#define MEMIOPTE	LOCSIZE
#define MEMIOSIZE	1
#define PHYSIOPTE	(MEMIOPTE+MEMIOSIZE)
#define	PHYSIOSIZE	2	/* size of physio block in pages. */

#define MAXMEM	(btop(v.v_uend-v.v_ustart))/* Maximum size of user program */
#define MAXBLK	20		/* max blocks possible for physical I/O */
#define USTART	0x0		/* logical start of user program */
#define UEND	(ADDRMASK+1)	/* logical end of user program +1 */
#ifdef	MMB
#define UDOT	0x1F40000	/* Logical base of udot area */
#else	MMB
#define UDOT	0x3FC00000	/* Logical base of udot area */
#endif	MMB
#define	CLKTICK	16667		/* microseconds in a clock tick */
#define	NPHYS	32		/* max simultaneous phys() calls */
#define STKMAGIC 0xA55A5AA5	/* stack verification size magic number */

#define	NSCATLOAD 0		/* number of scatter load segments */
#define DOFFSET	0		/* Data offset */
#define KVOFFSET 0		/* Kernel virtual offset from physical 0 mem */
#define TXTRND 0x400000		/* Shared text rounding factor of 4Meg */

#define	CPUPRI(ps)	((ps) & PS_IPL)		/* Mask for CPU priority bits */
#define	BASEPRI(ps)	(CPUPRI(ps) != 0)	/* CPU base priority */

#define	lobyte(X)	(((unsigned char *)&(X))[1])
#define	hibyte(X)	(((unsigned char *)&(X))[0])
#define	loword(X)	(((ushort *)&(X))[1])
#define	hiword(X)	(((ushort *)&(X))[0])

#ifdef	KERNEL
#define MIN(a,b)	(((a) < (b))?(a):(b))
#define MAX(a,b)	(((a) > (b))?(a):(b))
#define min(a,b)	MIN(a,b)		/* was in math.s for 3B20 */
#define bcopy(a,b,c)	blt(b, a, c)
#endif	KERNEL

/*
 * Macros for counting and rounding.
 */
#define howmany(x, y)   (((x)+((y)-1))/(y))
#define roundup(x, y)   ((((x)+((y)-1))/(y))*(y))

	/* macros to eliminate sep I/D */
#define fuiword(x)	fuword(x)
#define suiword(x,y)	suword(x,y)

#define SPL0()		asm("	mov.w	&0x2000,%sr")
#define SPL1()		asm("	mov.w	&0x2100,%sr")
#define SPL2()		asm("	mov.w	&0x2200,%sr")
#define SPL3()		asm("	mov.w	&0x2300,%sr")
#define SPL4()		asm("	mov.w	&0x2400,%sr")
#define SPL5()		asm("	mov.w	&0x2500,%sr")
#define SPL6()		asm("	mov.w	&0x2600,%sr")
#define SPL7()		asm("	mov.w	&0x2700,%sr")

#define splclk(x)	spl6(x)
#define spltty(x)	spl5(x)
#define SPLHI()		SPL7()
#define SPLCLK()	SPL6()
#define SPLTTY()	SPL5()
