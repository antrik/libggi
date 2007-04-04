/* $Id: hline.c,v 1.7 2007/04/04 13:58:21 ggibecka Exp $
******************************************************************************

   Graphics library for GGI. Horizontal lines.

   Copyright (C) 1995 Andreas Beck [becka@ggi-project.org]

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

#include <string.h>

#include "lin8lib.h"


int GGI_lin8_drawhline(struct ggi_visual *vis,int x,int y,int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);
	PREPARE_FB(vis);

	memset((uint8_t*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x,
	       (signed)LIBGGI_GC_FGCOLOR(vis), (size_t)(w));

	return 0;
}

int GGI_lin8_drawhline_nc(struct ggi_visual *vis,int x,int y,int w)
{
	PREPARE_FB(vis);

        memset((uint8_t*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x,
	       (signed)LIBGGI_GC_FGCOLOR(vis), (size_t)(w));

	return 0;
}

int GGI_lin8_puthline(struct ggi_visual *vis,int x,int y,int w,const void *buffer)
{
	const uint8_t *buf8 = buffer; 
	uint8_t *mem;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf8, *1);
	PREPARE_FB(vis);

	mem = (uint8_t*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x;
	memcpy(mem, buf8, (size_t)(w));

	return 0;
}

int GGI_lin8_gethline_nc(struct ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8_t *mem;

	PREPARE_FB(vis);

	mem = (uint8_t *)LIBGGI_CURREAD(vis) + y*LIBGGI_FB_R_STRIDE(vis) + x;

	memcpy(buffer, mem, (size_t)(w));

	return 0;
}

int GGI_lin8_gethline(struct ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8_t *mem;

	PREPARE_FB(vis);

	/* clip to virtual size */
	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;	/* x is negative. w will _de_crease */
		buffer=(void *)((uint8_t *)buffer-x);
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<0) return 0;

	mem = (uint8_t *)LIBGGI_CURREAD(vis) + y*LIBGGI_FB_R_STRIDE(vis) + x;

	memcpy(buffer, mem, (size_t)(w));

	return 0;
}
