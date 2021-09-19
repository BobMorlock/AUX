/*	@(#)video.c	UniPlus VVV.2.1.24	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

/*
 *	This is the total number of boards supported
 */

#define NDEVICES	1

#include "sys/types.h"
#include "sys/errno.h"
#include "sys/param.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/uconfig.h"
#include "sys/sysmacros.h"
#include "sys/reg.h"
#include "sys/mmu.h"
#include "sys/page.h"
#include "sys/seg.h"
#include "sys/systm.h"
#include "sys/time.h"
#include "sys/user.h"
#include "sys/video.h"
#include "setjmp.h"
#include "sys/stream.h"
#include "sys/via6522.h"
#include "sys/debug.h"
#include "sys/module.h"

int video_count = 0;			/* number of video boards */
static struct video video[NDEVICES];	/* their attributes */

#define VID_DIM		0x0
#define VID_PAGE00	0x4
#define VID_PAGE02	0x8
#define VID_MODE00	0x8
#define VID_MODE02	0x4
#define VID_INT		0xc

struct video *video_desc[NDEVICES];	/* pointer to video board description */
static struct video *video_index[16];	/* how to find it by slot number */

/*
 *	The following is for automatic device dimming (not supported on all
 *			devices)
 */

#define DEFAULT_DIMTIME 	10		/* minutes */

static video_ioctl();
static video_func();
static video_dimmer();
static video_clear();
static video_dim();
static video_undim();
static video_setpage();
static video_find();
static video_defcolor();
static video_setdepth();
static video_setinit();

/*
 *	NGC initialisation table
 */

unsigned char ngc_init[] = {
	0xdf, 0xb8, 0xff, 0xff, 0xe1, 0x1a, 0x88, 0xb9,
	0xfa, 0xfd, 0xfd, 0xfe, 0xf0, 0xbe, 0xfa, 0x37,
};

/*
 *	bit/pixel initialisation tables
 */

unsigned char ngc_bits[] = {
/* 1bit non-inter */
	0x20, 0x47, 0x00, 0x00, 0x1e, 0xe5, 0x77, 0x46,
	0x05, 0x02, 0x02, 0x01, 0x0f, 0x41, 0x05, 0xc8,

/* 2bit non-inter */
	0x40, 0x47, 0x00, 0x00, 0x3c, 0xe5, 0x77, 0x46,
      	0x05, 0x06, 0x06, 0x04, 0x20, 0x04, 0x0b, 0xd8,

/* 4bit non-inter */
	0x80, 0x47, 0x00, 0x00, 0x78, 0xe5, 0x77, 0x46,
      	0x05, 0x0e, 0x0e, 0x0a, 0x42, 0x8a, 0x16, 0xe8,

/* 8bit non-inter */
	0x00, 0x47, 0x00, 0x00, 0xf0, 0xe5, 0x77, 0x46,
      	0x05, 0x1e, 0x1e, 0x16, 0x86, 0x96, 0x2d, 0xf9,

/* 1bit inter */
	0x20, 0xb7, 0x00, 0x00, 0x16, 0xe5, 0x3b, 0x19,
	0x05, 0x00, 0x00, 0x00, 0x10, 0x00, 0x05, 0xc8,

/* 2bit inter */
	0x40, 0xb7, 0x00, 0x00, 0x2c, 0xe5, 0x3b, 0x19,
      	0x05, 0x02, 0x04, 0x02, 0x22, 0x02, 0x0b, 0xd8,

/* 4bit inter */
	0x80, 0xb7, 0x00, 0x00, 0x58, 0xe5, 0x3b, 0x19,
      	0x05, 0x03, 0x0c, 0x06, 0x42, 0x06, 0x17, 0xe8,

/* 8bit inter */
	0x00, 0xb7, 0x00, 0x00, 0x60, 0xe5, 0x3b, 0x19,
      	0x05, 0x08, 0x1a, 0x0e, 0x86, 0x9e, 0x2e, 0xf9,
};

/*
 *	Standard colour lookup tables for 1/2/4/8 bits/pixel
 */

unsigned char ngc_onetbl[] = {
	0xff, 0xff, 0xff,
	0x00, 0x00, 0x00,
};

