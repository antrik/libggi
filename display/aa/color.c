/* $Id: color.c,v 1.4 2003/12/13 21:12:02 mooz Exp $
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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/aa.h>
#include <string.h>


int GGI_aa_setPalette(ggi_visual_t vis,size_t start,size_t size, const ggi_color *colormap)
{
	aa_palette   *pal  = (aa_palette*)(LIBGGI_PAL(vis)->priv);
	ggi_color    *dest = LIBGGI_PAL(vis)->clut + start;
	ggi_color    *src  = (ggi_color*)colormap;
	size_t       end = start + size;

	GGIDPRINT_COLOR("AA setpalette.(%d,%d) %d\n",start,size,LIBGGI_PAL(vis)->size);
	
	for (; start<end; ++start, ++dest) {
		*dest = *(src++);
	
		aa_setpalette(*pal, start, dest->r>>8,
			                   dest->g>>8, 
			                   dest->b>>8 );
	}
		
	return 0;
}
