/* $Id: color.c,v 1.2 2002/09/08 21:37:47 soyt Exp $
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

int GGI_xf86dga_setpalvec(ggi_visual *vis, int start, int len,
			  ggi_color *colormap) 
{
	ggidga_priv *priv = LIBGGI_PRIVATE(vis);
	XColor xcol;
	int i;

	if (start == GGI_PALETTE_DONTCARE) start = 0;

	if (colormap == NULL || start+len > DGA_PRIV(vis)->x.nocols) return -1;

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	ggLock(priv->x.xliblock);

	for(i = start; i<start+len; i++) {
		xcol.red  =vis->palette[i].r;
		xcol.green=vis->palette[i].g;
		xcol.blue =vis->palette[i].b;
		xcol.pixel=i;
		xcol.flags= DoRed | DoGreen | DoBlue ;
		XStoreColor(priv->x.display, priv->x.cmap,&xcol);
		XStoreColor(priv->x.display, priv->cmap2,&xcol);
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
