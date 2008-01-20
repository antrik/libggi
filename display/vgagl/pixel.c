/* $Id: pixel.c,v 1.4 2008/01/20 19:26:42 pekberg Exp $
******************************************************************************

   SVGAlib target vgagl helper: pixels

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
#include "vgaglvis.h"


int GGI_vgagl_drawpixel_nc(struct ggi_visual *vis, int x, int y)
{
	gl_setpixel(x, y, LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgagl_drawpixel(struct ggi_visual *vis, int x, int y)
{
	CHECKXY(vis, x, y);
	
	gl_setpixel(x, y, LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgagl_putpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	gl_setpixel(x, y, col);

	return 0;
}

int GGI_vgagl_putpixel(struct ggi_visual *vis, int x, int y, ggi_pixel col)
{ 
	CHECKXY(vis, x, y);

	gl_setpixel(x, y, col);

	return 0;
}

int
GGI_vgagl_getpixel_nc(struct ggi_visual *vis, int x, int y, ggi_pixel *pixel)
{ 
	*pixel = gl_getpixel(x, y);
	
	return 0;
}
