#include	<sys/types.h>
#include	<sys/sysinfo.h>

main()
{
	register struct sysinfo *s = (struct sysinfo *)0;

	printf("\tglobal\tsysinfo\n");
	printf("\tset\tV_INTR%%,%d\n", &s->intr);
        exit(0);        /* sai -6/17/85- Vax make blows up without this */
}
