/* $Id: hline.c,v 1.1 2001/05/12 23:02:42 cegger Exp $
******************************************************************************

   SVGAlib target vgagl helper: horizontal lines

   Copyright (C) 1998-1999 Marcus Sundberg   [marcus@ggi-project.org]

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

#include <ggi/internal/ggi-dl.h>
#include "vgaglvis.h"


int GGI_vgagl_drawhline(ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	gl_hline(x, y, x+w-1, LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgagl_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	gl_hline(x, y, x+w-1, LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgagl_puthline(ggi_visual *vis, int x, int y, int w, void *buffer)
{ 
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	uint8 *buf = buffer;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf, *pixelsize);

        gl_putbox(x, y, w, 1, buffer);

	return 0;
}

int GGI_vgagl_gethline(ggi_visual *vis, int x, int y, int w, void *buffer)
{ 
	gl_getbox(x, y, w, 1, buffer);

	return 0;
}
