/*	@(#)genassym.c	UniPlus 2.1.2	*/
#include <sys/param.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#include <sys/signal.h>
#include <sys/errno.h>
#include <sys/dir.h>
#include <sys/mmu.h>
#include <sys/time.h>
#include <sys/page.h>
#include <sys/seg.h>
#include <sys/user.h>
#include <sys/reg.h>


main()
{
	register struct sysinfo *s = (struct sysinfo *)0;
	register struct user *u = (struct user *)0;

	printf("\tglobal\tsysinfo\n");
	printf("\tset\tV_INTR%%,%d\n", &s->intr);
	printf("\tset\tU_USER%%,%d\n", &u->u_user[0]);
        exit(0);        /* sai -6/17/85- Vax make blows up without this */
}