unsigned char ngc_twotbl[] = {
	0xff, 0xff, 0xff,
	0x96, 0x96, 0x00,
	0x00, 0x96, 0x96,
	0x00, 0x00, 0x00,
};

unsigned char ngc_fourtbl[] = {
	0xff, 0xff, 0xff,
	0x96, 0x96, 0x96,
	0x80, 0x80, 0x00,
	0xff, 0xff, 0x00,
	0x00, 0x80, 0x80,
	0x00, 0xff, 0xff,
	0x00, 0x80, 0x00,
	0x00, 0xff, 0x00,
	0x80, 0x00, 0x80,
	0xff, 0x00, 0xff,
	0x80, 0x00, 0x00,
	0xff, 0x00, 0x00,
	0x00, 0x00, 0x80,
	0x00, 0x00, 0xff,
	0x4b, 0x4b, 0x4b,
	0x00, 0x00, 0x00,
};


unsigned char ngc_eighttbl[] = {
	0xff, 0xff, 0xff,
	0xee, 0xee, 0xee,
	0xdd, 0xdd, 0xdd,
	0xbb, 0xbb, 0xbb,
	0xaa, 0xaa, 0xaa,
	0x88, 0x88, 0x88,
	0x77, 0x77, 0x77,
	0x55, 0x55, 0x55,
	0x44, 0x44, 0x44,
	0x22, 0x22, 0x22,
	0x11, 0x11, 0x11,
	0xee, 0x00, 0xee,
	0xdd, 0x00, 0xdd,
	0xbb, 0x00, 0xbb,
	0xaa, 0x00, 0xaa,
	0x88, 0x00, 0x88,
	0x77, 0x00, 0x77,
	0x55, 0x00, 0x55,
	0x44, 0x00, 0x44,
	0x22, 0x00, 0x22,
	0x11, 0x00, 0x11,
	0x00, 0xee, 0xee,
	0x00, 0xdd, 0xdd,
	0x00, 0xbb, 0xbb,
	0x00, 0xaa, 0xaa,
	0x00, 0x88, 0x88,
	0x00, 0x77, 0x77,
	0x00, 0x55, 0x55,
	0x00, 0x44, 0x44,
	0x00, 0x22, 0x22,
	0x00, 0x11, 0x11,
	0xee, 0xee, 0x00,
	0xdd, 0xdd, 0x00,
	0xbb, 0xbb, 0x00,
	0xaa, 0xaa, 0x00,
	0x88, 0x88, 0x00,
	0x77, 0x77, 0x00,
	0x55, 0x55, 0x00,
	0x44, 0x44, 0x00,
	0x22, 0x22, 0x00,
	0x11, 0x11, 0x00,
	0xff, 0xff, 0xcc,
	0xff, 0xff, 0x99,
	0xff, 0xff, 0x66,
	0xff, 0xff, 0x33,
	0xff, 0xff, 0x00,
	0xff, 0xcc, 0xff,
	0xff, 0xcc, 0xcc,
	0xff, 0xcc, 0x99,
	0xff, 0xcc, 0x66,
	0xff, 0xcc, 0x33,
	0xff, 0xcc, 0x00,
	0xff, 0x99, 0xff,
	0xff, 0x99, 0xcc,
	0xff, 0x99, 0x99,
	0xff, 0x99, 0x66,
	0xff, 0x99, 0x33,
	0xff, 0x99, 0x00,
	0xff, 0x66, 0xff,
	0xff, 0x66, 0xcc,
	0xff, 0x66, 0x99,
	0xff, 0x66, 0x66,
	0xff, 0x66, 0x33,
	0xff, 0x66, 0x00,
	0xff, 0x33, 0xff,
	0xff, 0x33, 0xcc,
	0xff, 0x33, 0x99,
	0xff, 0x33, 0x66,
	0xff, 0x33, 0x33,
	0xff, 0x33, 0x00,
	0xff, 0x00, 0xff,
	0xff, 0x00, 0xcc,
	0xff, 0x00, 0x99,
	0xff, 0x00, 0x66,
	0xff, 0x00, 0x33,
	0xff, 0x00, 0x00,
	0xcc, 0xff, 0xff,
	0xcc, 0xff, 0xcc,
	0xcc, 0xff, 0x99,
	0xcc, 0xff, 0x66,
	0xcc, 0xff, 0x33,
	0xcc, 0xff, 0x00,
	0xcc, 0xcc, 0xff,
	0xcc, 0xcc, 0xcc,
	0xcc, 0xcc, 0x99,
	0xcc, 0xcc, 0x66,
	0xcc, 0xcc, 0x33,
	0xcc, 0xcc, 0x00,
	0xcc, 0x99, 0xff,
	0xcc, 0x99, 0xcc,
	0xcc, 0x99, 0x99,
	0xcc, 0x99, 0x66,
	0xcc, 0x99, 0x33,
	0xcc, 0x99, 0x00,
	0xcc, 0x66, 0xff,
	0xcc, 0x66, 0xcc,
	0xcc, 0x66, 0x99,
	0xcc, 0x66, 0x66,
	0xcc, 0x66, 0x33,
	0xcc, 0x66, 0x00,
	0xcc, 0x33, 0xff,
	0xcc, 0x33, 0xcc,
	0xcc, 0x33, 0x99,
	0xcc, 0x33, 0x66,
	0xcc, 0x33, 0x33,
	0xcc, 0x33, 0x00,
	0xcc, 0x00, 0xff,
	0xcc, 0x00, 0xcc,
	0xcc, 0x00, 0x99,
	0xcc, 0x00, 0x66,
	0xcc, 0x00, 0x33,
	0xcc, 0x00, 0x00,
	0x99, 0xff, 0xff,
	0x99, 0xff, 0xcc,
	0x99, 0xff, 0x99,
	0x99, 0xff, 0x66,
	0x99, 0xff, 0x33,
	0x99, 0xff, 0x00,
	0x99, 0xcc, 0xff,
	0x99, 0xcc, 0xcc,
	0x99, 0xcc, 0x99,
	0x99, 0xcc, 0x66,
	0x99, 0xcc, 0x33,
	0x99, 0xcc, 0x00,
	0x99, 0x99, 0xff,
	0x99, 0x99, 0xcc,
	0x99, 0x99, 0x99,
	0x99, 0x99, 0x66,
	0x99, 0x99, 0x33,
	0x99, 0x99, 0x00,
	0x99, 0x66, 0xff,
	0x99, 0x66, 0xcc,
	0x99, 0x66, 0x99,
	0x99, 0x66, 0x66,
	0x99, 0x66, 0x33,
	0x99, 0x66, 0x00,
	0x99, 0x33, 0xff,
	0x99, 0x33, 0xcc,
	0x99, 0x33, 0x99,
	0x99, 0x33, 0x66,
	0x99, 0x33, 0x33,
	0x99, 0x33, 0x00,
	0x99, 0x00, 0xff,
	0x99, 0x00, 0xcc,
	0x99, 0x00, 0x99,
	0x99, 0x00, 0x66,
	0x99, 0x00, 0x33,
	0x99, 0x00, 0x00,
	0x66, 0xff, 0xff,
	0x66, 0xff, 0xcc,
	0x66, 0xff, 0x99,
	0x66, 0xff, 0x66,
	0x66, 0xff, 0x33,
	0x66, 0xff, 0x00,
	0x66, 0xcc, 0xff,
	0x66, 0xcc, 0xcc,
	0x66, 0xcc, 0x99,
	0x66, 0xcc, 0x66,
	0x66, 0xcc, 0x33,
	0x66, 0xcc, 0x00,
	0x66, 0x99, 0xff,
	0x66, 0x99, 0xcc,
	0x66, 0x99, 0x99,
	0x66, 0x99, 0x66,
	0x66, 0x99, 0x33,
	0x66, 0x99, 0x00,
	0x66, 0x66, 0xff,
	0x66, 0x66, 0xcc,
	0x66, 0x66, 0x99,
	0x66, 0x66, 0x66,
	0x66, 0x66, 0x33,
	0x66, 0x66, 0x00,
	0x66, 0x33, 0xff,
	0x66, 0x33, 0xcc,
	0x66, 0x33, 0x99,
	0x66, 0x33, 0x66,
	0x66, 0x33, 0x33,
	0x66, 0x33, 0x00,
	0x66, 0x00, 0xff,
	0x66, 0x00, 0xcc,
	0x66, 0x00, 0x99,
	0x66, 0x00, 0x66,
	0x66, 0x00, 0x33,
	0x66, 0x00, 0x00,
	0x33, 0xff, 0xff,
	0x33, 0xff, 0xcc,
	0x33, 0xff, 0x99,
	0x33, 0xff, 0x66,
	0x33, 0xff, 0x33,
	0x33, 0xff, 0x00,
	0x33, 0xcc, 0xff,
	0x33, 0xcc, 0xcc,
	0x33, 0xcc, 0x99,
	0x33, 0xcc, 0x66,
	0x33, 0xcc, 0x33,
	0x33, 0xcc, 0x00,
	0x33, 0x99, 0xff,
	0x33, 0x99, 0xcc,
	0x33, 0x99, 0x99,
	0x33, 0x99, 0x66,
	0x33, 0x99, 0x33,
	0x33, 0x99, 0x00,
	0x33, 0x66, 0xff,
	0x33, 0x66, 0xcc,
	0x33, 0x66, 0x99,
	0x33, 0x66, 0x66,
	0x33, 0x66, 0x33,
	0x33, 0x66, 0x00,
	0x33, 0x33, 0xff,
	0x33, 0x33, 0xcc,
	0x33, 0x33, 0x99,
	0x33, 0x33, 0x66,
	0x33, 0x33, 0x33,
	0x33, 0x33, 0x00,
	0x33, 0x00, 0xff,
	0x33, 0x00, 0xcc,
	0x33, 0x00, 0x99,
	0x33, 0x00, 0x66,
	0x33, 0x00, 0x33,
	0x33, 0x00, 0x00,
	0x00, 0xff, 0xff,
	0x00, 0xff, 0xcc,
	0x00, 0xff, 0x99,
	0x00, 0xff, 0x66,
	0x00, 0xff, 0x33,
	0x00, 0xff, 0x00,
	0x00, 0xcc, 0xff,
	0x00, 0xcc, 0xcc,
	0x00, 0xcc, 0x99,
	0x00, 0xcc, 0x66,
	0x00, 0xcc, 0x33,
	0x00, 0xcc, 0x00,
	0x00, 0x99, 0xff,
	0x00, 0x99, 0xcc,
	0x00, 0x99, 0x99,
	0x00, 0x99, 0x66,
	0x00, 0x99, 0x33,
	0x00, 0x99, 0x00,
	0x00, 0x66, 0xff,
	0x00, 0x66, 0xcc,
	0x00, 0x66, 0x99,
	0x00, 0x66, 0x66,
	0x00, 0x66, 0x33,
	0x00, 0x66, 0x00,
	0x00, 0x33, 0xff,
	0x00, 0x33, 0xcc,
	0x00, 0x33, 0x99,
	0x00, 0x33, 0x66,
	0x00, 0x33, 0x33,
	0x00, 0x33, 0x00,
	0x00, 0x00, 0xff,
	0x00, 0x00, 0xcc,
	0x00, 0x00, 0x99,
	0x00, 0x00, 0x66,
	0x00, 0x00, 0x33,
	0x00, 0x00, 0x00,
};

