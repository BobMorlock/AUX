/*	@(#)video.h	UniPlus VVV.2.1.12	*/
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

struct video {
	char	*video_addr;		/* address of screen bitmap */
	long 	video_mem_x;		/* screen x width in pixels */
	long 	video_mem_y;		/* screen y height in pixels */
	long 	video_scr_x;		/* memory x width in pixels */
	long 	video_scr_y;		/* memory y height in pixels */
	char 	*video_cmd;		/* command register address */
	char	*video_base;		/* board base address */
	int	(*video_func)();	/* screen function routine */
	char	video_dimmed;		/* dimmed state */
	char	video_page;		/* current screen page */
	int	video_dimtime;		/* time remaining before dimming
					   (in seconds) */
	int	video_timeout;		/* how long before dimming (in
					   seconds) */
	int	video_type;		/* Screen type */
	int	video_slot;		/* The slot it is in */
	int	(*video_ioctl)();	/* routine to do card dependant IOCTLs*/
	int	(*video_intr)();	/* routine to call on interrupt */
	int	video_mode;		/* current video mode */
	int	video_color;		/* are we a color screen? */
	char	*video_ptr;		/* extension area */
	int	video_off;		/* Our 'screen number' */
	unsigned char video_table[3*256];/* the clut load table */
	int	video_request;		/* do a colour table load on the next
					   interrupt */
	int	video_intstate;		/* the interrupt state */
};

extern struct video *video_desc[];	/* the video board's descriptions */
extern int video_count;			/* the number of video boards */

/*
 *	Screen functions
 */

#define	VF_CLEAR	1		/* clear screen */
#define	VF_SETPAGE	2		/* set screen page */
#define	VF_DIM		3		/* dim screen (not supported on all
					   boards) */
#define	VF_UNDIM	4		/* undim screen */
#define	VF_ENABLE	5		/* enable interrupt */
#define	VF_DISABLE	6		/* disable interrupt */
#define	VF_VINTR	7		/* is the vertical retrace
					   in progress */
#define	VF_ONCE		8		/* do at least one interrupt */

/*
 *	Video Board Types
 */

#define	VIDEO_1003_00	0		/* The original basic board */
#define	VIDEO_1003_02	1		/* The smt full function board */
#define	VIDEO_NGC_1_0	2		/* The 8 bit NGC rev 1.0 */

/*
 *	Video driver IOCTLs (all must be called via I_STR)
 */

#define VIDEO_RAW	(('V'<<8)|2)	/* put it into "raw" (keycode) mode */
#define VIDEO_ASCII	(('V'<<8)|3)	/* put it into vt100 mode */
#define VIDEO_DIM	(('V'<<8)|4)	/* set the dim timeout (1 longword
					   parameter .... time in seconds) 
					   (not supported on all boards) */
#define VIDEO_SIZE	(('V'<<8)|5)	/* return the size of the screen in
					   characters as 2 longwords, x and y */
#define VIDEO_SETMODE	(('V'<<8)|6)	/* Set the interlace/bits per pixel
					   mode (NGC board only). One longword
					   parameter ... see below */
#define VIDEO_SETCLUT	(('V'<<8)|7)	/* load the entire CLUT (NGC board
					   only). 256*3 bytes for red,
					   green, blue resp. */
#define VIDEO_SETCENT	(('V'<<8)|8)	/* load one CLUT entry (NGC board
					   only). 1 byte address, followed
					   by 3 bytes RGB */
#define VIDEO_SETDEF	(('V'<<8)|9)	/* Set up the screen for the default 
					   colour table for the current mode */
#define VIDEO_SETCOLOR	(('V'<<8)|10)	/* this is 1 for color, 0 for b/w, it
					   affects the default color table used
					   with the VIDEO_SETDEF call, one
					   longword parameter (see below) */
