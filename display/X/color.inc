/* $Id: color.inc,v 1.1 2001/05/12 23:01:55 cegger Exp $
******************************************************************************

   Color functions for the various X targets.

   Copyright (C) 1997 Andreas Beck      [becka@ggi-project.org]

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
#include <string.h>


/* Minimum number of colors needed before we use the `smart allocation'
 * method for choosing where to put colors with GGI_PALETTE_DONTCARE.
 */
#define COLOR_THRESHOLD(num,max)  1   /*  ((num)*4 >= (max)*3)  */


static int _ggi_smart_allocate(ggi_visual *vis, int len, ggi_color *cols)
{
	ggi_x_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_color X_pal[256];
	ggi_color G_pal[256];
	int i;
	
	if (len > 256) return -1;

	/* read X's palette */

	for (i=0; i < len; i++) {
		XColor xcol;

		ggLock(priv->xwin.x.xliblock);

		xcol.pixel = i;

		XQueryColor(priv->xwin.x.display,
			    DefaultColormap(priv->xwin.x.display, priv->xwin.x.screen), 
			    &xcol);

		ggUnlock(priv->xwin.x.xliblock);

		X_pal[i].r = xcol.red;
		X_pal[i].g = xcol.green;
		X_pal[i].b = xcol.blue;

		vis->palette[i] = G_pal[i] = cols[i];
	}

	_ggi_smart_match_palettes(vis->palette, len, X_pal, len);

	for (i=0; i < len; i++) {
		GGIDPRINT_COLOR("Smart alloc %03d: X=%02x%02x%02x "
		             " GGI=%02x%02x%02x  (orig: %02x%02x%02x)\n", i, 
			     X_pal[i].r >> 8, X_pal[i].g >> 8, 
			     X_pal[i].b >> 8, vis->palette[i].r >> 8, 
			     vis->palette[i].g >> 8, vis->palette[i].b >> 8,
			     G_pal[i].r >> 8, G_pal[i].g >> 8, 
			     G_pal[i].b >> 8);
	}

	priv->xwin.cmap_first = 0;
	priv->xwin.cmap_last  = len;

	return 0;
}

int GGI_X_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_x_priv *priv = LIBGGI_PRIVATE(vis);
	
	GGIDPRINT_COLOR("GGI_X_setpalvec(%p, %d, %d, {%d, %d, %d}) called\n",
			vis, start, len, colormap->r,colormap->g ,colormap->b);

	LIBGGI_APPASSERT(colormap != NULL,
			 "ggiSetPalette() called with NULL colormap!");

	if (start == GGI_PALETTE_DONTCARE) {
		if (len > priv->xwin.x.nocols) {
			return -1;
		}

		if (COLOR_THRESHOLD(len, priv->xwin.x.nocols)) {
			return _ggi_smart_allocate(vis, len, colormap);
		}

		start = priv->xwin.x.nocols - len;
	}

	if (start+len > priv->xwin.x.nocols || start < 0) return -1;

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	if (start < priv->xwin.cmap_first) {
		priv->xwin.cmap_first = start;
	}
	if (start+len > priv->xwin.cmap_last) {
		priv->xwin.cmap_last  = start+len;
	}

	GGIDPRINT_COLOR("X setpalvec success\n");

	return 0;
}
