/* $Id: color.c,v 1.1 2001/05/12 23:02:15 cegger Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ggi/display/palemu.h>


int GGI_palemu_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	GGIDPRINT("display-palemu: SetPalVec(%d,%d)\n", start, len);
	
	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > 256)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	if (len > 0) {
		UPDATE_MOD(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
	}

	for (; len > 0; len--, start++, colormap++) {

		priv->palette[start] = *colormap;
		priv->lookup[start] = ggiMapColor(priv->parent, colormap);
	}
	
	return 0;
}
