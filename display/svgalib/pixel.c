/* $Id: pixel.c,v 1.6 2008/01/20 19:26:41 pekberg Exp $
******************************************************************************

   SVGAlib target: pixels

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


int GGI_svga_drawpixel(struct ggi_visual *vis, int x, int y)
{
	CHECKXY(vis, x, y);
	
	vga_setcolor(LIBGGI_GC_FGCOLOR(vis));
	vga_drawpixel(x, y + vis->w_frame_num * LIBGGI_VIRTY(vis));

	return 0;
}

int GGI_svga_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	vga_setcolor(LIBGGI_GC_FGCOLOR(vis));
	vga_drawpixel(x, y + vis->w_frame_num * LIBGGI_VIRTY(vis));

	return 0;
}

int GGI_svga_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	vga_setcolor(col);
	vga_drawpixel(x, y + vis->w_frame_num * LIBGGI_VIRTY(vis));

	return 0;
}

int GGI_svga_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	CHECKXY(vis, x, y);

	vga_setcolor(col);
	vga_drawpixel(x, y + vis->w_frame_num * LIBGGI_VIRTY(vis));

	return 0;
}

int
GGI_svga_getpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{ 
	*pixel = 
		vga_getpixel(x, y+vis->r_frame_num * LIBGGI_VIRTY(vis));

	return 0;
}
