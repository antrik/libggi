/* $Id: color.c,v 1.1 2001/05/12 23:02:08 cegger Exp $
******************************************************************************

   Display-lcd823

   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include <ggi/display/lcd823.h>


int
GGI_lcd823_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_lcd823_priv *priv = LIBGGI_PRIVATE(vis);
	int nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));
	int i;

	GGIDPRINT_COLOR("GGI_lcd823_setpalvec(%p, %d, %d)\n", vis, start, len);

	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > nocols)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	for (i = 0; i < len; i ++) {
		     priv->pal[i+start] =
			     ((colormap[i].b >> 12) & 0x000f) |
			     ((colormap[i].g >> 8) & 0x00f0) |
			     ((colormap[i].r >> 4) & 0x0f00);
	}
	ioctl(LIBGGI_FD(vis), 3, &priv->pal);

	return 0;
}
