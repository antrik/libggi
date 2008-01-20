/* $Id: pixel.c,v 1.4 2008/01/20 19:26:37 pekberg Exp $
******************************************************************************

   Graphics library for GGI. Pixels.

   Copyright (C) 1998 Andrew Apted     [andrew.apted@ggi-project.org]

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

#include "t32lib.h"


#define PIXEL_RADDR(vis, x, y)  \
	((uint32_t *) LIBGGI_CURREAD(vis) + y*LIBGGI_FB_R_STRIDE(vis)/sizeof(uint32_t) + x)
#define PIXEL_WADDR(vis, x, y)  \
	((uint32_t *) LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis)/sizeof(uint32_t) + x)

int GGI_t32_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	*PIXEL_WADDR(vis, x, y) = LIBGGI_GC_FGCOLOR(vis);

	return 0;
}

int GGI_t32_drawpixel(struct ggi_visual *vis, int x, int y)
{
	CHECKXY(vis,x,y);
 
	*PIXEL_WADDR(vis, x, y) = LIBGGI_GC_FGCOLOR(vis);

	return 0;
}

int GGI_t32_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	*PIXEL_WADDR(vis, x, y) = col;

	return 0;
}

int GGI_t32_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	CHECKXY(vis,x,y);

	*PIXEL_WADDR(vis, x, y) = col;

	return 0;
}

int
GGI_t32_getpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{ 
	*pixel = *PIXEL_RADDR(vis, x, y);

	return 0;
}
