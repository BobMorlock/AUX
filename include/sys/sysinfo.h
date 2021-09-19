/*	@(#)sysinfo.h	UniPlus VVV.2.1.1	*/
/*
 *	System Information.
 */

struct sysinfo {
#define	CPU_IDLE	0
#define	CPU_USER	1
#define	CPU_KERNEL	2
#define	CPU_WAIT	3
#define	CPU_NICE	4
#define	NCPUSTATES	(CPU_NICE - CPU_IDLE + 1)
	time_t	cpu[NCPUSTATES];
#define	W_IO	0
#define	W_SWAP	1
#define	W_PIO	2
#define	WAITSTATES	(W_PIO - W_IO + 1)
	time_t	wait[WAITSTATES];
	long	bread;
	long	bwrite;
	long	lread;
	long	lwrite;
	long	phread;
	long	phwrite;
	long	swapin;
	long	swapout;
	long	bswapin;
	long	bswapout;
	long	pgpgin;
	long	pgpgout;
	long	pswitch;
	long	syscall;
	long	sysread;
	long	syswrite;
	long	sysfork;
	long	sysexec;
	long	runque;
	long	runocc;
	long	swpque;
	long	swpocc;
	long	iget;
	long	namei;
	long	dirblk;
	long	readch;
	long	writech;
	long	intr;
	long	rcvint;
	long	xmtint;
	long	mdmint;
	long	rawch;
	long	canch;
	long	outch;
	long	msg;
	long	sema;
};

struct syswait {
	short	iowait;
	short	swap;
	short	physio;
};

struct syserr {
	long	inodeovf;
	long	fileovf;
	long	textovf;
	long	procovf;
};

struct minfo {
	long 	freemem; 	/* freemem in page */
	long	freeswap;	/* free swap space */
	long    vfault;  	/* translation fault */
	long    demand;		/*  demand zero and demand fill pages */
	long    swap;		/*  pages on swap */
	long    cache;		/*  pages in cache */
	long    file;		/*  pages on file */
	long    pfault;		/* protection fault */
	long    cw;		/*  copy on write */
	long    steal;		/*  steal the page */
	long    freedpgs;	/* pages are freed */
	long    unmodsw;	/* getpages finds unmodified pages on swap */
	long	unmodfl;	/* getpages finds unmodified pages in file */ 
};

extern struct minfo minfo;

#ifdef KERNEL
extern int	dk_busy, dk_nunits;
extern long	dk_time[];
extern long	dk_seek[];
extern long	dk_xfer[];
extern long	dk_wds[];
extern long	dk_bps[];
extern struct sysinfo	sysinfo;
extern struct syswait	syswait;
extern struct syserr	syserr;
#endif
