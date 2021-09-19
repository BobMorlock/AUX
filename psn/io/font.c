/*	@(#)font.c	UniPlus VVV.2.1.1	*/
/*
 * (C) 1986 UniSoft Corp. of Berkeley CA
 *
 * UniPlus Source Code. This program is proprietary
 * with Unisoft Corporation and is not to be reproduced
 * or used in any manner except as authorized in
 * writing by Unisoft.
 */

#include "sys/video.h"
#include "sys/font.h"
extern font_clear();
extern font_scrollup();
extern font_scrolldown();
extern font_char();
extern font_erase();
extern font_invert();
extern font_delete();
extern font_insert();

struct fpnt *
font_find(font, point)
char *font;
{
	extern struct fpnt fonts[];
	struct fpnt *fp;

	for (fp = fonts;fp->font;fp++) {
		if (point == fp->point && strcmp(font, fp->name) == 0)
			return(fp);
	}
	return(0);
}

font_set(f, fxp, vp)
register struct video *vp;
register struct font *f;
register struct fpnt *fxp;
{
	register struct fontrec *fp;

	if (fxp == 0) {
		fxp = font_find("TTY", 12);
		if (fxp == 0)
			return;
	}
	fp = fxp->font;
	f->font_clear = font_clear;
	f->font_scrollup = font_scrollup;
	f->font_scrolldown = font_scrolldown;
	f->font_char = font_char;
	f->font_erase = font_erase;
	f->font_invert = font_invert;
	f->font_delete = font_delete;
	f->font_insert = font_insert;
	f->font_pnt = fp;
	f->font_screen = vp;
	f->font_height = fp->chHeight;
	f->font_theight = fp->chHeight+fp->leading;
	f->font_width = fp->fRectMax;
	if (f->font_width > fxp->width)
		f->font_width = fxp->width;
	f->font_leading = fp->leading;
	f->font_rowwords = ((unsigned int)fp->rowWords)<<4;
	f->font_bitimage = (unsigned char *)&fp->bitImage[0];
	f->font_loctable = &fp->bitImage[fp->rowWords*f->font_height];
	f->font_owtable = &fp->bitImage[fp->rowWords*f->font_height+
					 fp->lastChar + 2 - fp->firstChar];
	f->font_linewidth = (f->font_height+f->font_leading) * vp->video_mem_x;
	f->font_maxx = vp->video_scr_x / f->font_width;
	f->font_maxy = vp->video_scr_y / (f->font_height+f->font_leading);
}

#ifdef NOTDEF
font_debugd(i, v)
{
	printf("d%d = 0x%x\n", i&0xff, v);
}
font_debuga(i, v)
{
	printf("a%d = 0x%x\n", i&0xff, v);
}
#endif NOTDEF
