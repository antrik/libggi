/* $Id: color.c,v 1.2 2001/08/08 22:27:29 jtaylor Exp $
******************************************************************************

   SVGAlib target: palette driver

   Copyright (C) 1997 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 1997 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <string.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/svgalib.h>


int
GGI_svga_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	svga_priv *priv = LIBGGI_PRIVATE(vis);
	int maxlen = 1 << GT_DEPTH(LIBGGI_GT(vis));
	int *vgaptr;
	int i;

	LIBGGI_APPASSERT(colormap != NULL,
			 "GGI_svga_setpalvec() - colormap == NULL");

	if (start == GGI_PALETTE_DONTCARE) start = 0;

	if (start < 0 || start+len > maxlen) return -1;

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	vgaptr = priv->savepalette + start*3;

	/* vga_setpalvec() takes 6-bit r,g,b,
	   so we need to scale ggi_color's 16-bit values. */
	for (i = 0; i < len; i++) {
		*vgaptr = colormap->r >> 10;
		vgaptr++;
		*vgaptr = colormap->g >> 10;
		vgaptr++;
		*vgaptr = colormap->b >> 10;
		vgaptr++;
		colormap++;
	}

	if (!SVGA_PRIV(vis)->ismapped) return 0;

	vga_setpalvec(start, len, priv->savepalette + start*3);

	return 0;
}
