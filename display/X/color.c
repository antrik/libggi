/* $Id: color.c,v 1.9 2003/09/16 21:39:11 cegger Exp $
******************************************************************************

   Color functions for the X target.

   Copyright (C) 1997 Andreas Beck      [becka@ggi-project.org]
   Copyright (C) 1998 Marcus Sundberg   [marcus@ggi-project.org]
   Copyright (C) 2002 Brian S. Julin    [bri@tull.umassp.edu]

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
#include <ggi/internal/ggi-dl.h>
#include <string.h>
#include <ggi/display/x.h>

/* Minimum number of colors needed before we use the `smart allocation'
 * method for choosing where to put colors with GGI_PALETTE_DONTCARE.
 */
#define COLOR_THRESHOLD(num,max)  1   /*  ((num)*4 >= (max)*3)  */

static int _ggi_smart_allocate(ggi_visual *vis, int len, ggi_color *cols)
{
	ggi_x_priv *priv;
	ggi_color X_pal[256];
	ggi_color G_pal[256];
	int i, screen;

	priv = GGIX_PRIV(vis);
	screen = priv->vilist[priv->viidx].vi->screen;
			
	if (len > 256) return -1;

	/* read X's palette */

	for (i=0; i < len; i++) {
		XColor xcol;

		ggLock(priv->xliblock);

		xcol.pixel = i;
		XQueryColor(priv->disp,
			    DefaultColormap(priv->disp, screen),
			    &xcol);

		ggUnlock(priv->xliblock);

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

	priv->cmap_first = 0;
	priv->cmap_last  = len;

	return 0;
}

int _ggi_x_flush_cmap (ggi_visual *vis) {
	ggi_x_priv *priv;

	priv = GGIX_PRIV(vis);

	LIBGGI_ASSERT(priv->cmap, "No cmap!\n");

	if (priv->cmap_first >= priv->cmap_last) return 0;
	if (vis->palette) {
		int x;
		XColor xcol;

		for (x = priv->cmap_first; x < priv->cmap_last; x++) {
			xcol.red   = vis->palette[x].r;
			xcol.green = vis->palette[x].g;
			xcol.blue  = vis->palette[x].b;
			xcol.pixel = x;
			xcol.flags = DoRed | DoGreen | DoBlue;
			XStoreColor(priv->disp, priv->cmap, &xcol);
		}
		priv->cmap_first = priv->ncols;
		priv->cmap_last = 0;
		goto set;
	}
	if (priv->gammamap) {
		int x;
		XColor xcol;

		for (x = priv->gamma.start; x < priv->gamma.len; x++) {
			xcol.red   = priv->gammamap[x].red;
			xcol.green = priv->gammamap[x].green;
			xcol.blue  = priv->gammamap[x].blue;
			xcol.pixel = x;
			xcol.flags = DoRed | DoGreen | DoBlue;
			XStoreColor(priv->disp, priv->cmap, &xcol);
		}

		priv->gamma.start = priv->ncols;
		priv->gamma.len = 0;
		goto set;
	}
	return 0;
 set:
	if (priv->win) XSetWindowColormap(priv->disp, priv->win, priv->cmap);
	else XSetWindowColormap(priv->disp, priv->parentwin, priv->cmap);
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
		if (len > priv->ncols) {
			return -1;
		}

		if (COLOR_THRESHOLD(len, priv->ncols)) {
			return _ggi_smart_allocate(vis, len, colormap);
		}

		start = priv->ncols - len;
	}

	if (colormap == NULL || start+len > priv->ncols || start < 0) return -1;

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	if (start < priv->cmap_first) {
		priv->cmap_first = start;
	}
	if (start+len > priv->cmap_last) {
		priv->cmap_last  = start+len;
	}

	GGIDPRINT_COLOR("X setpalvec success\n");

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) _ggi_x_flush_cmap(vis);

	return 0;
}