/*
 *	Video Interrupt Handler
 */


video_intr(args)
struct args *args;
{
	register int i;
	register struct video *vp;

	vp = video_index[args->a_dev];
	if (vp == NULL)
		panic("video interrupt");

	/*
	 *	Clear the interrupt
	 */

	switch(vp->video_type) {
	case VIDEO_1003_00:
	case VIDEO_1003_02:
		*vp->video_cmd = VID_INT+vp->video_page;
		break;

	case VIDEO_NGC_1_0:
		((struct ngc *)vp->video_base)->ngc_intr = 0;
		break;
	}

	/*
	 *	If required call a service routine
	 */

	if (vp->video_intr)
		(*vp->video_intr)(vp);
}

/*
 *	video initialisation routine. First call video_find to probe the slots
 *	for video boards (we can't do this via autoconfig as we need the screen
 *	to come up and run autoconfig .... chicken and egg problem). Next we
 *	set up the video device descriptor and initialise the device. Finally
 *	we clear the screen.
 */

video_init()
{
	register struct video *vp;
	register int i;
	struct ngc *ngcp;

	video_find();
	for (vp = video, i = 0; i < video_count; vp++, i++) {
		video_desc[i] = vp;
		vp->video_timeout = DEFAULT_DIMTIME;
		vp->video_off = i;
		vp->video_intr = 0;
		vp->video_ioctl = video_ioctl;
		vp->video_func = video_func;
#ifndef PMMU
		vp->video_base = (caddr_t)sptalloc(btop(4*64*1024),
				PG_V|PG_RW, btop((unsigned)(vp->video_addr)));
		if (vp->video_base == 0)
			panic("video ram sptalloc failed");
#else
		vp->video_base = vp->video_addr;
#endif PMMU
		switch (vp->video_type) {
		case VIDEO_1003_00:
		case VIDEO_1003_02:
#ifndef PMMU
			vp->video_cmd = (char *)sptalloc(1, PG_V|PG_RW,
				btop((unsigned)(vp->video_addr+0x00880000)));
			if (vp->video_cmd == 0)
				panic("video command sptalloc failed");
#else
			vp->video_cmd = vp->video_addr+0x00880000;
#endif PMMU
			vp->video_addr = vp->video_base;
			*vp->video_cmd = (vp->video_type == VIDEO_1003_00 ?
					VID_MODE00 : VID_MODE02) | 1;
			video_setpage(vp, 0);
			video_clear(vp, 1);
			video_setpage(vp, 1);
			video_clear(vp, 1);
			video_setpage(vp, 2);
			video_clear(vp, 1);
			video_setpage(vp, 3);
			video_clear(vp, 1);
			video_undim(vp);
			viamkslotintr(vp->video_slot, video_intr, 0);
			timeout(video_dimmer, vp, 60*HZ);
			break;
		case VIDEO_NGC_1_0:

			/*
			 *	Assume PMMU .... MMB map in cost too much
			 */

#ifndef PMMU
			panic("NGC PMMU setup");
#endif PMMU
			vp->video_color = NGC_COLOR;
			video_setinit((struct ngc *)(vp->video_base));
			video_setdepth(vp, NGC_1NI);
			video_clear(vp, 1);
			viamkslotintr(vp->video_slot, video_intr, 1);
			break;
		}
	}
}

