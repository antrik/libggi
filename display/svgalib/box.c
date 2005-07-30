/* $Id: box.c,v 1.7 2005/07/30 10:58:27 cegger Exp $
******************************************************************************

   SVGAlib target: vgagl box implementation

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


int GGI_svga_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	int x2;
  
	LIBGGICLIP_XYWH(vis, x, y, w, h);

	vga_setcolor(LIBGGI_GC_FGCOLOR(vis));
	x2 = w + x - 1;
	y += vis->w_frame_num * LIBGGI_VIRTY(vis);
	while (h--) {
		vga_drawline(x, y, x2, y);
		y++;
	}
	return 0;
}

int
GGI_svga_putbox(ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	int rowadd = w * pixelsize;
	const uint8_t *buf = buffer;

	LIBGGICLIP_PUTBOX(vis, x, y, w, h, buf, rowadd, * pixelsize);

	y += vis->w_frame_num * LIBGGI_VIRTY(vis);
	if (SVGA_PRIV(vis)->ismodex
		&& w%4 == 0
		&& x%4 == 0) {
		vga_copytoplanar256((uint8_t *)buf, rowadd,
				    ((y*LIBGGI_VIRTX(vis) + x))/4,
				    LIBGGI_VIRTX(vis)/4, w, h);
	} else {
		while (h--) {
			ggiPutHLine(vis, x, y, w, buf);
			y++;
			buf += rowadd;
		}
	}
	
	return 0;
}
