/* $Id: hline.c,v 1.7 2005/07/30 10:58:27 cegger Exp $
******************************************************************************

   SVGAlib target: horizontal lines

   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <ggi/display/svgalib.h>


int GGI_svga_drawhline(ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	vga_setcolor(LIBGGI_GC_FGCOLOR(vis));
	y += vis->w_frame_num * LIBGGI_VIRTY(vis);
	vga_drawline(x,y,x+w-1,y);

	return 0;
}

int GGI_svga_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	vga_setcolor(LIBGGI_GC_FGCOLOR(vis));
	y += vis->w_frame_num * LIBGGI_VIRTY(vis);
	vga_drawline(x,y,x+w-1,y);

	return 0;
}

int GGI_svga_puthline(ggi_visual *vis,int x,int y,int w,const void *buffer)
{
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	const uint8_t *buf = buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf, *pixelsize);

	y += vis->w_frame_num * LIBGGI_VIRTY(vis);
	if (SVGA_PRIV(vis)->ismodex && x%4 != 0) {
		/* ModeX is always 8bpp */
		for (;x%4 != 0; x++,w--) {
			ggiPutPixel(vis, x, y, *buf);
			buf++;
		}
		for (;w%4 != 0; w--) {
			ggiPutPixel(vis, x, y, *(buf+w));
		}
	}
        vga_drawscansegment((uint8_t *)buf, x, y, w*pixelsize);

	return 0;
}
