/* $Id: color.c,v 1.8 2004/11/14 15:47:46 cegger Exp $
******************************************************************************

   Display-palemu: color

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
#include <ggi/display/palemu.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int GGI_palemu_setPalette(ggi_visual_t vis, size_t start, size_t len, const ggi_color *colormap)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	ggi_color       *src  = (ggi_color*)colormap;
	size_t          end   = start + len;

 	GGIDPRINT("display-palemu: SetPalette(%d,%d)\n", start, len);
	
	if (start < 0 || start + len > 256) {
		return GGI_ENOSPACE;
	}

	memcpy(LIBGGI_PAL(vis)->clut.data+start, src, len*sizeof(ggi_color));
	if (end > start) {
		UPDATE_MOD(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
	}

	for (; start < end; ++start, ++src) {
		priv->palette[start] = *src;
		priv->lookup[start] = ggiMapColor(priv->parent, src);
	}
	
	return 0;
}
