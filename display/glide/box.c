/* $Id: box.c,v 1.2 2002/09/08 21:37:45 soyt Exp $
******************************************************************************

   LibGGI GLIDE target - Box functions

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

#include <stdlib.h>
#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/glide.h>

int
GGI_glide_fillscreen(ggi_visual *vis)
{
	grBufferClear(LIBGGI_GC(vis)->fg_color, 0, GR_WDEPTHVALUE_FARTHEST);
	
	return 0;
}

int GGI_glide_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	glide_priv *priv = GLIDE_PRIV(vis);
	GrVertex br = priv->fgvertex;
	GrVertex var = priv->fgvertex;
	
	priv->fgvertex.x = x;
	priv->fgvertex.y = y;
	br.x = x+w;
	br.y = y+h;

	var.x = x;
	var.y = y+h;
	guDrawTriangleWithClip(&priv->fgvertex, &br, &var);
	var.x = x+w;
	var.y = y;
	guDrawTriangleWithClip(&priv->fgvertex, &br, &var);

	return 0;

}

int GGI_glide_putbox(ggi_visual *vis, int x, int y, int w, int h, void *data)
{
	glide_priv *priv = GLIDE_PRIV(vis);
	int bpp = priv->bytes_per_pixel;

	if (y< LIBGGI_GC(vis)->cliptl.y) {
		int diff=LIBGGI_GC(vis)->cliptl.y-y;
		y+=diff;
		h-=diff;
		data = (uint8 *)data + diff*bpp*w;
	}
	if (y+h>LIBGGI_GC(vis)->clipbr.y) {
		h=LIBGGI_GC(vis)->clipbr.y-y;
	}
	if (h<1) return 0;

	if (x< LIBGGI_GC(vis)->cliptl.x) {
		int diff=LIBGGI_GC(vis)->cliptl.x-x;
		x+=diff;
		w-=diff;
		data = (uint8*)data + diff*bpp;
	}
	if (x+w>LIBGGI_GC(vis)->clipbr.x) {
		w=LIBGGI_GC(vis)->clipbr.x-x;
	}
	if (w<1) return 0;

	grLfbWriteRegion(priv->writebuf, x, y,
			 priv->src_format,
			 w, h, bpp*w, data);

	return 0;
}

int GGI_glide_getbox(ggi_visual *vis, int x, int y, int w, int h, void *data)
{
	glide_priv *priv = GLIDE_PRIV(vis);
	grLfbReadRegion(priv->readbuf, x, y, w, h,
			priv->bytes_per_pixel*w, data);
	return 0;
}

#if 0 /* Doesn't seem to make any difference... */
int GGI_glide_copybox(ggi_visual *vis, int x, int y, int w, int h, int nx, int ny)
{
	glide_priv *priv = GLIDE_PRIV(vis);
	void *chunkbuf;

	/* Clipping */

	if (nx < LIBGGI_GC(vis)->cliptl.x) {
		int diff = LIBGGI_GC(vis)->cliptl.x - nx;
		x  += diff;
		nx += diff;
		w  -= diff;
	}
	if (ny < LIBGGI_GC(vis)->cliptl.y) {
		int diff = LIBGGI_GC(vis)->cliptl.y - ny;
		y  += diff;
		ny += diff;
		h  -= diff;
	}
	if (nx+w > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - nx;
	}
	if (ny+h > LIBGGI_GC(vis)->clipbr.y) {
		h = LIBGGI_GC(vis)->clipbr.y - ny;
	}

     	if ((h <= 0) || (w <= 0)) {
		return 0;
	}
	
	chunkbuf = malloc(w*h*priv->bytes_per_pixel);
	if (chunkbuf == NULL) return GGI_ENOMEM;
	
	grLfbReadRegion(priv->readbuf, x, y,
			w, h, priv->bytes_per_pixel*w,
			chunkbuf);
	grLfbWriteRegion(priv->writebuf, nx, ny,
			 priv->src_format,
			 w, h, priv->bytes_per_pixel*w,
			 chunkbuf);

	free(chunkbuf);
	
	return 0;
}
#endif
