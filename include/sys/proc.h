/* NFSSRC @(#)proc.h	2.1 86/04/11 */
/*	@(#)proc.h	UniPlus VVV.2.1.6	*/

/*	One structure allocated per active process. It contains all
 *	data needed about the process while the process may be swapped
 *	out.  Other per process data (user.h) may swapped with the
 *	process but in fact it is not.
 */

typedef struct	proc	{
	uint		p_flag;		/* Flags defined below.	     */
	char		p_stat;		/* Status of process.	     */
	char		p_pri;		/* priority, negative is high */
	char		p_cpu;		/* cpu usage for scheduling */
	char		p_nice;		/* nice for cpu usage */
	char		p_time;		/* resident time for scheduling */
	caddr_t 	p_wchan;	/* event process is awaiting */

	char    	p_cursig;
	int		p_sig;		/* signals pending to this process */
	int     	p_sigmask;      /* current signal mask (BSD) */
	int     	p_sigignore;    /* signals being ignored (BSD) */
	int     	p_sigcatch;     /* signals being caught by user (BSD) */

	ushort		p_uid;		/* real user id */
	ushort		p_suid;		/* effective user id */
	short		p_pgrp;		/* name of process group leader */
	short		p_pid;		/* unique process id */
	short		p_ppid;		/* process id of parent */

	struct	proc	*p_link;	/* linked list of processes */
	struct	proc	*p_parent;	/* ptr to parent process    */
#define	p_pptr		p_parent
	struct	proc	*p_child;	/* ptr to first child process */
	struct	proc	*p_sibling;	/* ptr to next sibling	    */

	pte_t		p_uptbl[USIZE];	/* u_area page table - see  start.s */
        int     	p_sbr;		/* process segment base register */
	short		p_size;		/* size of swappable image in pages */
	ste_t		p_addr;		/* segment table entry for uarea */
	preg_t		*p_region;	/* process regions */

	time_t		p_utime;	/* user time, this proc */
	time_t		p_stime;	/* system time, this proc */
	struct itimerval p_realtimer;

	short		p_xstat;	/* exit status for wait */
	int		p_clktim;	/* time to alarm clock signal */

	long		p_compatflags;	/* compatibility mode flags */
} proc_t;

#ifndef AUTOCONFIG
extern struct proc proc[];		/* the proc table itself */
#else AUTOCONFIG
extern struct proc *proc;		/* the proc table itself */
#endif AUTOCONFIG

/* stat codes */
#define	SSLEEP	1		/* Awaiting an event.		*/
#define	SRUN	2		/* Running.			*/
#define	SZOMB	3		/* Process terminated but not	*/
				/* waited for.			*/
#define	SSTOP	4		/* Process stopped by signal	*/
				/* since it is being traced by	*/
				/* its parent.			*/
#define	SIDL	5		/* Intermediate state in	*/
				/* process creation.		*/
#define	SONPROC	6		/* Process is being run on a	*/
				/* processor.			*/
#define SXBRK	7		/* process being xswapped       */

/* flag codes */
#define	SSYS	0x0001		/* System (resident) process.	*/
#define	STRC	0x0002		/* Process is being traced.	*/
#define	SWTED	0x0004		/* Stopped process has been	*/
				/* given to parent by wait	*/
				/* system call.  Don't return	*/
				/* this process to parent again	*/
				/* until it runs first.		*/
#define SNWAKE	0x0008		/* Process cannot wakeup by	*/
				/* a signal.			*/
#define SLOAD	0x0010		/* in core                      */
#define SLOCK   0x0020		/* have mem_lock locked 	*/

#define SPGRPL	0x0100		/* login-type process group leader */
#define SPAGEIN	0x0200		/* currently trying to fault in a page */
#define SCOFF	0x0400		/* COFF binary format */
#define SSEL    0x1000          /* selecting, cleared if rescan needed */
#define STIMO   0x2000          /* timing out during sleep */
#define	SPGRP42	0x4000		/* 4.2 style process groups for job control */
#define	SOMASK	0x8000		/* restore old mask after taking signal */