/*
 *	Clear the screen
 */

static
video_clear(vp, value)
struct video *vp;
{
	register int i, lines, width, inc;
	register char *cp;

	if (value)
		value = 0xffffffff;
	cp = vp->video_addr;
	lines = vp->video_scr_y - 1;
	width = vp->video_scr_x/(8*sizeof(long)) - 1;
	inc = (vp->video_mem_x - vp->video_scr_x)>>3;
	do {
		i = width;
		do {
			*(long *)cp = value;
			cp += sizeof(long);
		} while(i--);
		cp += inc;
	} while (lines--);
}

/*
 *	perform device dependant functions to the screen (this is called via
 *	a pointer in the descriptor so that other people can write their own
 *	versions of this routine). Not all functions are supported by all
 *	devices.
 */

static 
video_func(vp, code, p1)
struct video *vp;
int code;
{
	switch(code) {
	case VF_CLEAR:

		/*
		 *	clear the screen
		 */

		video_clear(vp, p1);
		return(0);

	case VF_SETPAGE:

		/*
		 *	set the displayed page
		 */

		video_setpage(vp, p1);
		return(0);

	case VF_DIM:

		/*
		 *	dim the screen
		 */

		video_dim(vp);
		return(0);

	case VF_UNDIM:

		/*
		 *	undim the screen
		 */

		video_undim(vp);
		return(0);

	case VF_ENABLE:

		/*
		 *	turn on vertical retrace interrupts
		 */

		if (vp->video_type == VIDEO_NGC_1_0)
			((struct ngc *)vp->video_base)->ngc_intr = 0;
		return(0);

	case VF_DISABLE:

		/*
		 *	turn off vertical retrace interrupts
		 */

		if (vp->video_type == VIDEO_NGC_1_0)
			((struct ngc *)vp->video_base)->ngc_disi = 0;
		return(0);

	case VF_VINTR:

		/*
		 *	return 1 if we are currently doing a vertical retrace
		 */

		if (vp->video_type == VIDEO_NGC_1_0)
			return(((struct ngc *)vp->video_base)->ngc_vsync&1);
		return(1);

	default:
		return(0);
	}
}

