/* $Id: color.c,v 1.4 2004/11/13 15:56:22 cegger Exp $
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
#include <string.h>
#include <sys/ioctl.h>

#include <ggi/display/lcd823.h>


int
GGI_lcd823_setPalette(ggi_visual *vis, size_t start, size_t size, const ggi_color *colormap)
{
	uint16          *pal  = (uint16*)(LIBGGI_PAL(vis)->priv);
	size_t nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));
	size_t i;

	GGIDPRINT_COLOR("GGI_lcd823_setPalette(%p, %d, %d)\n", vis, start, size);

	if (start < 0 || start+size > nocols) {
		return -1;
	}

	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap, size*sizeof(ggi_color));

	for (i = 0; i < size; i ++) {
		     pal[i+start] =
			     ((colormap[i].b >> 12) & 0x000f) |
			     ((colormap[i].g >> 8) & 0x00f0) |
			     ((colormap[i].r >> 4) & 0x0f00);
	}
	ioctl(LIBGGI_FD(vis), 3, &pal);

	return 0;
}

size_t GGI_lcd823_getPrivSize(ggi_visual_t vis)
{
  return (256 * sizeof(uint16));
}
