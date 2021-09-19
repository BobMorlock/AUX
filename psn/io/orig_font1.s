/*	@(#)font1.s	UniPlus VVV.2.1.2	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */
 #
 #	Warning!!! ----> this only runs on 68020s ......
 #

#define	D(x, y)	global font_debugd ; \
		movm.l	&0xc0c0, -(%sp) ;\
		mov.l	y, -(%sp) ;\
		mov.l	&x, -(%sp) ;\
		jsr	font_debugd ;\
		addq.l	&8, %sp ;\
		movm.l	(%sp)+, &0x0303
	
#define	A(x, y)	global font_debuga ; \
		movm.l	&0xc0c0, -(%sp) ;\
		mov.l	y, -(%sp) ;\
		mov.l	&x, -(%sp) ;\
		jsr	font_debuga ;\
		addq.l	&8, %sp ;\
		movm.l	(%sp)+, &0x0303
	
#define	ffp	8(%fp)
#define	x	12(%fp)
#define	y	16(%fp)
#define	c	20(%fp)

#define firstChar	0x02
#define lastChar	0x04


#define	font_pnt 	0x00
#define	font_height 	0x04
#define	font_width 	0x08
#define	font_leading 	0x0c
#define	font_rowwords 	0x10
#define	font_bitimage 	0x14
#define	font_loctable 	0x18
#define	font_owtable 	0x1c
#define	font_linewidth 	0x20
#define	font_maxx 	0x24
#define	font_maxy 	0x28
#define	font_inverse 	0x2c
#define	font_screen 	0x30
#define	font_theight 	0x34


#define	video_addr	0x00
#define	video_mem_x	0x04
#define	video_mem_y	0x08
#define	video_scr_x	0x0c
#define	video_scr_y	0x10

	global font_char
font_char:
	link	%fp, &0
	movm.l	&0x3ff8, -(%sp)
	mov.l	ffp, %a3		/* font pointer */
	mov.l	font_pnt(%a3), %a2	/* fontrec pointer */
	mov.l	c, %d0			/* character to output */
	cmp.w	%d0, firstChar(%a2)	/* make sure it is in range */
	bge	L1
bad:	clr.l	%d0			/* return a 0 on an error */
	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts
L1:	cmp.w	%d0, lastChar(%a2)
	bgt	bad
	mov.l	x, %d1			/* now test the x position for
					   validity */
	blt	bad
	cmp.l	%d1, font_maxx(%a3)
	bge	bad
	mov.l	y, %d2			/* and then the y */
	blt	bad
	cmp.l	%d2, font_maxy(%a3)
	bge	bad
	sub.w	firstChar(%a2), %d0	/* adjust the character index */
	mov.l	font_loctable(%a3), %a2	/* find the font offset */
	lea	(0,%a2,%d0.w*2), %a2
	clr.l	%d3
	mov.w	(%a2), %d3
	clr.l	%d4			/* get the width (if 0 return) */
	mov.w	2(%a2), %d4
	sub.l	%d3, %d4
	bgt	cnt			/* only valid 0 length is ' ' */
	blt	bad
	mov.l	c, %d0
	cmp.b	%d0, &0x20
	bne	bad
cnt:	mov.l	font_width(%a3), %d7	/* get the output width */
	mulu.w	%d7, %d1		/* get the screen index */
	mulu.l	font_linewidth(%a3), %d2
	add.l	%d1, %d2
	mov.l	font_screen(%a3), %a4	/* get the screen base */
	mov.l	video_mem_x(%a4), %a2	/* get the line increment */
	mov.l	video_addr(%a4), %a4
	mov.l	font_rowwords(%a3), %a1	/* font row width */
	mov.l	font_bitimage(%a3), %a0	/* font info pointer */
	mov.l	font_inverse(%a3), %d5
	mov.l	%d7, %d6		/* get the center offset */
	sub.l	%d4, %d6
	blt	chk
	lsr.l	&1, %d6
	bra	cont
chk:	clr.l	%d6
cont:	mov.l	font_height(%a3), %d1	/* loop for every line */
	subq.l	&1, %d1
 	tst.l	%d4
	beq	blank			/* blank is a special case */
loop:
		bfextu	(%a0){%d3:%d4}, %d0 /* get the character row */
		add.l	%a1, %d3	/* increment the row index */
 		lsl.l	%d6, %d0	/* center the character */
		tst.b	%d5
		bne	inv
		not.l	%d0		/* invert it if required */