/*
 *	dim the screen
 */

static
video_dim(vp)
struct video *vp;
{
	switch(vp->video_type) {
	case VIDEO_1003_00:
	case VIDEO_1003_02:
		*vp->video_cmd = VID_DIM|1;
		break;
	case VIDEO_NGC_1_0:
		break;
	}
	vp->video_dimmed = 1;
}

/*
 *	undim the screen
 */

static
video_undim(vp)
struct video *vp;
{
	switch(vp->video_type) {
	case VIDEO_1003_00:
	case VIDEO_1003_02:
		*vp->video_cmd = VID_DIM|0;
		break;
	case VIDEO_NGC_1_0:
		break;
	}
	vp->video_dimmed = 0;
	vp->video_dimtime = vp->video_timeout;
}

/*
 *	this is the timeout routine that dims the screen
 */

static
video_dimmer(vp)
struct video *vp;
{
	if (!vp->video_dimmed && vp->video_dimtime != 0xffffffff)
	if (vp->video_dimtime) {
		vp->video_dimtime--;
	} else {
		video_dim(vp);
	}
	timeout(video_dimmer, vp, 60*HZ);
}

/*
 *	set the current page
 */

static
video_setpage(vp, page)
struct video *vp;
{
	switch(vp->video_type) {
	case VIDEO_1003_00:
	case VIDEO_1003_02:
		if (page >= 0 && page <= 3) {
			*vp->video_cmd = (vp->video_type == VIDEO_1003_00 ?
				VID_PAGE00 : VID_PAGE02) + page;
			vp->video_addr = vp->video_base + page*64*1024;
		}
		break;
	case VIDEO_NGC_1_0:
		break;
	}
	vp->video_page = page;
}

