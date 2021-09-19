#include "sys/debug.h"

#include "sys/acct.h"
struct	acct	acctbuf;

#include "sys/buf.h"
struct	buf	*sbuf;		/* Start of buffer headers */
struct  bufhd bufhash[NHBUF];  /* heads of hash lists */
		/* Start of I/O buffers. */
/*char		physbuf[NBUF * SBUFSIZE + sizeof(int) - 1];*/
struct	buf	bfreelist[BQUEUES];	/* head of the free list of buffers */
struct	pfree	pfreelist;		/* Head of physio header pool */
#ifndef AUTOCONFIG
struct	buf	pbuf[NPBUF];		/* Physical io header pool */
#else
struct	buf	*pbuf;			/* Physical io header pool */
#endif AUTOCONFIG
caddr_t		iobufs;			/* Start of I/O buffers. */

#include "svfs/fsdir.h"
#include "sys/signal.h"
#include "sys/seg.h"
#include "sys/time.h"
#include "sys/user.h"

#include "sys/tty.h"

#include "sys/file.h"
#ifndef AUTOCONFIG
struct	file	file[NFILE];	/* file table */
#else
struct	file	*file;		/* file table */
#endif AUTOCONFIG

#include "sys/vnode.h"
#include "svfs/inode.h"
#ifndef AUTOCONFIG
struct	inode	inode[NINODE];	/* inode table */
#else
struct	inode	*inode;		/* inode table */
#endif AUTOCONFIG
struct	vnode	*acctp;

#include "sys/map.h"
#ifndef AUTOCONFIG
struct	map	sptmap[NSPTMAP];	/* map of kernel page tables */
#else
struct	map	*sptmap;		/* map of kernel page tables */
#endif AUTOCONFIG


#include "sys/callout.h"
#ifndef AUTOCONFIG
struct callout callout[NCALL];
#else
struct callout *callout;
#endif AUTOCONFIG

#include "sys/vfs.h"
#include "svfs/mount.h"
#ifndef AUTOCONFIG
struct	mount	mounttab[NMOUNT];
#else
struct	mount	*mounttab;
#endif AUTOCONFIG

#include "sys/elog.h"
#include "sys/err.h"
struct	err	err = {
	NESLOT,
} ;

#include "sys/locking.h"
#ifndef AUTOCONFIG
struct	locklist locklist[NFLOCK];
#else
struct	locklist *locklist;
#endif AUTOCONFIG

#include "sys/page.h"
pte_t	*kptbl;		/* Base of kernel page tables. */
pte_t	*ekptbl;	/* End of kernel page tables. */
pte_t	*mmpte;		/* Page tables for memory driver. */
caddr_t	uptbase;	/* Lowest address of user page table.  */
caddr_t	*uptvaddrs;	/* Table for translating physical to	*/
			/* virtual page table addresses.	*/
int	nptalloced;	/* Total number of page tables allocated.	*/
int	nptfree;	/* Number of free page tables.			*/

#include "sys/region.h"
#ifndef AUTOCONFIG
reg_t	region[NREGION];
#else
reg_t	*region;
#endif AUTOCONFIG
reg_t	ractive;
reg_t	rfree;
reg_t	sysreg;

#include "sys/proc.h"
#ifndef AUTOCONFIG
struct	proc	proc[NPROC];	/* process table */
#else
struct	proc	*proc;		/* process table */
#endif AUTOCONFIG

struct prog *runq;		/* Head of linked list of running procs. */

#include "sys/pfdat.h"
struct pfdat	phead;		/* Head of free page list. */
struct pfdat	*pfdat;		/* Page frame database. */
struct pfdat	*phash;		/* Page hash access to pfdat. */
struct pfdat	ptfree;		/* Head of page table free list. */
int		phashmask;	/* Page hash mask. */

#include "sys/sysinfo.h"
struct sysinfo sysinfo;
struct	syswait	syswait;
struct	syserr	syserr;
struct  minfo minfo;

int	dk_busy, dk_nunits;
long	dk_time[DK_NDRIVE];
long	dk_seek[DK_NDRIVE];
long	dk_xfer[DK_NDRIVE];
long	dk_wds[DK_NDRIVE];
long	dk_bps[DK_NDRIVE];

/*
 *	Streams interface routines
 */

extern int nodev();
extern int nulldev();

int (*stream_open)() = nodev;
int (*stream_close)() = nulldev;
int (*stream_read)() = nulldev;
int (*stream_write)() = nulldev;
int (*stream_ioctl)() = nulldev;
int (*stream_run)() = nulldev;
char qrunflag = 0;
char queueflag = 0;
int strmsgsz = STRMSGSZ;

