/* $Id: color.c,v 1.1 2001/05/12 23:01:57 cegger Exp $
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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/aa.h>


int GGI_aa_setpalvec(ggi_visual *vis,int start,int len,ggi_color *colormap)
{
        ggi_aa_priv *priv = LIBGGI_PRIVATE(vis);
	int x;

	GGIDPRINT_COLOR("AA setpalette.\n");

	if (start == GGI_PALETTE_DONTCARE) start = 0;

	if (colormap==NULL || start+len > (1<<GT_DEPTH(LIBGGI_GT(vis)))) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

       	for (x=start; x<start+len; x++) {
		aa_setpalette(priv->pal, x, vis->palette[x].r>>8,
			      vis->palette[x].g>>8, 
			      vis->palette[x].b>>8 );
	}

	return 0;
}