#ifdef SLOTS
/*
 *	Autoconfig video cards (chicken/egg problem: needed because we have
 *		to come up with a console before we can autoconfig)
 */

static
video_find()
{
	register long slot;
	register struct video *vp;
	register struct auto_data *adp;

	vp = video;
	video_count = 0;
	adp = (struct auto_data *)AUTO_ADDR;
	if (adp->auto_magic != AUTO_MAGIC)
		return;
	for (slot = SLOT_LO; slot <= SLOT_HI; slot++) {
		if (video_count >= NDEVICES)
			break;
		if (adp->auto_id[slot] != 5)
			continue;
		vp->video_addr = (char *)(0xf0000000 | (slot<<24));
		video_index[slot] = vp;
		vp->video_slot = slot;
		vp->video_type = VIDEO_NGC_1_0;
		vp->video_mem_x = 1024;
		vp->video_mem_y = 512;
		vp->video_scr_x = 640;
		vp->video_scr_y = 480;
		video_count++;
		vp++;
	}
}

#else SLOTS

/*
 *	Routine to read a long from the rom
 */

static long
v_long(virt, off, rs)
char *virt;
int off, rs;
{
	long v;

	v = (long)virt | (off*rs);
	virt = (char *)v;
	v = *virt;
	virt += rs;
	v = (v<<8)|(*virt&0xff);
	virt += rs;
	v = (v<<8)|(*virt&0xff);
	virt += rs;
	return((v<<8)|(*virt&0xff));
}

/*
 *	Autoconfig video cards (chicken/egg problem: needed because we have
 *		to come up with a console before we can autoconfig)
 */

