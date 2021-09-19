/* NFSSRC @(#)user.h	2.1 86/04/11 */
/*	@(#)user.h	UniPlus VVV.2.1.5	*/
#include	<sys/resource.h>

/*	A temporary duplicate of aouthdr.h
*/

/* 68k version */
typedef struct ufhd {		/* header of executable file */
	short	ux_mag;		/* magic number */
	short	ux_stamp;	/* stamp */
	unsigned ux_tsize;	/* text size */
	unsigned ux_dsize;	/* data size */
	unsigned ux_bsize;	/* bss size */
	unsigned ux_entloc;	/* entry location */
	unsigned ux_unused;
	unsigned ux_relflg;
	unsigned ux_ssize;	/* symbol table size */
} ufhd_t;
#define	ux_txtorg	ux_unused
#define	ux_datorg	ux_relflg
#define	ux_tstart	ux_ssize

#define	PSARGSZ 40 		/* size of arguments saved for ps */
#define	COMMSIZ	14		/* size of comm field (== SVFSDIRSIZ) */
#define	USERSIZ	4		/* size of area reserved for OEM/user use */

/*
 *	The User Structure.
 *	One allocated per process.
 *	Contains all per process data that doesn't need to be referenced
 *	while the process is swapped.
 *	The user block is USIZE pages long; resides at a
 *	fixed kernel virtual address;
 *	contains the system stack per user; is cross referenced
 *	with the proc structure for the same process.
 */
 
