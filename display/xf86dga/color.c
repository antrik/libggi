/* $Id: color.c,v 1.4 2003/12/20 11:09:24 cegger Exp $
******************************************************************************

   XF86DGA target: color

   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
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

#include <string.h>
#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/xf86dga.h>


int GGI_xf86dga_setPalette(ggi_visual * vis, size_t start, size_t size,
			   const ggi_color * colormap)
{
	ggidga_priv *priv = DGA_PRIV(vis);
	XColor xcol;
	size_t i;

	size_t end = start + size;

	GGIDPRINT_COLOR
	    ("GGI_xf86dga_setPalette(%p, %d, %d, {%d, %d, %d}) called\n",
	     vis, start, size, colormap->r, colormap->g, colormap->b);

	if (colormap == NULL || end > (size_t) (DGA_PRIV(vis)->x.nocols)) {
		return -1;
	}
	/* if */
	LIBGGI_PAL(vis)->size = size;
	memcpy(LIBGGI_PAL(vis)->clut + start, colormap,
	       size * sizeof(ggi_color));

	if (start < LIBGGI_PAL(vis)->rw_start) {
		LIBGGI_PAL(vis)->rw_start = start;
	}
	if (end > LIBGGI_PAL(vis)->rw_stop) {
		LIBGGI_PAL(vis)->rw_stop = end;
	}

	ggLock(priv->x.xliblock);

	for (i = LIBGGI_PAL(vis)->rw_start; i < LIBGGI_PAL(vis)->rw_stop;
	     ++i) {
		xcol.red = LIBGGI_PAL(vis)->clut[i].r;
		xcol.green = LIBGGI_PAL(vis)->clut[i].g;
		xcol.blue = LIBGGI_PAL(vis)->clut[i].b;
		xcol.pixel = i;
		xcol.flags = DoRed | DoGreen | DoBlue;
		XStoreColor(priv->x.display, priv->x.cmap, &xcol);
		XStoreColor(priv->x.display, priv->cmap2, &xcol);
	}

	/* Work around a nasty DGA bug */
	if (priv->activecmap) {
		_ggi_XF86DGAInstallColormap(priv->x.display,
					    priv->x.screen, priv->x.cmap);
	} else {
		_ggi_XF86DGAInstallColormap(priv->x.display,
					    priv->x.screen, priv->cmap2);
	}

	priv->activecmap = !priv->activecmap;

	ggUnlock(priv->x.xliblock);

	return 0;
}
