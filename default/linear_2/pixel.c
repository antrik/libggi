/* $Id: pixel.c,v 1.1 2001/05/12 23:01:40 cegger Exp $
******************************************************************************

   Linear 2 pixel drawing (high-pair-left)

   Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]

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

#include "lin2lib.h"


#define PIXEL_RADDR(vis,x,y)  \
	((uint8 *)LIBGGI_CURREAD(vis)+y*LIBGGI_FB_R_STRIDE(vis)+(x>>2))

#define PIXEL_WADDR(vis,x,y)  \
	((uint8 *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>2))


int GGI_lin2_putpixel_nc(ggi_visual *vis, int x, int y, ggi_pixel col)
{
	int shift = (3 - (x & 3)) << 1;
	
	*PIXEL_WADDR(vis,x,y) &= ~(0x03 << shift);
	*PIXEL_WADDR(vis,x,y) |= (col & 0x03) << shift;
	
	return 0;
}

int GGI_lin2_getpixel(ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{
	int shift = (3 - (x & 3)) << 1;

	*pixel = (*PIXEL_RADDR(vis,x,y) >> shift) & 0x03;

	return 0;
}
