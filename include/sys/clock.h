/*
 * Zilog Z8036 Z8000 Z-CIO Counter/Timer and Parallel I/O Unit
 *
 * (C) 1984 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */


#define	CLK_ADDR 0xfffb0000	/* clock address */
#define	COUNT	33333		/* counter initial value (1/60 second) */
#define	CNT2	0xfffb0038	/* VERSAbus interrupt mask */

/* bit defines for Master Interrupt Control Register */
#define	RESET	0x01		/* reset Z8036 */
#define	ZERO	0x00		/* clear reset bit */
#define	MIE	0x80		/* Master Interrupt Enable */
#define	NV	0x20		/* No Vector Returned During IACK */
#define	RJA	0x02		/* Right Justified Address */
#define	DLC	0x40		/* Disable Lower Chain */

/* bit defines for Master Configuration Control Register */
#define	CT1E	0x40		/* Master Interrupt Enable */
#define PBE	0x80		/* Port B enable (VM04 compatibility) */
#define PAE	0x04		/* Port A enable (VM04 compatibility) */

/* bit defines for Counter/timer Mode Specification  Register */
#define	CSC	0x80		/* Continuous cycle */

/* bit defines for Counter/timer Command/Status  Register */
#define	CLIPIUS	0x20		/* Clear IP & IUS */
#define	GCB	0x04		/* Allow Countdown Sequence */
#define	TCB	0x02		/* Trigger Countdown Sequence */
#define	IE	0xc0		/* Interrupt Enable */

struct clk_map
{
	/* only registers relevant to counter 1 are defined here */

	char c_intctl;		/* master interrupt control register */
	char c_confctl;		/* master config control register */
	char fill1[2];
	char c_intvec;		/* counter interrupt vector */
	char fill2[5];
	char c_cs1;		/* counter 1 command/status */
	char fill3[5];
	unsigned short c_cc1;	/* counter 1 current count */
	char fill4[4];
	unsigned short c_tc1;	/* counter 1 time constant */
	char fill5[4];
	char c_mode1;		/* counter 1 mode specification */
};
