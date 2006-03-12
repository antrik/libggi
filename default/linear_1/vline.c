/* $Id: vline.c,v 1.4 2006/03/12 23:15:06 soyt Exp $
******************************************************************************

   Linear 1 vertical lines.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************************
*/

#include "lin1lib.h"


int GGI_lin1_drawvline_nc(struct ggi_visual *vis,int x,int y,int height)
{
	uint8_t *adr;
	int i,sw,bm;

	PREPARE_FB(vis);

	bm=(0x80>>(x&7));
	sw=LIBGGI_FB_W_STRIDE(vis);
	adr=((uint8_t *)(LIBGGI_CURWRITE(vis)));
	adr+=(x>>3)+y*sw;

	if(LIBGGI_GC_FGCOLOR(vis)&1) 
		for (i=height;i--;adr+=sw) *adr |= bm;
	else
		for (i=height;i--;adr+=sw) *adr &= ~bm;
  
	return 0;
}

int GGI_lin1_putvline(struct ggi_visual *vis,int x,int y,int height,const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff=(const uint8_t *)buffer;
	int mask=0x80,sw,i,bm;

	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y     +=diff;
		height-=diff;
		buff +=diff>>3;
		mask>>=diff&7;
	}
	if (y+height>(LIBGGI_GC(vis)->clipbr.y)) {
		height=(LIBGGI_GC(vis)->clipbr.y)-y;
	}

	PREPARE_FB(vis);

	bm=(0x80>>(x&7));
	sw=LIBGGI_FB_W_STRIDE(vis);
	adr=((uint8_t *)(LIBGGI_CURWRITE(vis)));
	adr+=(x>>3)+y*sw;

	for (i=0;i<height;i++,adr+=sw) {
		if (*buff & mask) 
			*adr |= bm;
		else
			*adr &= ~bm;
		mask >>= 1;
		if (mask==0) {
			mask=0x80;
			buff++;
		}
	}

  	return 0;
}

int GGI_lin1_getvline(struct ggi_visual *vis,int x,int y,int height,void *buffer)
{ 
	uint8_t *adr,*buff=(uint8_t *)buffer;
	int mask,sw,i,bm;

	PREPARE_FB(vis);

	sw=LIBGGI_FB_R_STRIDE(vis);
	adr=((uint8_t *)(LIBGGI_CURREAD(vis)));
	adr+=(x>>3)+y*sw;

	bm=(0x80>>(x&7));
	mask=0x80;
	for (i=0;i<height;i++,adr+=sw) {
		*buff |= (*adr & bm) ? mask : 0;
		mask>>=1;
		if (mask==0) {
			mask=0x80;
			buff++;
		}
	}

  	return 0;
}
