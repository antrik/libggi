/* $Id: line.c,v 1.5 2006/03/22 20:22:27 cegger Exp $
******************************************************************************

   LibGGI GLIDE target - Line functions

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/glide.h>


int
GGI_glide_drawline(struct ggi_visual *vis,int x,int y,int xe,int ye)
{
	GrVertex end = GLIDE_PRIV(vis)->fgvertex;
	
	GLIDE_PRIV(vis)->fgvertex.x = x;
	GLIDE_PRIV(vis)->fgvertex.y = y;
	end.x = xe;
	end.y = ye;
	
	grDrawLine(&GLIDE_PRIV(vis)->fgvertex, &end);

	return 0;
}

int
GGI_glide_drawhline(struct ggi_visual *vis,int x,int y,int w)
{
	GrVertex end = GLIDE_PRIV(vis)->fgvertex;
	
	GLIDE_PRIV(vis)->fgvertex.x = x;
	GLIDE_PRIV(vis)->fgvertex.y = y;
	end.x = x+w;
	end.y = y;
	
	grDrawLine(&GLIDE_PRIV(vis)->fgvertex, &end);

	return 0;
}

int
GGI_glide_drawvline(struct ggi_visual *vis,int x,int y,int h)
{
	GrVertex end = GLIDE_PRIV(vis)->fgvertex;
	
	GLIDE_PRIV(vis)->fgvertex.x = x;
	GLIDE_PRIV(vis)->fgvertex.y = y;
	end.x = x;
	end.y = y+h;
	
	grDrawLine(&GLIDE_PRIV(vis)->fgvertex, &end);

	return 0;
}

int
GGI_glide_puthline(struct ggi_visual *vis, int x, int y, int w, const void *data)
{
	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) {
		return 0;
	}
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		int diff=(LIBGGI_GC(vis)->cliptl.x)-x;
		x+=diff;
		w-=diff;
		data=((const uint8_t *)data)+diff*GLIDE_PRIV(vis)->bytes_per_pixel;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	if (w<1) return 0;

	grLfbWriteRegion(GLIDE_PRIV(vis)->writebuf, x, y,
			 GLIDE_PRIV(vis)->src_format,
			 w, 1, GLIDE_PRIV(vis)->bytes_per_pixel*w, data);

	return 0;
}

int
GGI_glide_putvline(struct ggi_visual *vis, int x, int y, int h, const void *data)
{
	/* Clipping */
	if (x<(LIBGGI_GC(vis)->cliptl.x) || x>=(LIBGGI_GC(vis)->clipbr.x)) {
		return 0;
	}
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y+=diff;
		h-=diff;
		data=((const uint8_t *)data)+diff*GLIDE_PRIV(vis)->bytes_per_pixel;
	}
	if (y+h>(LIBGGI_GC(vis)->clipbr.y)) {
		h=(LIBGGI_GC(vis)->clipbr.y)-y;
	}
	if (h<1) return 0;

	grLfbWriteRegion(GLIDE_PRIV(vis)->writebuf, x, y,
			 GLIDE_PRIV(vis)->src_format,
			 1, h, GLIDE_PRIV(vis)->bytes_per_pixel, data);

	return 0;
}

int
GGI_glide_gethline(struct ggi_visual *vis, int x, int y, int w, void *data)
{
	grLfbReadRegion(GLIDE_PRIV(vis)->readbuf, x, y,
			 w, 1, GLIDE_PRIV(vis)->bytes_per_pixel*w, data);
	return 0;
}

int
GGI_glide_getvline(struct ggi_visual *vis, int x, int y, int h, void *data)
{
	grLfbReadRegion(GLIDE_PRIV(vis)->readbuf, x, y,
			 1, h, GLIDE_PRIV(vis)->bytes_per_pixel, data);

	return 0;
}