#include "sys/var.h"
struct var v = {
	NBUF,
	NCALL,
	NINODE,
#ifdef lint
#ifndef AUTOCONFIG
	&inode[NINODE],
#else
	0,
#endif AUTOCONFIG
	NFILE,
#ifndef AUTOCONFIG
	&file[NFILE],
#else
	0,
#endif AUTOCONFIG
	NMOUNT,
#ifndef AUTOCONFIG
	&mounttab[NMOUNT],
#else
	0,
#endif AUTOCONFIG
	NPROC,
#ifndef AUTOCONFIG
	&proc[1],
#else
	0,
#endif AUTOCONFIG
	NTEXT,
	&text[NTEXT],
#else
#ifndef AUTOCONFIG
	(char *)(&inode[NINODE]),
#else
	0,
#endif AUTOCONFIG
	NFILE,
#ifndef AUTOCONFIG
	(char *)(&file[NFILE]),
#else
	0,
#endif AUTOCONFIG
	NMOUNT,
#ifndef AUTOCONFIG
	(char *)(&mounttab[NMOUNT]),
#else
	0,
#endif AUTOCONFIG
	NPROC,
#ifndef AUTOCONFIG
	(char *)(&proc[1]),
#else
	0,
#endif AUTOCONFIG
	NTEXT,
	(char *)NULL,
#endif
	NCLIST,
	NSABUF,
	MAXUP,
	CMAPSIZ,
	SMAPSIZ,
	NHBUF,
	NHBUF-1,
	NFLOCK,
	NPHYS,
	CLSIZE,
	TXTRND,
	DEV_BSIZE,
	CXMAPSIZ,
	CLKTICK,
	HZ,
	USIZE,
	PAGESHIFT,
	PAGEMASK,
	SEGSHIFT,
	SEGMASK,
	USTART,
	UEND,
#ifdef lint
#ifndef AUTOCONFIG
	&callout[NCALL],
#else
	0,
#endif AUTOCONFIG
#else
#ifndef AUTOCONFIG
	(char *)(&callout[NCALL]),
#else
	0,
#endif AUTOCONFIG
#endif
	STACKGAP,
	CPU_MC68000,
	VER_MC68010,
	MMU_SUN,
	DOFFSET,
	KVOFFSET,
	NSVTEXT,
	(char *)NULL,
	NPBUF,
	NSCATLOAD,
	(struct user *)UDOT,
	NREGION,			/* v_region - nbr of regions.	*/
	NSPTMAP,			/* v_sptmap - size of system	*/
					/*	      space map.	*/
	VHNDFRAC,			/* v_vhndfrac - vhand fraction	*/
	MAXPMEM,			/* v_maxpmem - max physical	*/
					/*	       mem to use or 0	*/
					/*	       to use all.	*/
	NMBUFS,				/* buffers for networking */
	NPTY,				/* number of pseudo tty's */
	MAXCORE,
	MAXHEADER,
	NSTREAM,			/* number of stream heads */
	NQUEUE,				/* number of queue heads */
	NBLK4096,			/* number of 4k stream blocks */
	NBLK2048,			/* number of 2k stream blocks */
	NBLK1024,			/* number of 1k stream blocks */
	NBLK512,			/* number of 512 byte stream blocks */
	NBLK256,			/* number of 256 byte stream blocks */
	NBLK128,			/* number of 128 byte stream blocks */
	NBLK64,				/* number of 64 byte stream blocks */
	NBLK16,				/* number of 16 byte stream blocks */
	NBLK4,				/* number of 4 byte stream blocks */
#ifndef AUTOCONFIG
#ifdef lint
	&proc[0],			/* address of the proc table */
#else
	(char *)(&proc[0]),		/* address of the proc table */
#endif lint
#else
	0,				/* &proc[0] address of the proc table */
#endif AUTOCONFIG
} ;

#include	"sys/tuneable.h"
tune_t	tune = {
		GETPGSLOW,	/* t_gpgslo - get pages low limit.	*/
		GETPGSHI,	/* t_gpgshi - get pages high limit.	*/
		GETPGSMSK,	/* t_gpgsmsk - get pages mask.		*/
		VHANDR,		/* t_handr - vhand wakeup rate.		*/
		0,		/* t_vhandl - vhand steal limit.	*/
				/* 	      Set in vhand.		*/
		MAXSC,		/* t_maxsc - max swap count.		*/
		MAXFC,		/* t_maxfc - max free count.		*/
		MAXUMEM,	/* t_maxumem - max user mem in pages.	*/
		PPMEM,		/* t_ppmem - percentage of physical 	*/
				/* 		memory not used to 	*/
				/*		avoid deadlock		*/
};

#include	"sys/swap.h"
swpt_t	swaptab[MSFILES];
int	nextswap;

