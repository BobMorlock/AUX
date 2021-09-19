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

extern char mouse_button[];
extern short mouse_x[];
extern short mouse_y[];

#define MOUSE_OPEN	0
#define MOUSE_CHANGE	1

#define MOUSE_BUTTON	(('M'<<8)|1)

#define MOUSE_OP_DELAY	0
#define MOUSE_OP_MODE	1
#define MOUSE_OP_INTR	2
#define MOUSE_OP_OPEN	3
