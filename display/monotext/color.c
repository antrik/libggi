/* $Id: color.c,v 1.6 2004/10/31 14:25:02 cegger Exp $
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

#include "config.h"
#include <ggi/display/monotext.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int GGI_monotext_setPalette(ggi_visual_t vis, size_t start, size_t size, const ggi_color *colormap)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);
	ggi_color *src = (ggi_color*)colormap;
	size_t    end = start + size - 1;

 	GGIDPRINT("display-monotext: SetPalette(%d,%d)\n", start, size);
		
	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap, size*sizeof(ggi_color));
		
	if (end > start) {
		UPDATE_MOD(priv, 0, 0, priv->size.x, priv->size.y);
	}
	
	for (; start<=end; ++start, ++src) {

		int r = (src->r >> 11) & 0x1f;
 		int g = (src->g >> 11) & 0x1f;
 		int b = (src->b >> 11) & 0x1f;
			
		priv->colormap[start] = *src;

		priv->greymap[start] = priv->rgb_to_grey[(r << 10) | (g << 5) | b];
	}
	
	UPDATE_SYNC;
	return 0;
}