#define VIDEO_MOUSE	(('V'<<8)|11)	/* make the mouse generate escape
					   sequences in the display's keyboard's
					   input stream. These sequences consist
					   of MOUSE_ESCAPE (below) followed by 2
					   bytes (the high and low bytes of 
					   register 0, read from the mouse) this
					   fails if someone is already using
					   the mouse */
	
#define VIDEO_NOMOUSE	(('V'<<8)|12)	/* turn off mouse inputs into the
					   keyboard input stream */

#define VIDEO_M_BUTTON	(('V'<<8)|13)	/* If the terminal is in VIDEO_MOUSE
					   mode (above) then only put
					   data into the input stream when the
					   mouse button changes. Ignore the
					   position information. This option
					   only returns MOUSE_ESCAPE followed
					   by 1 byte (0/1) for the mouse 
					   position. */

#define VIDEO_M_ALL	(('V'<<8)|14)	/* This option returns the mouse
					   events to the default (as per
					   VIDEO_MOUSE above) */

#define	VIDEO_M_DELTA	(('V'<<8)|15)	/* This only works when in VIDEO_MOUSE
					   mode .... it returns the CHANGE in
					   mouse position since the last
					   call (2 shorts, horizontal followed
					   by vertical) */

#define	VIDEO_M_ABS	(('V'<<8)|16)	/* This only works when in VIDEO_MOUSE
					   mode .... it returns the ABSLOUTE 
					   mouse position (2 shorts, horizontal
					   followed by vertical) */

#define	VIDEO_ADDR	(('V'<<8)|17)	/* return (as a long-word) the physical
					   address of the video ram on the video
					   board (suitable for use in a phys(2)
					   system call) */

/*
 *	The escape character in the input stream to indicate mouse data. It 
 *	should be different from any keycode generated by the keyboard in
 *	ascii raw mode
 */

#define MOUSE_ESCAPE	0x7e

/*
 *	Defines for NGC screen types (VIDEO_SETCOLOR ioctl)
 */

#define	NGC_BW		0		/* Black/White screen */
#define	NGC_COLOR	1		/* Color screen */

/*
 *	Defines for NGC card modes (VIDEO_SETMODE ioctl)
 */

#define	NGC_1NI		0		/* 1-bit non-interlaced */
#define	NGC_2NI		1		/* 2-bit non-interlaced */
#define	NGC_4NI		2		/* 4-bit non-interlaced */
#define	NGC_8NI		3		/* 8-bit non-interlaced */
#define	NGC_1I		4		/* 1-bit interlaced */
#define	NGC_2I		5		/* 2-bit interlaced */
#define	NGC_4I		6		/* 4-bit interlaced */
#define	NGC_8I		7		/* 8-bit interlaced */

/*
 *	Info for the NGC card
 */

struct ngc {
	char ngc_ram[8*64*1024];
	struct tfb {
		char x;
		char 					pad[3];
	} ngc_tfb[16];
	char 						ngc_1[(64*1024)-
							 32*sizeof(struct tfb)];
	struct tfb ngc_tfbi[16];	/* tfb init */
	struct clut {
		char					pad0[16];
		char overlay;		/* Overlay registers (not used) */
		char					pad1[3];
		char addr2;
		char					pad2[3];
		char palette;		/* Colour palette */
		char					pad3[3];
		char addr;		/* Address register */
		char					pad4[3];
	} ngc_clut;
	char 						ngc_2[(64*1024)-
						         sizeof(struct clut)];
	char ngc_intr;			/* Interrupt clear */
	char 						ngc_3[(64*1024)-1];
	char ngc_seti;			/* Set interlace bit */
	char 						ngc_4[(64*1024)-1];
	char ngc_clri;			/* Clear interlace bit */
	char 						ngc_5[(64*1024)-1];
	char ngc_vsync;			/* Read VSYNC */
	char 						ngc_6[(64*1024)-1];
	char ngc_disi;			/* Disable interrupt */
	char 						ngc_7[(64*1024)-1];
	char ngc_rom[(64*1024)];	/* ROM */
};

