/*
 *	Font data structures
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

struct fontrec {
	unsigned short		fontType;
	unsigned short		firstChar;
	unsigned short		lastChar;
	unsigned short		widMax;
	short			kernMax;
	short			nDescent;
	unsigned short		fRectMax;
	unsigned short		chHeight;
	unsigned short		owTLoc;
	short			ascent;
	short			descent;
	short			leading;
	unsigned short		rowWords;
	unsigned short		bitImage[1];
};

#define	PROPFONT	0x9000	
#define	FIXEDFONT	0xB000	
#define	FONTWID		0xACB0	

struct font {
	struct fontrec *font_pnt;
	unsigned int font_height;
	unsigned int font_width;
	unsigned int font_leading;
	unsigned int font_rowwords;
	unsigned char *font_bitimage;
	unsigned short *font_loctable;
	unsigned short *font_owtable;
	unsigned int font_linewidth;
	unsigned int font_maxx;
	unsigned int font_maxy;
	unsigned int font_inverse;
	struct video *font_screen;
	unsigned int font_theight;
	int (*font_char)();		/* output a char */
	int (*font_scrollup)();		/* scroll the screen up */
	int (*font_scrolldown)();	/* scroll it down */
	int (*font_clear)();		/* clear the screen */
	int (*font_invert)();		/* reverse video a char */
	int (*font_delete)();		/* delete characters */
	int (*font_insert)();		/* insert characters */
	int (*font_erase)();		/* erase characters */
};

struct fpnt {
	struct fontrec *font;
	char *name;
	int point;
	int width;
};

extern struct fpnt *font_find();
