/*	@(#)shl.h	UniPlus 2.1.1	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#define NLAYERS		8

/*
 *	One of these per terminal
 */

struct shlr {
	queue_t			*sh_q;			/* the original queue */
	queue_t			*sh_layer[NLAYERS];	/* the downward layers*/
	mblk_t			*sh_wait;		/* waiting ioctl */
	int 			sh_pid;			/* owner pid */
	short			sh_current;		/* current layer */
	unsigned char		sh_flag;		/* flag */
	char			sh_swtch;		/* switch character */
};

#define SH_FREE		0
#define	SH_USED		1

/*
 *	master ioctls
 */

#define SH_SET		(('}'<<8)|0)

/*
 *	One of these per 'layer'
 */

struct shl {
	struct shlr	*s_data;
	mblk_t		*s_m;
	short		s_pgrp;
	short		s_state;
	short		s_lflag;
	short		s_iflag;
	short		s_oflag;
	short		s_cflag;
	char		s_cc[NCC];
	char		s_info;
	unsigned char	s_ind;
	unsigned char	s_slot;
};

#define S_FREE		0
#define S_CLOSING	1
#define	S_BLOCKED	2
#define	S_RUNNING	3

extern struct shl shl_data[];
extern int shl_count;
extern struct shlr shlr_data[];
extern int shlr_count;