int GGI_X_setgammamap(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_x_priv *priv;
	int i;

	priv = GGIX_PRIV(vis);
	if (priv->vilist[priv->viidx].vi->class != DirectColor) return -2;

	if (colormap == NULL) return -1;
	if (start >= priv->gamma.len) return -1;
	if (start < 0) return -1;
	if (len > (priv->gamma.len - start)) return -1;

	i = 0;
	do {
		if ((start + i) < priv->gamma.maxwrite_r)
			priv->gammamap[start + i].red   = colormap[i].r;
		if ((start + i) < priv->gamma.maxwrite_g)
			priv->gammamap[start + i].green = colormap[i].g;
		if ((start + i) < priv->gamma.maxwrite_b)
			priv->gammamap[start + i].blue  = colormap[i].b;
	} while (i++ < len);
	if (start < priv->gamma.start) {
		priv->gamma.start = start;
	}
	if (start+len > priv->gamma.len) {
		priv->gamma.len  = start+len;
	}

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) _ggi_x_flush_cmap(vis);

	return 0;
}

int GGI_X_getgammamap(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_x_priv *priv;
	int i;

	priv = GGIX_PRIV(vis);
	if (priv->vilist[priv->viidx].vi->class != TrueColor &&
	    priv->vilist[priv->viidx].vi->class != DirectColor) return -2;

	if (colormap==NULL) return -1;
	if (start >= priv->ncols) return -1;
	if (start < 0) return -1;
	if (len > (priv->ncols - start)) return -1;

	i = 0;
	do {
		colormap[i].r = priv->gammamap[start + i].red;
		colormap[i].g = priv->gammamap[start + i].green;
		colormap[i].b = priv->gammamap[start + i].blue;
	} while (i++ < len);

	return 0;
}

void _ggi_x_free_colormaps(ggi_visual *vis)
{
	ggi_x_priv *priv;
	priv = GGIX_PRIV(vis);

	if (priv->cmap != None)   XFreeColormap(priv->disp,priv->cmap);
	if (priv->cmap2 != None)  XFreeColormap(priv->disp,priv->cmap2);
	if (vis->palette) {
		free(vis->palette);
		vis->palette = NULL;
	}
	if (priv->gammamap != NULL) free(priv->gammamap);
	priv->gammamap = NULL;
}

