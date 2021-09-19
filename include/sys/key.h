/*	@(#)key.h	UniPlus VVV.2.1.3	*/
/*
 *
 *	Copyright 1986 Unisoft Corporation of Berkeley CA
 *
 *
 *	UniPlus Source Code. This program is proprietary
 *	with Unisoft Corporation and is not to be reproduced
 *	or used in any manner except as authorized in
 *	writing by Unisoft.
 *
 */

/*
 *	Default wait before repeating (in KEY_DELAY units)
 */

#define KEY_DEFWAIT	HZ/2

/*
 *	Default gap between repeated characters (in KEY_DELAY units)
 */

#define KEY_DEFGAP	HZ/20

/*
 *	Interrupt types
 */

#define KC_OPEN		0
#define KC_RAW0		1
#define KC_CHAR		2
#define KC_RAW2		3

/*
 *	Internal key stuff
 */

#define KEY_OPEN	1
#define KEY_CLOSED	0
#define KEY_TIMEOUT	1

/*
 *	Modes to be passed to key_open
 */

#define KEY_ASCII	0	/* ascii mode ... with timeouts */
#define KEY_RAW		1	/* raw key codes ... when something changes */
#define KEY_ARAW	2	/* raw key codes ... with KEY_ASCII interface */

/*
 *	Request parameters to key_op
 */

#define	KEY_OP_KEYPAD	0	/* put the keyboard driver into keypad mode */
#define	KEY_OP_MODE	1	/* change from ascii <-> raw modes */
#define	KEY_OP_WAIT	2	/* change the wait before repeat */
#define	KEY_OP_GAP	3	/* change the repeat rate */
#define	KEY_OP_DELAY	4	/* change the keyboard poll delay */
#define	KEY_OP_RESTART	5	/* restart the state machine */
#define	KEY_OP_INTR	6	/* change the interrupt routine (return */
				/*	the old one) */
#define	KEY_OP_OPEN	7	/* is it already open? */

/*
 *	In Raw mode ... copies of the latest raw keyboard registers
 */

extern short key_r0[];
extern short key_r1[];

/*
 *	Register r1's bit fields
 */

#define	KEY_R1_OAPPLE	0x0100
#define	KEY_R1_CAPPLE	0x0200
#define	KEY_R1_SHIFT	0x0400
#define	KEY_R1_CONTROL	0x0800
#define	KEY_R1_RESET	0x1000
#define	KEY_R1_CAPSLOCK	0x2000
#define	KEY_R1_DELETE	0x4000
#define	KEY_R1_OPTION	KEY_R1_CAPPLE

/*
 *	Key families from the keypad maps
 */

#define	KEY_VALUE	0x1f
#define	KEY_TYPE	0xe0

#define	KEY_EMPTY	0x00
#define	KEY_PLAIN	0x20
#define	KEY_CAPS	0x40
#define	KEY_SHIFT	0x60
#define KEY_CONTROL	0x80
#define	KEY_SCONTROL	0xA0
#define	KEY_SPECIAL	0xC0
#define	KEY_KEYPAD	0xE0

/*
 *	The following is for compatability
 */
#ifndef DVT
#define	KEY_DELAY	1
#endif
