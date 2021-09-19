/*	@(#)intrmsg.c	UniPlus VVV.2.1.1	*/

#ifdef lint
#include "sys/sysinclude.h"
#else lint
#include "sys/types.h"
#endif lint

/*
 *	Print interrupt messages.  Called to print a message whenever
 *	wierd and wonderful things happen to the 3B-20.
 */


int callzero;
zerofunc()
{
	if(callzero)
		panic("call of function at location 0");
	else
		printf("call of function at location 0\n");
}

/* <@(#)intrmsg.c	6.1> */
