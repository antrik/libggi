/* $Id: gc.c,v 1.5 2006/03/11 18:49:12 soyt Exp $
******************************************************************************

   Graphics library for GGI. GC handling

   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
  
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
#include <ggi/internal/internal.h>

int ggiSetGCForeground(ggi_visual_t v,ggi_pixel color)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIBGGI_GC(vis)->fg_color=color;
	LIBGGI_GC(vis)->version++;

	if (vis->opgc->gcchanged != NULL)
		vis->opgc->gcchanged(vis, GGI_GCCHANGED_FG);

	return 0;
}

int ggiSetGCBackground(ggi_visual_t v,ggi_pixel color)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	LIBGGI_GC(vis)->bg_color=color;
	LIBGGI_GC(vis)->version++;

	if (vis->opgc->gcchanged != NULL)
		vis->opgc->gcchanged(vis, GGI_GCCHANGED_BG);

	return 0;
}

int ggiGetGCForeground(ggi_visual_t v,ggi_pixel *color)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	*color=LIBGGI_GC(vis)->fg_color;
	return 0;
}

int ggiGetGCBackground(ggi_visual_t v,ggi_pixel *color)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	*color=LIBGGI_GC(vis)->bg_color;
	return 0;
}

int ggiSetGCClipping(ggi_visual_t v,int left,int top,int right,int bottom)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	if ((left < 0) || (top < 0) || 
	    (right  > LIBGGI_VIRTX(vis)) || 
	    (bottom > LIBGGI_VIRTY(vis)) ||
	    (left>right) || 
	    (top>bottom)) {
		return GGI_ENOSPACE;
	}

	LIBGGI_GC(vis)->cliptl.x = left;
	LIBGGI_GC(vis)->cliptl.y = top;
	LIBGGI_GC(vis)->clipbr.x = right;
	LIBGGI_GC(vis)->clipbr.y = bottom;
	LIBGGI_GC(vis)->version++;

	if (vis->opgc->gcchanged != NULL) {
		vis->opgc->gcchanged(vis, GGI_GCCHANGED_CLIP);
	}

	return 0;
}


int ggiGetGCClipping(ggi_visual_t v,int *left,int *top,int *right,int *bottom)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	*left   = LIBGGI_GC(vis)->cliptl.x;
	*top    = LIBGGI_GC(vis)->cliptl.y;
	*right  = LIBGGI_GC(vis)->clipbr.x;
	*bottom = LIBGGI_GC(vis)->clipbr.y;

	return 0;
}