static
video_find()
{
	jmp_buf jb;int *saved_jb;
	register long slot, base;
	register struct video *vp;
	int i, rs;
	long inc;

	vp = video;
	video_count = 0;

	for (slot = 9; slot <= 14; slot++) {
		if (video_count >= NDEVICES)
			break;
		base = 0xf0ff0000 | (slot<<24);	/* get a card ROM address */
#ifndef PMMU
		base = (long)sptalloc(btop(64*1024), PG_V|PG_RW, btop(base));
		if (base == 0)
			panic("video sptalloc");
#endif PMMU
		saved_jb = u.u_nofault;
		if (!setjmp(jb)) {
			/*
			 *	Read the ROM looking for a video card, if
			 *		one is found, mark it and set up it's
			 *		parameters. If no board is present
			 *		we buserr and longjump back to the
			 *		setjmp above ....
			 */

			u.u_nofault = jb;
			rs = 4;
			inc = 0xffa0/rs;
			i = v_long(base, inc+4, rs);
			if (i != 0x5a932bc7 && i != 0x00fffc14 &&
			    i != 0x00fff014) {
				u.u_nofault = saved_jb;
				continue;
			}
			vp->video_addr = (char *)(base&0xff000000);
			video_index[slot] = vp;
			vp->video_slot = slot;
			vp->video_type = VIDEO_NGC_1_0;
			vp->video_mem_x = 1024;
			vp->video_mem_y = 512;
			vp->video_scr_x = 640;
			vp->video_scr_y = 480;
			video_count++;
			vp++;
		}
		u.u_nofault = saved_jb;
#ifndef PMMU
		sptfree(base, btop(64*1024), 0);
#endif PMMU
	}
}
#endif SLOTS

/*
 *	This routine is called from below to do various ioctls (not all of
 *	which are supported on all cards
 */

static
video_ioctl(vp, iocbp, m)
struct video *vp;
struct iocblk *iocbp;
mblk_t *m;
{
	register struct ngc *ngcp;
	register int i;
	register unsigned char *cp;
	register mblk_t *m1;

	switch(iocbp->ioc_cmd) {
	case VIDEO_DIM:

	/*
	 *	Set the dim timeout (in seconds)
	 */

		if (vp->video_type == VIDEO_NGC_1_0 ||
		    iocbp->ioc_count != sizeof(long)) 
			return(1);
		vp->video_timeout = *(long *)m->b_cont->b_rptr;
		if (vp->video_timeout == 0)
			vp->video_timeout = 0xffffffff;
		iocbp->ioc_count = 0;
		freemsg(unlinkb(m));
		return(0);

	case VIDEO_SETCOLOR:

		/*
		 *	set the device to black and white or colour
	   	 */

		if (vp->video_type != VIDEO_NGC_1_0 ||
		    iocbp->ioc_count != sizeof(long)) 
			return(1);
		i = *(long *)m->b_cont->b_rptr;
		if (i != NGC_BW && i != NGC_COLOR)
			return(1);
		iocbp->ioc_count = 0;
		freemsg(unlinkb(m));
		vp->video_color = i;
		return(0);

	case VIDEO_SETDEF:

		/*
		 *	return the colour lookup table (CLUT) to its default
		 */

		if (vp->video_type != VIDEO_NGC_1_0)
			return(1);
		freemsg(unlinkb(m));
		iocbp->ioc_count = 0;
		video_defcolor(vp);
		return(0);

	case VIDEO_SETMODE:

		/*
		 *	change the cards mode (bits/pixel, interlace)
		 */

		if (vp->video_type != VIDEO_NGC_1_0 ||
		    iocbp->ioc_count != sizeof(long)) 
			return(1);
		i = *(long *)m->b_cont->b_rptr;
		if (i < 0 || i >= 8)
			return(1);
		freemsg(unlinkb(m));
		iocbp->ioc_count = 0;
		video_setdepth(vp, i);
		return(0);

	case VIDEO_SETCENT:

		/*
		 *	set up an entry in the CLUT
		 */

		if (vp->video_type != VIDEO_NGC_1_0 ||
		    iocbp->ioc_count != 3*256*sizeof(char)) 
			return(1);
		ngcp = (struct ngc *)vp->video_base;
		m1 = m->b_cont;
		ngcp->ngc_clut.addr = *m1->b_rptr++;
		ngcp->ngc_clut.palette = *m1->b_rptr++;
		ngcp->ngc_clut.palette = *m1->b_rptr++;
		ngcp->ngc_clut.palette = *m1->b_rptr++;
		freemsg(unlinkb(m));
		iocbp->ioc_count = 0;
		return(0);

	case VIDEO_SETCLUT:

		/*
		 *	load an entire clut
		 */

		if (vp->video_type != VIDEO_NGC_1_0 ||
		    iocbp->ioc_count != 3*256*sizeof(char)) 
			return(1);
		ngcp = (struct ngc *)vp->video_base;
		ngcp->ngc_clut.addr = 0xff;
		m1 = m->b_cont;
		for (i = 0; i < 256*3; i++) {
			while (m1->b_rptr >= m1->b_wptr)
				m1 = m1->b_cont;
			ngcp->ngc_clut.palette = *m1->b_rptr++;
		}
		freemsg(unlinkb(m));
		iocbp->ioc_count = 0;
		return(0);
	default:
		return(1);
	}
}

