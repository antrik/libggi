/* $Id: pixel.c,v 1.3 2002/09/29 19:27:42 skids Exp $
******************************************************************************

   LibGGI GLIDE target - Pixel functions

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
#include <ggi/display/glide.h>
#include "../../default/color/color.h"


int
GGI_glide_drawpixel(ggi_visual *vis,int x,int y)
{
	GLIDE_PRIV(vis)->fgvertex.x = x;
	GLIDE_PRIV(vis)->fgvertex.y = y;

	grDrawPoint(&GLIDE_PRIV(vis)->fgvertex);
	
	return 0;
}

int
GGI_glide_putpixel(ggi_visual *vis, int x, int y, ggi_pixel pix)
{
	color_truepriv *colorpriv = vis->colorpriv;
	GrVertex vert;

	vert.x = x;
	vert.y = y;
	vert.r = SSHIFT(LIBGGI_GC(vis)->fg_color
			& LIBGGI_PIXFMT(vis)->red_mask,
			colorpriv->red_unmap - 8);
	vert.g = SSHIFT(LIBGGI_GC(vis)->fg_color
			& LIBGGI_PIXFMT(vis)->green_mask,
			colorpriv->green_unmap - 8);
	vert.b = SSHIFT(LIBGGI_GC(vis)->fg_color
			& LIBGGI_PIXFMT(vis)->blue_mask,
			colorpriv->blue_unmap - 8);
	vert.a = 255;

	grDrawPoint(&vert);

	return 0;
}

int
GGI_glide_getpixel(ggi_visual *vis,int x,int y,ggi_pixel *pixel)
{
	/* This is needed to handle big/little endianess */
	switch (GLIDE_PRIV(vis)->bytes_per_pixel) {
	case 4: {
		uint32 pix;
		grLfbReadRegion(GLIDE_PRIV(vis)->readbuf, x, y,
				1, 1, 1, &pix);
		*pixel = pix;
		break;
	}
	case 3: {
		/* FIXME? */
		uint32 pix;
		grLfbReadRegion(GLIDE_PRIV(vis)->readbuf, x, y,
				1, 1, 1, &pix);
		*pixel = pix;
		break;
	}
	case 2: {
		uint16 pix;
		grLfbReadRegion(GLIDE_PRIV(vis)->readbuf, x, y,
				1, 1, 1, &pix);
		*pixel = pix;
		break;
	}
	}
	return 0;
}
