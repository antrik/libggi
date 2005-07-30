/* $Id: vline.c,v 1.3 2005/07/30 11:40:02 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "lin4rlib.h"

/********************************/
/* draw/get/put a vertical line */
/********************************/

static inline void
do_drawvline(ggi_visual *vis, int x, int y, int h)
{
	uint8_t *ptr;
	int stride = LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 1) ? 4 : 0;
	uint8_t color = LIBGGI_GC_FGCOLOR(vis) << shift;
	uint8_t mask = 0x0f << shift;

	PREPARE_FB(vis);

	ptr = (uint8_t*)LIBGGI_CURWRITE(vis) + y*stride + x/2;

	for(; h > 0; h--, ptr += stride) {
		*ptr = color | (*ptr & mask);
	}
	
}

int GGI_lin4r_drawvline(ggi_visual *vis, int x, int y, int h)
{
	LIBGGICLIP_XYH(vis, x, y, h);
	
	do_drawvline(vis, x, y, h);

	return 0;
}

int GGI_lin4r_drawvline_nc(ggi_visual *vis,int x,int y,int h)
{
	do_drawvline(vis, x, y, h);
	
	return 0;
}

int GGI_lin4r_putvline(ggi_visual *vis,int x,int y,int h,const void *buffer)
{
	uint8_t *ptr;
	const uint8_t *buf8=(const uint8_t *)buffer;
	int stride=LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 0x01) << 2;
	uint8_t mask = 0x0f << shift;
	uint8_t antishift = shift ^ 4;

	LIBGGICLIP_XYH_BUFMOD(vis, x, y, h, buf8, /2);
	PREPARE_FB(vis);

	ptr=(uint8_t *)LIBGGI_CURWRITE(vis)+y*((stride+x)>>1);

	for(; h > 1; h-=2, ptr+=(stride<<1)) {
		*ptr=(*buf8 >> shift) | (*ptr & mask);
		*(ptr+stride) = (*(buf8++) << antishift)
			| (*(ptr+stride) & mask);
	}
	
	if (h) {
		*ptr=(*buf8 >> shift) | (*ptr & mask);
	}
		
	return 0;
}

int GGI_lin4r_getvline(ggi_visual *vis,int x,int y,int h,void *buffer)
{
	uint8_t *ptr,*buf8=(uint8_t *)buffer;
	int stride=LIBGGI_FB_W_STRIDE(vis);
	uint8_t shift = (x & 0x01) << 2;
	uint8_t mask = 0x0f << shift;
	uint8_t antishift = shift ^ 4;
	
	PREPARE_FB(vis);
	ptr = (uint8_t *)LIBGGI_CURREAD(vis)+y*((stride+x)>>1);

	/* Warning: unnecessary bit operations ahead! */
	for (; h > 1; h-=2, ptr+=stride<<1) {
		*buf8 = ((*ptr & mask) << shift)
			| ((*(ptr+stride) & mask) >> antishift);
	}
	
	/* Here we can't lazily stick the extra pixel into the buffer, since
	 * it might be off the screen. */
	if (h) {
		*buf8 = ((*ptr & mask) << shift);
	}
	
	return 0;
}