#include	"sys/ipc.h"
#include	"sys/msg.h"

struct map	msgmap[MSGMAP];

struct msqid_ds	msgque[MSGMNI];
struct msg	msgh[MSGTQL];
struct msginfo	msginfo = {
	MSGMAP,
	MSGMAX,
	MSGMNB,
	MSGMNI,
	MSGSSZ,
	MSGTQL,
	MSGSEG
};

#	ifndef IPC_ALLOC
#	include	"sys/ipc.h"
#	endif
#include	"sys/sem.h"
struct semid_ds	sema[SEMMNI];
struct sem	sem[SEMMNS];
struct map	semmap[SEMMAP];
#ifndef AUTOCONFIG
struct	sem_undo	*sem_undo[NPROC];
#else
struct	sem_undo	**sem_undo;
#endif AUTOCONFIG
#define	SEMUSZ	(sizeof(struct sem_undo)+sizeof(struct undo)*SEMUME)
#ifdef lint
struct sem_undo semu[SEMMNU];
#else
int	semu[((SEMUSZ*SEMMNU)+NBPW-1)/NBPW];
#endif
union {
	short		semvals[SEMMSL];
	struct semid_ds	ds;
	struct sembuf	semops[SEMOPM];
}	semtmp;

struct	seminfo seminfo = {
	SEMMAP,
	SEMMNI,
	SEMMNS,
	SEMMNU,
	SEMMSL,
	SEMOPM,
	SEMUME,
	SEMUSZ,
	SEMVMX,
	SEMAEM
};

#ifndef	IPC_ALLOC
#include	"sys/ipc.h"
#endif
#include	"sys/shm.h"
struct shmid_ds	shmem[SHMMNI];	
struct	shminfo shminfo = {
	SHMMAX,
	SHMMIN,
	SHMMNI,
	SHMSEG,
	0,	/* shmbrk not used on 3b20 */
	SHMALL
};

/*	Each process has 3 pregions (text, data, and stack) plus
 *	enough for the maximum number of shared memory segments
 *	plus the number of phys regions.
 *	We also need one extra null pregion to indicate the end
 *	of the list.
 */

int	pregpp = 3 + SHMSEG + NPHYS + 1;

/*
 * line discipline switch table
 */

extern nulldev();
extern	ttopen(), ttclose(), ttread(), ttwrite(), ttioctl(), ttin(), ttout();

extern	sxtin(), sxtout(), sxtrwrite();

struct linesw linesw[] = {

{ ttopen, ttclose, ttread, ttwrite, ttioctl, ttin, ttout, nulldev },

{ nulldev, nulldev, nulldev, sxtrwrite, nulldev, sxtin, sxtout, nulldev },
} ;


int	linecnt = (sizeof linesw/sizeof linesw[0]);

int	apgen = 0;

/* file and record locking */
#include <sys/flock.h>
struct	flckinfo flckinfo = {
	FLCKREC,
	FLCKFIL,
	0,
	0,
	0,
	0,
} ;

#ifndef AUTOCONFIG
struct	filock	flox[FLCKREC];		/* lock structures. */
struct	flino	flinotab[FLCKFIL];	/* inode to lock assoc. structures */
#else
struct	filock	*flox;			/* lock structures. */
struct	flino	*flinotab;		/* inode to lock assoc. structures */
#endif AUTOCONFIG

#undef	MFREE
#include <sys/mbuf.h>
#define	MCLBYTES	1024		/* same as mclbytes in uipc_mbuf.c */
#ifndef AUTOCONFIG
#if	NMBUFS > 0
struct mbuf	mbufbufs[NMBUFS + 1];
struct mbuf	mbutl[((NMBCLUSTERS * MCLBYTES) + MCLBYTES) / sizeof (struct mbuf)];
#endif
#else
struct mbuf	*mbufbufs;
struct mbuf	*mbutl;
#endif AUTOCONFIG

#ifndef AUTOCONFIG
#if	NPTY > 0
struct  tty	pts_tty[NPTY];
struct	pt_ioctl	pts_ioctl[NPTY];
#endif
#else AUTOCONFIG
struct  tty	*pts_tty;
struct	pt_ioctl	*pts_ioctl;
#endif AUTOCONFIG

#include	<sys/heap_kmem.h>
/*
 *	N.B.:	the definitions for mbufbufs[], mbutl[], and Core[] must occur
 *		in exactly that order.  Several macros depend on it.
 */
#ifndef AUTOCONFIG
unsigned long Core[(MAXCORE / sizeof(long)) + 1];
struct freehdr FreeHdr[MAXHEADER];
#else
unsigned long *Core;
struct freehdr *FreeHdr;
#endif AUTOCONFIG

/* <@(#)space.h	7.6.2.1> */