/* This function may fail, if so priv->cmap will be set to None */
void _ggi_x_create_colormaps(ggi_visual *vis, XVisualInfo *vi)
{
	ggi_x_priv *priv;
	Colormap defcmap;
	XColor xcell;
	int i, j;
	ggi_pixelformat *fmt;

	fmt = LIBGGI_PIXFMT(vis);
	priv = GGIX_PRIV(vis);

	defcmap = DefaultColormap(priv->disp, vi->screen);

	vis->gamma->maxwrite_r = vis->gamma->maxwrite_g = 
		vis->gamma->maxwrite_b = vis->gamma->maxread_r = 
		vis->gamma->maxread_g = vis->gamma->maxread_b = 0;
	vis->gamma->gamma_r = vis->gamma->gamma_g = vis->gamma->gamma_b = 1.0;

	if (vi->class == PseudoColor || vi->class == GrayScale ||
	    vi->class == StaticColor || vi->class == StaticGray)
	{
		priv->cmap = XCreateColormap(priv->disp, priv->parentwin,
					     vi->visual, AllocAll);
		if (priv->cmap == None) return;
		priv->ncols = 1 << vi->depth;
		vis->palette = _ggi_malloc(sizeof(ggi_color) * priv->ncols);
		if (vis->palette == NULL) {
			XFreeColormap(priv->disp, priv->cmap);
			priv->cmap = None;
			return;
		}

		/* Fill the colormap with the original colors */
		for (i = 0; i < priv->ncols; i++) {
			xcell.pixel = i;
			xcell.flags = DoRed | DoGreen | DoBlue;
			XQueryColor(priv->disp, defcmap, &xcell);
			if (vi->class == PseudoColor || 
			    vi->class == GrayScale)
			{
				XStoreColor(priv->disp, priv->cmap, &xcell);
			}

			vis->palette[i].r = xcell.red;
			vis->palette[i].g = xcell.green;
			vis->palette[i].b = xcell.blue;
		}
		if (vi->class == PseudoColor || vi->class == GrayScale) {
			vis->opcolor->setpalvec = GGI_X_setpalvec;
		}

		priv->cmap_first=256;
		priv->cmap_last=0;
		GGIDPRINT_MODE("X: copied default colormap into (%x)\n",
			       priv->cmap);
		return;
	} else if (vi->class != DirectColor) {
		LIBGGI_APPASSERT(vi->class == TrueColor, "Unknown class!\n");
		priv->cmap = XCreateColormap(priv->disp, priv->parentwin,
					     vi->visual, AllocNone);
		if (priv->cmap == None) return;
		if (vi->class != TrueColor) return;
	} else {
		GGIDPRINT("Filmed on location in DirectColor\n");
		vis->opcolor->setgammamap = GGI_X_setgammamap;
		priv->cmap = XCreateColormap(priv->disp, priv->parentwin,
					     vi->visual, AllocAll);
		if (priv->cmap == None) return;
		vis->gamma->maxwrite_r = 1 << _ggi_countbits(fmt->red_mask);
		vis->gamma->maxwrite_g = 1 << _ggi_countbits(fmt->green_mask);
		vis->gamma->maxwrite_b = 1 << _ggi_countbits(fmt->blue_mask);
	}

	vis->opcolor->getgammamap = GGI_X_getgammamap;
	vis->gamma->maxread_r = _ggi_countbits(fmt->red_mask);
	vis->gamma->maxread_g = _ggi_countbits(fmt->green_mask);
	vis->gamma->maxread_b = _ggi_countbits(fmt->blue_mask);
	priv->ncols = vis->gamma->maxread_r;
	if (priv->ncols < vis->gamma->maxread_g)
		priv->ncols = vis->gamma->maxread_g;
	if (priv->ncols < vis->gamma->maxread_b)
		priv->ncols = vis->gamma->maxread_b;
	priv->ncols = 1 << priv->ncols;
	LIBGGI_APPASSERT(priv->ncols > 0, "X: Spurious Pixel Format");

	/* Fill the colormap with the original colors (or just read) */
	priv->gammamap = calloc((size_t)priv->ncols, sizeof(XColor));
	if (priv->gammamap == NULL) {
		XFreeColormap(priv->disp, priv->cmap);
		priv->cmap = None;
		return;
	}


	priv->gamma.start = 0;
	priv->gamma.len = priv->ncols;

	i = j = 0;
	do {
		priv->gammamap[j].pixel = 
			(i >> fmt->red_shift) & fmt->red_mask;
		j++;
		i += (0x80000000 >> (vis->gamma->maxread_r - 1));
	} while (i);
	i = j = 0;
	do {
		priv->gammamap[j].pixel |= 
			(i >> fmt->green_shift) & fmt->green_mask;
		j++;
		i += (0x80000000 >> (vis->gamma->maxread_g - 1));
	} while (i);
	i = j = 0;
	do {
		priv->gammamap[j].pixel |= 
			(i >> fmt->blue_shift) & fmt->blue_mask;
		j++;
		i += (0x80000000 >> (vis->gamma->maxread_b - 1));
	} while (i);
	vis->gamma->maxread_r = 1 << vis->gamma->maxread_r;
	vis->gamma->maxread_g = 1 << vis->gamma->maxread_g;
	vis->gamma->maxread_b = 1 << vis->gamma->maxread_b;

	XQueryColors(priv->disp, defcmap, priv->gammamap, priv->gamma.len);

	/* This accomplishes the documented behavior of ggiSetGammaMap
	 * by preventing aliasing of lower order color channel values.
	 * It must be done after XQueryColors as that sets all flags.
	 */
	for (i = 0; i < priv->ncols; i++) {
		priv->gammamap[i].flags = 0;
	}
	for (i = 0; i < vis->gamma->maxread_r; i++)
		priv->gammamap[i].flags |= DoRed;
	for (i = 0; i < vis->gamma->maxread_g; i++)
		priv->gammamap[i].flags |= DoGreen;
	for (i = 0; i < vis->gamma->maxread_b; i++)
		priv->gammamap[i].flags |= DoBlue;

	if (vi->class != DirectColor) return;
	XStoreColors(priv->disp, priv->cmap, priv->gammamap, priv->gamma.len);
	GGIDPRINT_MODE("X: copied default colormap into (%x)\n", priv->cmap);
}
