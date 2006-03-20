/* $Id: color.c,v 1.3 2006/03/20 19:56:27 cegger Exp $
******************************************************************************

   Display quartz : color management

   Copyright (C) 2004-2005 Christoph Egger

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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "quartz.h"
#include <ggi/internal/ggi_debug.h>


int GGI_quartz_setpalvec(struct ggi_visual *vis,int start,int len,const ggi_color *colormap)
{
	CGTableCount  i;
	CGDeviceColor color;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	DPRINT("quartz setpalette.\n");

	fprintf(stderr, "setpalvec (1)\n");
	if (start == GGI_PALETTE_DONTCARE) {
		if (len > priv->ncols) {
			return GGI_ENOSPACE;
		}	/* if */

		start = priv->ncols - len;
	}	/* if */

	fprintf(stderr, "setpalvec (2)\n");

	if (start+len > priv->ncols || start < 0)
		return GGI_ENOSPACE;

	fprintf(stderr, "setpalvec (3)\n");

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	for (i = (unsigned)start; i < (unsigned)start+len; i++) {
		/* Clamp colors between 0.0 and 1.0 */
		color.red   = colormap->r / 65535.0;
		color.green = colormap->g / 65535.0;
		color.blue  = colormap->b / 65535.0;

		fprintf(stderr, "%i. colormap (%X, %X, %X), color (%.02f, %.02f, %.02f)\n",
			i, colormap->r, colormap->g, colormap->b,
			color.red, color.green, color.blue);

		colormap++;

		CGPaletteSetColorAtIndex (priv->palette, color, i);
	}	/* for */

	if ( CGDisplayNoErr != CGDisplaySetPalette (priv->display_id, priv->palette) )
		return 0;

	return 0;
}	/* GGI_quartz_setpalvec */




/* Gamma correction
 */

int GGI_quartz_setgamma(struct ggi_visual *vis,ggi_float r,ggi_float g,ggi_float b)
{
	ggi_quartz_priv *priv;
	const CGGammaValue min = 0.0, max = 1.0;

	priv = QUARTZ_PRIV(vis);

	if (r == 0.0) {
		r = FLT_MAX;
	} else {
		r = 1.0 / r;
	}	/* if */

	if (g == 0.0) {
		g = FLT_MAX;
	} else {
		g = 1.0 / g;
	}	/* if */

	if (b == 0.0) {
		b = FLT_MAX;
	} else {
		b  = 1.0 / b;
	}	/* if */


	if ( CGDisplayNoErr == CGSetDisplayTransferByFormula
		(priv->display_id, min, max, r, min, max, g, min, max, b) )
	{
		return 0;
	} else {
		return -1;
	}	/* if */
}	/* GGI_quartz_setgamma */


int GGI_quartz_getgamma(struct ggi_visual *vis,ggi_float *r,ggi_float *g,ggi_float *b)
{
	ggi_quartz_priv *priv;
	CGGammaValue dummy;

	priv = QUARTZ_PRIV(vis);

	if ( CGDisplayNoErr != CGGetDisplayTransferByFormula
	   (priv->display_id, &dummy, &dummy, (CGGammaValue *)r,
		&dummy, &dummy, (CGGammaValue *)g,
		&dummy, &dummy, (CGGammaValue *)b) )
	{
		return GGI_ENOMATCH;
	}	/* if */

	return 0;
}	/* GGI_quartz_getgamma */


int GGI_quartz_setgammamap(struct ggi_visual *vis, int start, int len, const ggi_color *colormap)
{
	int i;
	ggi_quartz_priv *priv;

	/* Note: If the compiler breaks here,
	 * then it is not ANSI C99 conform.
	 */
	const CGTableCount tableSize = len;
	CGGammaValue redTable[tableSize];
	CGGammaValue greenTable[tableSize];
	CGGammaValue blueTable[tableSize];

	priv = QUARTZ_PRIV(vis);

	if (colormap == NULL) return GGI_EARGINVAL;
	if (start < 0 || start >= vis->gamma->len) return GGI_ENOSPACE;
	if (len > (vis->gamma->len - start)) return GGI_ENOSPACE;

	/* Extract gamma values into separate tables,
	 * convert to floats between 0.0 and 1.0
	 */

#if 0
	i = 0;
	do {
		if ((start + i) < priv->gamma.maxwrite_r) {
			priv->gammamap[start + i].red   = colormap[i].r;
		}	/* if */
		if ((start + i) < priv->gamma.maxwrite_g) {
			priv->gammamap[start + i].green = colormap[i].g;
		}	/* if */
		if ((start + i) < priv->gamma.maxwrite_b) {
			priv->gammamap[start + i].blue  = colormap[i].b;
		}	/* if */
	} while (i++ < len);
#endif

	i = 0;
	do {
		redTable[i] = colormap[i].r / 65535.0;
		greenTable[i] = colormap[i].g / 65535.0;
		blueTable[i] = colormap[i].b / 65535.0;
	} while (i++ < len);

	if ( CGDisplayNoErr != CGSetDisplayTransferByTable
	  (priv->display_id, tableSize, redTable, greenTable, blueTable) )
	{
		return -1;
	}	/* if */

	return 0;
}	/* GGI_quartz_setgammamap */


int GGI_quartz_getgammamap(struct ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	ggi_quartz_priv *priv = QUARTZ_PRIV(vis);

	/* Note: If the compiler breaks here,
	 * then it is not ANSI C99 conform.
	 */
	const CGTableCount tableSize = vis->gamma->len;
	CGGammaValue redTable[tableSize];
	CGGammaValue greenTable[tableSize];
	CGGammaValue blueTable[tableSize];
	CGTableCount actualSize;
	int i;

	if (colormap==NULL) return GGI_EARGINVAL;
	if (start < 0 || start >= vis->gamma->len) return GGI_ENOSPACE;
	if (len > (vis->gamma->len - start)) return GGI_ENOSPACE;


	if ( CGDisplayNoErr != CGGetDisplayTransferByTable
	  (priv->display_id, tableSize, redTable, greenTable, blueTable, &actualSize))
	{
		return -1;
	}	/* if */

	if ((unsigned)len < actualSize) len = actualSize;
	if (len < start) return GGI_ENOSPACE;

	/* Pack tables into one array, with values from 0 to 65535 */
	i = 0;
	do {
		colormap[i].r = redTable[start + i] * 65535.0;
		colormap[i].g = greenTable[start + i] * 65535.0;
		colormap[i].b = blueTable[start + i] * 65535.0;
	} while (i++ < len);

	return 0;
}	/* GGI_quartz_getgammamap */
