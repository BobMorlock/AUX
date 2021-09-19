#include "sys/sysinclude.h"

/*
 * Definitions of system specific kernel variables to keep lint happy
 */

char	SyncTbl[100];
char	TagData[100];
int	mmu_on = 0;

idle()
{
	parityenable();
	scintr((struct args *)NULL);
	via1intr((struct args *)NULL);
	via2intr((struct args *)NULL);
	netintr();
	abintr();
#ifdef UCB_NET
	{struct uba_driver *udp = (struct uba_driver *)0; if ((int)udp) return;}
	lrintr();
	netintr();
	(void) getsock(0);
#endif UCB_NET
#ifdef FLOAT
	fpintr();
	mnkintr();
#endif
}

int FormatTrack(i) {return(i);}

int snrd(addr)
  caddr_t addr; {return((int)addr);}

int snwr(addr)
  caddr_t addr; {return((int)addr);}

int rdhead(addr)
  caddr_t addr; {return((int)addr);}

struct file *getsock(fdes)
  int fdes; {return((struct file *)fdes);}
