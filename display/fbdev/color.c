/* $Id: color.c,v 1.1 2001/05/12 23:02:03 cegger Exp $
******************************************************************************

   Display-FBDEV

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <linux/fb.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>


int
GGI_fbdev_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_fbdev_priv *priv = LIBGGI_PRIVATE(vis);
	struct fb_cmap cmap;
	int nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));

	GGIDPRINT_COLOR("display-fbdev: SetPalVec(%d,%d)\n", start, len);
	
	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > nocols)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	if (!priv->ismapped) return 0;

	cmap.start  = start;
	cmap.len    = len;
	cmap.red    = &priv->reds[start];
	cmap.green  = &priv->greens[start];
	cmap.blue   = &priv->blues[start];
	cmap.transp = NULL;

	for (; len > 0; start++, colormap++, len--) {
		priv->reds[start]   = colormap->r;
		priv->greens[start] = colormap->g;
		priv->blues[start]  = colormap->b;
	}

	if (fbdev_doioctl(vis, FBIOPUTCMAP, &cmap) < 0) {
		GGIDPRINT_COLOR("display-fbdev: PUTCMAP failed.");
		return -1;
	}

	return 0;
}
