/* $Id: color.c,v 1.12 2006/12/31 08:43:21 cegger Exp $
******************************************************************************

   Graphics library for GGI.  Palette functions for AA target.

   Copyright (C) 1997 Andreas Beck    [becka@ggi-project.org]

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
#include <string.h>
#include <ggi/display/aa.h>
#include <ggi/internal/ggi_debug.h>


int GGI_aa_setPalette(struct ggi_visual *vis,size_t start,size_t size, const ggi_color *colormap)
{
	aa_palette   *pal  = (aa_palette*)(LIBGGI_PAL(vis)->priv);
	ggi_color    *dest = LIBGGI_PAL(vis)->clut.data + start;
	const ggi_color    *src  = colormap;
	size_t        end = start + size;

	DPRINT_COLOR("AA setpalette.(%d,%d) %d\n",
			start,size,LIBGGI_PAL(vis)->clut.size);
	
	for (; start<end; ++start, ++dest) {
		*dest = *(src++);
	
		aa_setpalette(*pal, start, dest->r>>8,
			                   dest->g>>8, 
			                   dest->b>>8 );
	}
		
	return 0;
}
