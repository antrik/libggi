/* $Id: color.c,v 1.1 2001/05/12 23:02:12 cegger Exp $
******************************************************************************

   Display-monotext: color management

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include <ggi/display/monotext.h>


int GGI_monotext_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_monotext_priv *priv = LIBGGI_PRIVATE(vis);

	GGIDPRINT("display-monotext: SetPalVec(%d,%d)\n", start, len);
	
	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > 256)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));
	
	if (len > 0) {
		UPDATE_MOD(priv, 0, 0, priv->size.x, priv->size.y);
	}

	for (; len > 0; len--, start++, colormap++) {

		int r = (colormap->r >> 11) & 0x1f;
		int g = (colormap->g >> 11) & 0x1f;
		int b = (colormap->b >> 11) & 0x1f;

		priv->colormap[start] = *colormap;

		priv->greymap[start] = 
			priv->rgb_to_grey[(r << 10) | (g << 5) | b];
	}
	
	UPDATE_SYNC;
	return 0;
}