inv:
		bfins	%d0, (%a4){%d2:%d7} /* insert it in the bitmap */
		add.l	%a2, %d2	/* move to the next line */
	dbra	%d1, loop		/* loop through each row */
	mov.l	font_leading(%a3), %d1	/* do the last empty rows */
	beq	xit
blankit:
	sub.l	&1, %d1
	tst.b	%d5			/* get the fill value */
	bne	invL
	mov.l	&0xffffffff, %d0
	br	loopL
invL:
	clr.l	%d0
loopL:
		bfins	%d0, (%a4){%d2:%d7} /* insert it */
		add.l	%a2, %d2
	dbra	%d1, loopL
xit:	mov.l	&1, %d0			/* return 1 */
	movm.l	(%sp)+, &0x1ffc
	unlk	%fp
	rts
blank:	
	add.l	font_leading(%a3), %d1	/* just fill with background */
	add.l	&1, %d1
	br	blankit

/*
 *	Code to do an upward scroll
 */
#define	ffq	8(%fp)
#define	top	12(%fp)
#define	bottom	16(%fp)
#define	ll	20(%fp)
	global	font_scrollup
font_scrollup:
	link	%fp, &0
	movm.l	&0x3800, -(%sp)
	mov.l	ffq, %a1			/* Font pointer */
	mov.l	bottom, %d0			/* Last line */
	blt	retn
	cmp.l	%d0, font_maxy(%a1)		/* Check it against the screen*/
	blt	nxt				/*	bounds */
	mov.l	font_maxy(%a1), %d0
	subq.l	&1, %d0
nxt:
	addq.l	&1, %d0				/* Add one to get next line */
	mov.l	%d0, %d3
	mov.l	ll, %d1				/* Line count */
	ble	retn
	sub.l	top, %d3			/* Get the scroll height */
	ble	retn
	cmp.l	%d1, %d3			/* Set the line count to a max*/
	ble	nxt2				/*	of the scroll height */
	mov.l	%d3, %d1
nxt2:
	mov.l	font_screen(%a1), %a0		/* Get the screen info */
	mov.l	video_mem_x(%a0), %d3
	mov.l	video_scr_x(%a0), %d4
	sub.l	%d4, %d3			/* The line increment */
	lsr.l	&3, %d3				/*	in bytes */
	lsr.l	&5, %d4				/* Width in longs */
	subq.l	&1, %d4				/*	suitable for a dbra */
	mov.l	video_addr(%a0), %a0		/* Get the screen base */
	mov.l	font_linewidth(%a1), %d2	/* Get the line width */
	mulu.l	%d2, %d0			/* Get the end of scroll */
	lsr.l	&3, %d0				/*	as a byte offset */
	mulu.l	%d2, %d1			/* Get the increment */
	lsr.l	&3, %d1				/*	as a byte offset */
	sub.l	%d1, %d0			/* Get the end of the copy */
	mulu.l	top, %d2			/* Get the start of the scroll*/
	lsr.l	&3, %d2				/*	in bytes */
	add.l	%a0, %d0			/* Make it a screen address */
	add.l	%d2, %a0 			/* Do the same for the end */
loop2:	cmp.l	%a0, %d0			/* Loop per line */
	bcc	fill
		mov.l	%d4, %d2
ll2:
		mov.l	(0,%a0,%d1.l*1), (%a0)	/* Copy a word */
		addq.l	&4, %a0
		dbra	%d2, ll2
		add.l	%d3, %a0		/* Skip to the next scanline */
	bra	loop2
fill:	add.l	%d1, %d0			/* Now point to the end */
	tst.l	font_inverse(%a1)		/* Do the right thing for inv */
	bne	inv2				/*	screens */
loop3:	cmp.l	%a0, %d0
	bcc	retn
		mov.l	%d4, %d2
ll3:
		mov.l	&0xffffffff, (%a0)+	/* Loop filling the scrolled */
		dbra	%d2, ll3		/*	region */
		add.l	%d3, %a0
	bra	loop3
inv2:
loop4:	cmp.l	%a0, %d0
	bcc	retn
		mov.l	%d4, %d2
ll4:
		clr.l	(%a0)+			/* Loop filling the scrolled */
		dbra	%d2, ll4		/*	region */
		add.l	%d3, %a0
	bra	loop4
retn:	movm.l	(%sp)+, &0x001c
	unlk	%fp
	rts

