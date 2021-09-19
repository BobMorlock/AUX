/*	@(#)sysent.h	UniPlus VVV.2.1.1	*/
/*
 *	System Entry Table
 */

struct	sysent	{
#ifndef ORIG3B20
	short	sy_narg;		/* total number of arguments */
	short	sy_qsav;		/* 1 if you need qsave(),	*/
#else ORIG3B20
	char	sy_narg;		/* total number of arguments */
	char	sy_nrarg;		/* number of args in registers	*/
	char	sy_stjmp;		/* 1 if you need setjump,	*/
#endif ORIG3B20
					/* 0 otherwise			*/
	int	(*sy_call)();		/* handler */
} ;

extern	struct	sysent	sysent[];
