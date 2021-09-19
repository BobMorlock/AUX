/* NFSSRC @(#)systm.h	2.1 86/04/11 */
/*	@(#)systm.h	UniPlus VVV.2.1.1	*/
/* Random set of variables used by more than one routine.
 */

extern struct proc *runq;	/* head of list of running processes */
extern int cputype;		/* type of cpu */
extern time_t lbolt;		/* time in HZ since last boot */
extern struct timeval time;	/* time in sec from 1970 */
extern	pte_t	*iduptbl;	/* dummy u_area when processor idle */
extern	pte_t	*uptbl;		/* page table for u_area */

extern int runrun;		/* scheduling flag */
extern int curpri;		/* current priority */

extern int	maxmem;		/* max available memory  */
extern int	physmem;	/* physical memory on this CPU */
extern daddr_t	swaplow;	/* block number of swap space */
extern int	swapcnt;	/* size of swap space in blocks */
extern dev_t	rootdev;	/* device of the root */
extern dev_t	swapdev;	/* swapping device */
extern struct vnode *swapdev_vp;	/* vnode equivalent to above */
extern dev_t	pipedev;	/* pipe device */
extern struct vnode *pipedev_vp;	/* vnode equivalent to above */
extern	struct	mount	*pipemnt;	/* mnt entry for pipedev */
extern dev_t	dumpdev;	/* dump device */
extern char	*panicstr;	/* panic string pointer */
extern int	blkacty;	/* active block devices */
extern int	selwait;
extern	pwr_cnt, pwr_act;
extern int	(*pwr_clr[])();

dev_t		getmdev();
daddr_t		bmap();
struct inode	*ialloc();
struct inode	*iget();
struct inode	*owner();
struct inode	*maknode();
struct inode	*namei();
struct buf	*alloc();
struct buf	*getblk();
struct buf	*geteblk();
struct buf	*bread();
struct buf	*breada();
struct file	*getf();
struct file	*falloc();
int		uchar();
caddr_t		kmem_alloc();
struct vnode	*devtovp();

extern 	char  runin;		/* scheduling flag */
extern  char  runout;		/* scheduling flag */
extern	struct	inode uinode;	/* lock on sync */

extern	sspeed;			/* default console speed */
extern	short icode[];		/* user init code */
extern	szicode;		/* init code size */
extern	char regloc[];		/* locs of saved user registers (trap.c) */
extern	char	hostname[];
extern	int	hostnamelen;
int	selwait;			/* wchan for select waits */
extern struct proc	*pfind();
extern struct file	*getsock();

#define	nofault	u.u_nofault	/* kludge for backward compatibility */
