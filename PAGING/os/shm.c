/*	@(#)shm.c	UniPlus VVV.2.1.2	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#include "sys/mmu.h"
#include "sys/seg.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/ipc.h"
#include "sys/shm.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/systm.h"
#include "sys/sysmacros.h"
#include "sys/debug.h"
#endif lint

extern	struct	shmid_ds	shmem[];	/* shared memory headers */
extern	struct	shminfo		shminfo;	/* shared memory info structure */
extern struct timeval	time;			/* system idea of date */

struct	ipc_perm	*ipcget();
struct	shmid_ds	*shmconv();


/*
 * Shmat (attach shared segment) system call.
 */
shmat()
{
	register struct a {
		int	shmid;
		uint	addr;
		int	flag;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;
	register struct region		*rp;
	register struct pregion		*prp;
	int shmn;

	if ((sp = shmconv(uap->shmid)) == NULL)
		return;
	if (ipcaccess(&sp->shm_perm, SHM_R))
		return;
	if ((uap->flag & SHM_RDONLY) == 0)
		if (ipcaccess(&sp->shm_perm, SHM_W))
			return;
	for (prp = u.u_procp->p_region , shmn = 0; prp->p_reg; prp++)
		if (prp->p_type == PT_SHMEM)
			shmn++;
	if (shmn >= shminfo.shmseg) {
		u.u_error = EMFILE;
		return;
	}
	if (uap->flag & SHM_RND) {
		uap->addr = uap->addr & ~(SHMLBA - 1);
	} else
	if (uap->addr == 0) {
		if ((uap->addr = shmaddr(sp->shm_segsz)) == 0) {
			u.u_error = ENOMEM;
			return;
		}
	}
	rp = sp->shm_reg;
	reglock(rp);
	if (rp->r_pgsz  &&  chkpgrowth(rp->r_pgsz) < 0){
		regrele(rp);
		u.u_error = ENOMEM;
		return;
	}

	if ((prp = attachreg(rp, &u, (caddr_t)uap->addr, PT_SHMEM,
	   uap->flag & SHM_RDONLY? SEG_RO : SEG_RW)) == NULL) {
		regrele(rp);
		return;
	}
	if (sp->shm_perm.mode & SHM_INIT) {
		if (chkpgrowth(btop(sp->shm_segsz)) < 0  ||
		   growreg(prp, btop(sp->shm_segsz), DBD_DZERO) < 0) {
			detachreg(prp, &u);
			u.u_error = ENOMEM;
			return;
		}
		sp->shm_perm.mode &= ~SHM_INIT;
	}
	regrele(rp);
	u.u_rval1 = (int) prp->p_regva;
	sp->shm_atime = time.tv_sec;
	sp->shm_lpid = u.u_procp->p_pid;
}

/*
 * Convert user supplied shmid into a ptr to the associated
 * shared memory header.
 */
struct shmid_ds *
shmconv(s)
register int	s;	/* shmid */
{
	register struct shmid_ds	*sp;	/* ptr to associated header */

	if (s < 0)
	{
		u.u_error = EINVAL;
		return(NULL);
	}
	sp = &shmem[s % shminfo.shmmni];
	if (!(sp->shm_perm.mode & IPC_ALLOC)  
		|| s / shminfo.shmmni != sp->shm_perm.seq) {
		u.u_error = EINVAL;
		return(NULL);
	}
	return(sp);
}

/*
 * Shmctl system call.
 */
shmctl()
{
	register struct a {
		int		shmid,
				cmd;
		struct shmid_ds	*arg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct region		*rp;	/* shmem region */
	struct shmid_ds			ds;	/* hold area for IPC_SET */

	if ((sp = shmconv(uap->shmid)) == NULL)
		return;

	switch(uap->cmd) {

	/* Remove shared memory identifier. */
	case IPC_RMID:
		if (u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			&& !suser())
			return;
		rp = sp->shm_reg;
		sp->shm_reg = NULL;
		sp->shm_perm.mode = 0;
		sp->shm_segsz = 0;
		if (((int)(++(sp->shm_perm.seq) * shminfo.shmmni + (sp - shmem))) < 0)
			sp->shm_perm.seq = 0;
		reglock(rp);
		if (rp->r_refcnt == 0)
			freereg(rp);
		else {
			rp->r_flags &= ~RG_NOFREE;
			regrele(rp);
		}
		break;

	/* Set ownership and permissions. */
	case IPC_SET:
		if (u.u_uid != sp->shm_perm.uid && u.u_uid != sp->shm_perm.cuid
			 && !suser())
			 return;
		if (copyin(uap->arg, &ds, sizeof(ds))) {
			u.u_error = EFAULT;
			return;
		}
		sp->shm_perm.uid = ds.shm_perm.uid;
		sp->shm_perm.gid = ds.shm_perm.gid;
		sp->shm_perm.mode = (ds.shm_perm.mode & 0777) |
			(sp->shm_perm.mode & ~0777);
		sp->shm_ctime = time.tv_sec;
		break;

	/* Get shared memory data structure. */
	case IPC_STAT:
		if (ipcaccess(&sp->shm_perm, SHM_R))
			return;

		/*	The following is needed because
		**	a user can look at it.  In
		**	particular, the regression tests
		**	require it.
		*/

		sp->shm_nattch = sp->shm_reg->r_refcnt;
		sp->shm_cnattch = sp->shm_nattch;

		if (copyout(sp, uap->arg, sizeof(*sp)))
			u.u_error = EFAULT;
		break;

	/* Lock segment in memory */
	case SHM_LOCK:
		if (!suser())
			return;

		rp = sp->shm_reg;
		reglock(rp);
		rp->r_flags |= RG_NOSWAP;
		regrele(rp);
		break;

	/* Unlock segment */
	case SHM_UNLOCK:
		if (!suser())
			return;

		rp = sp->shm_reg;
		reglock(rp);
		rp->r_flags &= ~RG_NOSWAP;
		regrele(rp);
		break;

	default:
		u.u_error = EINVAL;
	}
}

/*
 * Detach shared memory segment
 */
shmdt()
{
	register struct a {
		caddr_t	addr;
	} *uap = (struct a *)u.u_ap;
	register struct pregion	*prp;
	register struct region	*rp;
	register struct shmid_ds *sp;

	/*
	 * Find matching shmem address in process region list
	 */
	for (prp = u.u_procp->p_region; prp->p_reg; prp++)
		if (prp->p_type == PT_SHMEM && prp->p_regva == uap->addr)
			break;
	if (prp->p_reg == NULL) {
		u.u_error = EINVAL;
		return;
	}
	/*
	 * Detach region from process
	 * We must remember rp here since detach clobbers p_reg
	 */
	rp = prp->p_reg;
	reglock(rp);
	detachreg(prp,&u);

	clratb(USRATB);

	/*
	 * Find shmem region in system wide table.  Update detach time
	 * and pid, and free if appropriate
	 */
	for (sp = shmem; sp < &shmem[shminfo.shmmni]; sp++)
		if (sp->shm_reg == rp)
			break;
	if (sp >= &shmem[shminfo.shmmni])
		return;	/* shmem has been removed already */
	sp->shm_dtime = time.tv_sec;
	sp->shm_lpid = u.u_procp->p_pid;
}

/*
 * Exec, exit, and fork subroutines not needed any longer
 * There function is implemented gratis by normal region handling
 */
shmexec() { }
shmexit() { }
shmfork() { }

/*
 * Shmget (create new shmem) system call.
 */
shmget()
{
	register struct a {
		key_t	key;
		uint	size,
			shmflg;
	}	*uap = (struct a *)u.u_ap;
	register struct shmid_ds	*sp;	/* shared memory header ptr */
	register struct region		*rp;	/* shared memory region ptr */
	int				s;	/* ipcget status */
	int shmipcget();

	if ((sp = (struct shmid_ds *)
		ipcget(uap->key, (int)uap->shmflg, shmipcget, &s)) == NULL)
		return;
	if (s) {

		/*
		 * This is a new shared memory segment.
		 * Allocate a region and init shmem table
		 */
		if (uap->size < shminfo.shmmin || uap->size > shminfo.shmmax) {
			u.u_error = EINVAL;
			sp->shm_perm.mode = 0;
			return;
		}
		if ((rp = allocreg((struct inode *)NULL, RT_SHMEM)) == NULL) {
			sp->shm_perm.mode = 0;
			return;
		}
		sp->shm_perm.mode |= SHM_INIT;	/* grow on first attach */
		sp->shm_segsz = uap->size;
		sp->shm_reg = rp;
		sp->shm_atime = sp->shm_dtime = 0;
		sp->shm_ctime = time.tv_sec;
		sp->shm_lpid = 0;
		sp->shm_cpid = u.u_procp->p_pid;
		rp->r_flags |= RG_NOFREE;
		regrele(rp);
	} else {
		/*
		 * Found an existing segment.  Check size
		 */
		if (uap->size && uap->size > sp->shm_segsz) {
			u.u_error = EINVAL;
			return;
		}
	}

	u.u_rval1 = sp->shm_perm.seq * shminfo.shmmni + (sp - shmem);
}

/*
	routine to return perm info to ipcget
*/
shmipcget(i,ip)
register int i;
register struct ipc_perm **ip;
{
	if (i < 0 || i >= shminfo.shmmni)
		return(0);
	*ip = &shmem[i].shm_perm;
	return(1);
}

/*
 * System entry point for shmat, shmctl, shmdt, and shmget system calls.
 */
shmsys()
{
	register struct a {
		uint	id;
	}	*uap = (struct a *)u.u_ap;
	int		shmat(),
			shmctl(),
			shmdt(),
			shmget();
	static int	(*calls[])() = {shmat, shmctl, shmdt, shmget};

	if (uap->id > 3) {
		u.u_error = EINVAL;
		return;
	}
	u.u_ap = &u.u_arg[1];
	(*calls[uap->id])();
}

/*
 * Select attach address based on segment size
 */
shmaddr(sz)
{
	register vaddr;
	register preg_t	*prp;
	register preg_t	*oprp1;
	register preg_t	*oprp2;

	oprp1 = NULL;
	oprp2 = NULL;
	vaddr = -1;
	for (prp = u.u_procp->p_region ; prp->p_reg ; prp++) {
		if (prp->p_type == PT_SHMEM || prp->p_type == PT_DATA)
			if ((oprp1 == NULL || prp->p_regva > oprp1->p_regva)) 
				oprp1 = prp;
	}
	if (oprp1 == NULL)
		return(0);
	for (prp = u.u_procp->p_region ; prp->p_reg ; prp++) {
		if (prp->p_regva > oprp1->p_regva)
			if ((oprp2 == NULL || prp->p_regva < oprp2->p_regva)) 
				oprp2 = prp;
	}

	vaddr = (int)(stob(ptos(oprp1->p_reg->r_pgsz)) + oprp1->p_regva);
	if ((oprp2->p_regva - (caddr_t)vaddr) < sz)
		vaddr = 0;
	return (vaddr);
}

/* <@(#)shm.c	6.3> */
