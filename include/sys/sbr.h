/*	@(#)sbr.h	UniPlus VVV.2.1.1	*/
/*
 *	Segment Base Register
 */

#define S_SEG		0xFE000000	/* highest equipped segment */
#define S_MISC		0x01C00000	/* miscellaneous software bits */
#define S_STBL		0x003FFFFF	/* word address of segment table */
#define S_SEGSHFT	25		/* shift for stbl field */

#define mksbr(s,p)	(((unsigned)(p) >> BPWSHFT) | (((s) - 1) << S_SEGSHFT))