/*
 *	Code to do a downward scroll
 */
	global	font_scrolldown
font_scrolldown:
	link	%fp, &0
	movm.l	&0x3800, -(%sp)
	mov.l	ffq, %a1		/* font pointer */
	mov.l	top, %d0		/* start line */
	blt	retn2
	mov.l	bottom, %d3		/* ending line */
	cmp.l	%d0, %d3
	bgt	retn2
	addq.l	&1, %d3			/* actaually make it the next one */
	mov.l	ll, %d1			/* line count */
	ble	retn2
	mov.l	%d3, bottom		/* save it for later */
	sub.l	%d0, %d3		/* check the length */
	cmp.l	%d1, %d3
	blt	nxt4
	mov.l	%d3, %d1		/* make it the max */
	beq	retn2
nxt4:
	mov.l	font_screen(%a1), %a0	/* get the screen base */
	mov.l	video_mem_x(%a0), %d3	/* get the mem width */
	mov.l	video_scr_x(%a0), %d4	/* get the screen width */
	sub.l	%d4, %d3		/* figure out the increment */
	lsr.l	&3, %d3			/* convert it into bytes */
	lsr.l	&5, %d4			/* figure out the screen width in */
	subq.l	&1, %d4			/* 	longs */
	mov.l	video_addr(%a0), %a0	/* get the screen address */
	mov.l	font_linewidth(%a1), %d2/* get the number of pixels in a line */
	mulu.l	%d2, %d0		/* convert the line count to pixels */
	lsr.l	&3, %d0			/* 	now back to bytes */
	mulu.l	%d2, %d1		/* get the starting address in pixels */
	lsr.l	&3, %d1			/* 	and now in bytes */
	add.l	%d1, %d0		/* Now figure out the move end */
	neg.l	%d1			/* Negate the increment */
	add.l	%a0, %d0		/* Point at the video card */
	mulu.l	bottom, %d2		/* Now decide where to start */
	lsr.l	&3, %d2			/*	in bytes */
	add.l	%d2, %a0		/* 	from the physical address */
loop5:	cmp.l	%a0, %d0		/* Loop to the end */
	bls	fill2
		mov.l	%d4, %d2	/* Get the loop counter */
		sub.l	%d3, %a0	/* Decrement the memory pointer */
ll1:
		subq.l	&4, %a0		/* loop in */
		mov.l	(0,%a0,%d1.l*1), (%a0) /* do the move */
		dbra	%d2, ll1
	bra	loop5
fill2:	add.l	%d1, %d0		/* We are done ... now blank the rest */
	tst.l	font_inverse(%a1)	/*	depending on its inversion */
	bne	inv3
loop6:	cmp.l	%a0, %d0
	bls	retn2
		mov.l	%d4, %d2
		sub.l	%d3, %a0
ll5:
		mov.l	&0xffffffff, -(%a0)
		dbra	%d2, ll5
	bra	loop6
inv3:
loop7:	cmp.l	%a0, %d0
	bls	retn2
		mov.l	%d4, %d2
		sub.l	%d3, %a0
ll6:
		clr.l	-(%a0)
		dbra	%d2, ll6
	bra	loop7
retn2:	movm.l	(%sp)+, &0x001c
	unlk	%fp
	rts

/*
 *	erase the whole screen
 */
	global	font_clear
font_clear:
	link	%fp, &0
	movm.l	&0x3000, -(%sp)
	mov.l	ffq, %a1		/* font pointer */
	mov.l	font_maxy(%a1), %d0
	mulu.l	font_linewidth(%a1), %d0
	asr.l	&3, %d0
	mov.l	font_screen(%a1), %a0
	mov.l	video_mem_x(%a0), %d3
	mov.l	video_scr_x(%a0), %d1
	sub.l	%d1, %d3
	lsr.l	&3, %d3
	lsr.l	&5, %d1
	subq.l	&1, %d1
	mov.l	video_addr(%a0), %a0
	add.l	%a0, %d0
	tst.l	font_inverse(%a1)
	bne	inv5
	mov.l	&0xffffffff, %a1
	bra	lll
inv5:	sub.l	%a1, %a1
lll:	cmp.l	%a0, %d0
	bcc	retn3
		mov.l	%d1, %d2
ll7:
		mov.l	%a1, (%a0)+
		dbra	%d2, ll7
		add.l	%d3, %a0
	bra	lll
retn3:	movm.l	(%sp)+, &0x000c
	unlk	%fp
	rts

	