typedef struct	user	{
	ste_t 	u_stbl[NSEGP];		/* process segment table     */
	struct	proc *u_procp;		/* pointer to proc structure */
	label_t	u_qsav;			/* premature sys call return */
	label_t	u_rsav;			/* save info when exchanging stacks */
	label_t	u_ssav;			/* label variable for swapping */
	int	*u_nofault;		/* ``nofault'' save area */
	int	u_utilid;		/* utility ID for field test */
					/* set and fts command.	     */
	int	u_arg[10];		/* arguments to system call */
	int	*u_ap;			/* pointer to arglist */
	union	 {			/* syscall return values */
		struct	{
			int	r_val1;
			int	r_val2;
		}r_reg;
		off_t	r_off;
		time_t	r_time;
	} u_r;
	short	u_errcnt;		/* syscall error count */
	char	u_error;		/* return error code */
	char	u_eosys;		/* special action on end of syscall */

	short	u_fmode;		/* file mode for IO */
	ushort	u_pbsize;		/* bytes in block for IO */
	ushort	u_pboff;		/* offset in block for IO */
	dev_t	u_pbdev;		/* real device for IO */
	daddr_t	u_rablock;		/* read ahead block address */

	struct vnode *u_cdir;		/* ptr to vnode of current directory */
	struct vnode *u_rdir;		/* root directory of current process */
	caddr_t	u_dirp;			/* pathname pointer */
	struct vnode *u_pdir;		/* vnode of parent directory of dirp */
					/* of dirp		     */
	struct	file *u_ofile[NOFILE];	/* file structures of open   */
					/* files		     */
	char	u_pofile[NOFILE];	/* per-process flags of open files */
#define	UF_EXCLOSE 	0x1		/* auto-close on exec */

	uint	u_tsize;		/* text size (pages) */
	uint	u_datorg;		/* start virtual address for data */
	uint	u_dsize;		/* data size (pages) */
	uint	u_ssize;		/* stack size (pages) */

	int	(*u_signal[NSIG])();	/* disposition of signals */
	int	u_sigmask[NSIG];        /* signals to be blocked */
	int	u_sigonstack;		/* signals to take on sigstack */
	int	u_oldmask;		/* saved mask from before sigpause */
	struct	sigstack u_sigstack;	/* sp & on stack state variable */
	int	u_code;			/* ``code'' to trap */
	short	u_traptype;		/* type of last trap for m68k w/COFF */
	int	(*u_sigcode)();		/* pointer to user sigcode routine
					   for 4.2 style signals */
#define	u_onstack       u_sigstack.ss_onstack
#define	u_sigsp         u_sigstack.ss_sp

	time_t	u_utime;		/* this process user time */
	time_t	u_stime;		/* this process system time */
	time_t	u_cutime;		/* sum of childs' utimes */
	time_t	u_cstime;		/* sum of childs' stimes */
	struct itimerval u_timer[3];

	int	*u_ar0;			/* address of users saved R0 */

	struct ucred *u_cred;		/* user credentials (uid, gid, etc) */
#define	u_uid		u_cred->cr_uid
#define	u_gid		u_cred->cr_gid
#define	u_groups	u_cred->cr_groups
#define	u_ruid		u_cred->cr_ruid
#define	u_rgid		u_cred->cr_rgid

	struct	{			/* profile arguments */
		short	*pr_base;	/* buffer base */
		uint	pr_size;	/* buffer size */
		uint	pr_off;		/* pc offset */
		uint	pr_scale;	/* pc scaling */
	} u_prof;

	short	*u_ttyp;		/* pointer to pgrp in "tty" structure */
	dev_t	u_ttyd;			/* controlling tty dev */

	ufhd_t  u_exdata;
	char	u_comm[COMMSIZ];
	time_t	u_start;
	time_t	u_ticks;

	long	u_mem;
	long	u_ior;
	long	u_iow;
	long	u_iosw;
	long	u_ioch;
	char	u_acflag;

	daddr_t	u_limit;		/* maximum write address */
        struct  rlimit u_rlimit[RLIM_NLIMITS];

	short	u_cmask;		/* mask for file creation */
	char	u_psargs[PSARGSZ];	/* command arg string for ps */
	short	u_fcode;		/* function code on bus errors */
	long	u_aaddr;		/* access address on bus errors */
	short	u_ireg;			/* instruction register on bus errors*/
	struct phys {
		int	u_phladdr;	/* phys logical address */
		int	u_phsize;	/* phys size */
		int	u_phpaddr;	/* phys physical address */
		struct pregion	*u_phprp; /* process region for phys */
	} u_phys[NPHYS];
	short	u_lock;			/* process locking */
	int	u_fixflt[2];		/* On a protection or	*/
					/* valid fault, may	*/
					/* contain data to fix	*/
					/* up faulting page.  	*/
					/* See fault.c		*/
#ifdef mc68881
	char	u_fpsaved;		/* fp unit state saved */
	char	u_fpexc;		/* last exception which caused SIGFPE */
	char	u_fpfill[2];		/* unused */
	long	u_fpsysreg[3];		/* system regs CONTROL/STATUS/IADDR */
#define	FPDSZ	12			/* size of fp data reg FP0-FP7 */
	char	u_fpdreg[8][FPDSZ];	/* data regs FP0,FP1,...,FP7 */
	char	u_fpstate[184];		/* internal state, FSAVE/FRESTORE */
#endif mc68881

	long	u_user[USERSIZ];	/* reserved for OEM/user use */
	int	u_stack[1];
					/* kernel stack per user extends
					 * from u.u_stack upwards,
					 * not to reach u + ptob(USIZE).
					 */
} user_t;

extern struct user u;

#define	u_ste		u.u_fixflt[0]
#define	u_faddr		u.u_fixflt[1]
#define	u_rval1		u_r.r_reg.r_val1
#define	u_rval2		u_r.r_reg.r_val2
#define	u_roff		u_r.r_off
#define	u_rtime		u_r.r_time

/*
 * Flag bits for u_profile
 */

#define	EXCLOSE 01	/* close on exec flag */

/* u_eosys values */
#define	JUSTRETURN	0
#define	RESTARTSYS	1
#define	SIMULATERTI	2
#define	REALLYRETURN	3

#define	TRAPNORM	0		/* "normal" trap */
#define	TRAPBUS		1		/* bus error trap */
#define	TRAPADDR	2		/* address error trap */
#define	TRAPLONG	0x8000		/* long buserr frame - 68010 && 68020 */
#define	TRAPSHORT	0x4000		/* short buserr frame - 68020 only */

struct ucred {
	u_short	cr_ref;			/* reference count */
	short   cr_uid;			/* effective user id */
	short   cr_gid;			/* effective group id */
	int     cr_groups[NGROUPS];	/* groups, 0 terminated */
	short   cr_ruid;		/* real user id */
	short   cr_rgid;		/* real group id */
};
#ifdef KERNEL
#define	crhold(cr)	(cr)->cr_ref++
struct ucred *crget();
struct ucred *crcopy();
struct ucred *crdup();
#endif
