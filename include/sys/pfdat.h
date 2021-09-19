/*	@(#)pfdat.h	UniPlus VVV.2.1.1	*/

typedef struct pfdat {
	unsigned	pf_blkno : 24,	/* Disk block nummber.	*/
			pf_flags : 8;	/* page flags		*/
	cnt_t		pf_use;		/* share use count	*/
	u_int		pf_dev;		/* Disk device code.	*/
	char		pf_swpi;	/* Index into swaptab.	*/
	char		pf_rawcnt;	/* Cnt of processes	*/
					/* doing raw I/O to 	*/
					/* page.		*/
	short		pf_waitcnt;	/* Number of processes	*/
					/* waiting for PG_DONE	*/
	struct pfdat	*pf_next;	/* Next free pfdat.	*/
	struct pfdat	*pf_prev;	/* Previous free pfdat.	*/
	struct pfdat	*pf_hchain;	/* Hash chain link.	*/
} pfd_t;

#define	P_QUEUE		0x01	/* Page on free queue		*/
#define	P_BAD		0x02	/* Bad page (parity error, etc.)*/
#define	P_HASH		0x04	/* Page on hash queue		*/
#define P_DONE		0x08	/* I/O to read page is done	*/
#define	P_SWAP		0x10	/* Page on swap (not file).	*/

extern char mem_lock;
extern struct pfdat phead;
extern struct pfdat *pfdat;
extern struct pfdat *phash;
extern struct pfdat ptfree;
extern int phashmask;
#ifndef ORIG3B20
extern struct pfdat	*pagefind();
#else ORIG3B20
extern struct pfdat	*pfind();
#endif ORIG3B20


#define BLKNULL		0	/* pf_blkno null value		*/

#ifdef NOTDEFASA
THESE WILL EITHER BE FROM page.c OR WE ELIMINATE memlock/memunlock
#define memlock()			\
{					\
	u.u_procp->p_flag |= SLOCK;	\
	while (mem_lock)		\
		sleep(&mem_lock, PZERO);\
	mem_lock = 1;			\
}
#define memunlock()			\
{					\
	ASSERT(mem_lock);		\
	wakeup(&mem_lock);		\
	mem_lock = 0;			\
	u.u_procp->p_flag &= ~SLOCK;	\
}
#endif NOTDEFASA