/*
 *	This initialises the TFB after a reset
 */

static
video_setinit(ngcp)
register struct ngc *ngcp;
{
	register int i;

	for (i = 15; i >= 0; i--) {
		ngcp->ngc_tfbi[i].x = ngc_init[15-i];
	}
}

/*
 *	Here we set the display depth 1/2/4/8 bits/pixel, interlaced or
 *		non-interlaced
 */

static
video_setdepth(vp, d)
register struct video *vp;
register int d;
{
	register unsigned char *cp;
	register struct ngc *ngcp;

	ngcp = (struct ngc *)vp->video_base;
	if (d >= 8 || d < 0)
		return(0);
	vp->video_mode = d;
	cp = &ngc_bits[d*16];
	ngcp->ngc_tfb[15].x = 0xb7;
	for (d = 0; d < 16; d++)
		ngcp->ngc_tfb[d].x = ~*cp++;
	return(video_defcolor(vp));
}

/*
 *	This routine sets up the device with the default colour lookup
 *	table (appropriatly greyed if we have a B/W monitor)
 */

static
video_defcolor(vp)
register struct video *vp;
{
	register int i, j, k, l;
	register unsigned char *cp1, *cp2;
	unsigned char b[3*256];
	register struct ngc *ngcp;

	ngcp = (struct ngc *)vp->video_base;

	/*
	 *	Find the table dimensions 
	 */

	switch(vp->video_mode) {
	case NGC_1NI:
	case NGC_1I:
		cp1 = ngc_onetbl;
		k = 2;
		l = 256>>1;
		break;
	case NGC_2NI:
	case NGC_2I:
		cp1 = ngc_twotbl;
		k = 4;
		l = 256>>2;
		break;
	case NGC_4NI:
	case NGC_4I:
		cp1 = ngc_fourtbl;
		k = 16;
		l = 256>>4;
		break;
	case NGC_8NI:
	case NGC_8I:
		cp1 = ngc_eighttbl;
		k = 256;
		l = 256>>8;
		break;
	default:
		return(0);
	}

	/*
	 *	Build the table in core
	 */

	cp2 = b;
	for (i = 0; i < k; i++) {
		for (j = 0; j < l; j++) {
			*cp2++ = cp1[0];
			*cp2++ = cp1[1];
			*cp2++ = cp1[2];
		}
		cp1 += 3;
	}

	/*
	 *	If it's B/W then gray scale it
	 */

	if (vp->video_color == NGC_BW) {
		cp2 = b;
		for (i = 0; i < 256; i++) {
			j = ((30*cp2[0])+(59*cp2[1])+(11*cp2[2]))/100;
			if (j > 255)
				j = 255;
			*cp2++ = j;
			*cp2++ = j;
			*cp2++ = j;
		}
	}

	/*
	 *	zap it at the CLUT
	 */

	cp2 = b;
	ngcp->ngc_clut.addr = 0xff;
	for (j = 0; j < (3*256); j++)
		ngcp->ngc_clut.palette = *cp2++;
	return(1);
}
