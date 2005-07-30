/* $Id: pixel.c,v 1.2 2005/07/30 11:40:00 cegger Exp $
******************************************************************************

   Linear 1 pixel drawing.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
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


#define PIXEL_RADDR(vis,x,y)  \
	((uint8_t *)LIBGGI_CURREAD(vis)+y*LIBGGI_FB_R_STRIDE(vis)+(x>>3))

#define PIXEL_WADDR(vis,x,y)  \
	((uint8_t *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>3))


int GGI_lin1_drawpixel_nc(ggi_visual *vis, int x, int y)
{
	if (LIBGGI_GC_FGCOLOR(vis) & 1) {
		*PIXEL_WADDR(vis,x,y) |=  (0x80 >> (x & 7));
	} else {
		*PIXEL_WADDR(vis,x,y) &= ~(0x80 >> (x & 7));
	}
	
	return 0;
}

int GGI_lin1_drawpixel(ggi_visual *vis, int x, int y)
{
	CHECKXY(vis, x, y);
 
	if (LIBGGI_GC_FGCOLOR(vis) & 1) {
		*PIXEL_WADDR(vis,x,y) |=  (0x80 >> (x & 7));
	} else {
		*PIXEL_WADDR(vis,x,y) &= ~(0x80 >> (x & 7));
	}
	
	return 0;
}

int GGI_lin1_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	if (col & 1) {
		*PIXEL_WADDR(vis,x,y) |=  (0x80 >> (x & 7));
	} else {
		*PIXEL_WADDR(vis,x,y) &= ~(0x80 >> (x & 7));
	}
	
	return 0;
}

int GGI_lin1_putpixel(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	CHECKXY(vis, x, y);
 
	if (col & 1) {
		*PIXEL_WADDR(vis,x,y) |=  (0x80 >> (x & 7));
	} else {
		*PIXEL_WADDR(vis,x,y) &= ~(0x80 >> (x & 7));
	}
	
	return 0;
}

int GGI_lin1_getpixel(ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{
	*pixel = !! (*PIXEL_RADDR(vis,x,y) & (0x80 >> (x & 7)));

	return 0;
}
