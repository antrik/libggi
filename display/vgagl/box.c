/* $Id: box.c,v 1.3 2004/12/01 23:08:24 cegger Exp $
******************************************************************************

   SVGAlib target vgagl helper: box stubs

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include "vgaglvis.h"


int GGI_vgagl_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	LIBGGICLIP_XYWH(vis, x, y, w, h);

	gl_fillbox(x, y, w, h, LIBGGI_GC_FGCOLOR(vis));

	return 0;
}

int GGI_vgagl_putbox(ggi_visual *vis, int x, int y, int w, int h, const void *buffer)
{ 
	int pixelsize = (LIBGGI_PIXFMT(vis)->size+7)/8;
	int rowadd = w * pixelsize;
	const uint8 *buf = buffer;

	LIBGGICLIP_PUTBOX(vis, x, y, w, h, buf, rowadd, * pixelsize);

	gl_putbox(x, y, w, h, buf);

	return 0;
}

int GGI_vgagl_getbox(ggi_visual *vis, int x, int y, int w, int h, void *buffer)
{ 
	gl_getbox(x, y, w, h, buffer);

	return 0;
}
