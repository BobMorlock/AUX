/*	@(#)fdb.h	UniPlus VVV.2.1.2	*/
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

#define FDB_LISTEN	0x01		/* a listen completed */
#define FDB_TALK	0x02		/* a talk completed */
#define	FDB_EXISTS	0x04		/* a 'exists' completed */
#define	FDB_FLUSH	0x08		/* a flush completed */
#define	FDB_RESET	0x10		/* a reset completed */
#define	FDB_INT		0x20		/* an interrupt occured */
#define	FDB_POLL	0x40		/* a poll succeeded */
#define	FDB_UNINT	0x80		/* an int poll failed */

#define	F_RESET		0x00
#define	F_FLUSH		0x01
#define	F_LISTEN	0x08
#define	F_TALK		0x0c

#define FDB_KEYBOARD	2
#define FDB_MOUSE	3
