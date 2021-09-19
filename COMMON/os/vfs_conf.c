/*	@(#)vfs_conf.c 1.1 86/02/03 SMI	*/
/*	@(#)vfs_conf.c	2.1 86/04/15 NFSSRC */

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/vfs.h"
#undef	NFS
#include "sys/mount.h"

int	novfs_entry();
struct vfsops novfs_vfsops = {
	novfs_entry,
	novfs_entry,
	novfs_entry,
	novfs_entry,
	novfs_entry,
};
extern	struct vfsops svfs_vfsops;	/* XXX Should be ifdefed */

struct vfsops *vfssw[MOUNT_MAXTYPE + 1] = {
	&svfs_vfsops,		/* MOUNT_UFS */
	&novfs_vfsops,		/* MOUNT_NFS */
	&novfs_vfsops,		/* MOUNT_PC (== MOUNT_MAXTYPE) */
};

novfs_entry()
{
	return (ENODEV);
}
