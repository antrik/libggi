/* $Id: color.c,v 1.12 2004/11/27 16:42:19 soyt Exp $
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

#include "config.h"
#include <ggi/display/fbdev.h>
#include <ggi/internal/ggi_debug.h>

void GGI_fbdev_color_reset(ggi_visual *vis);
void GGI_fbdev_color_setup(ggi_visual *vis);
static int GGI_fbdev_getgammamap(ggi_visual *vis, int start, int len, 
				 ggi_color *colormap);
static int GGI_fbdev_setgammamap(ggi_visual *vis, int start, int len, 
				 const ggi_color *colormap);
static int GGI_fbdev_setPalette(ggi_visual *vis, size_t start, size_t end, 
			       const ggi_color *colormap);
static size_t GGI_fbdev_getPrivSize(ggi_visual_t vis);



/* Restore and free palette/gamma entries.  Called before changing modes. */
void GGI_fbdev_color_reset(ggi_visual *vis) {
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);

	if (LIBGGI_PAL(vis)->clut.data == NULL) return; /* New visual. */

	if ((LIBGGI_PAL(vis)->setPalette != NULL) && (vis->opcolor->setpalvec != NULL))
		vis->opcolor->setpalvec(vis, 0, 1 << GT_DEPTH(LIBGGI_GT(vis)),
			       LIBGGI_PAL(vis)->clut.data);
	else if (vis->opcolor->setgammamap != NULL)
	  vis->opcolor->setgammamap(vis, 0, vis->gamma->len, LIBGGI_PAL(vis)->clut.data);

	/* Unhook the entry points */
	LIBGGI_PAL(vis)->setPalette = NULL;
	LIBGGI_PAL(vis)->getPrivSize = NULL;
	vis->opcolor->getpalvec = NULL; /* ##### */
	vis->opcolor->setgammamap = NULL;
	vis->opcolor->getgammamap = NULL;

	/* Free the convenience array */
	free (LIBGGI_PAL(vis)->priv);
	LIBGGI_PAL(vis)->priv = NULL;
	
	/* 
	 * Clean up any pointers that could be problematic if left set.
	 * Technically they should be taken care of below, but why not...
	 */
	priv->reds = priv->greens = priv->blues = NULL;
	vis->gamma = NULL;

	/* Free the storage area for the palettes. */
	priv->gamma.map = priv->orig_cmap = NULL;
	free (LIBGGI_PAL(vis)->clut.data);
	LIBGGI_PAL(vis)->clut.data = NULL;
}

void GGI_fbdev_color_setup(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	struct fb_cmap cmap;
	int len;

	/* We rely on caller to have deallocated old storage */
	priv->orig_cmap = LIBGGI_PAL(vis)->clut.data = priv->gamma.map = NULL; 
	vis->gamma = NULL;
	priv->reds = priv->greens = priv->blues = NULL;
	priv->gamma.maxread_r = priv->gamma.maxread_g = 
	  priv->gamma.maxread_b = priv->gamma.maxread_r = 
	  priv->gamma.maxwrite_g = priv->gamma.maxwrite_b = -1;
	priv->gamma.len = priv->gamma.start = 0;

	if (!priv->var.bits_per_pixel) return;
	if (priv->fix.visual == FB_VISUAL_TRUECOLOR) return; /* No gamma. */

	if (priv->fix.visual == FB_VISUAL_DIRECTCOLOR) {

		DPRINT("display-fbdev: trying gamma.\n");

		priv->gamma.maxwrite_r = priv->gamma.maxread_r = 
			1 << priv->var.red.length;
		priv->gamma.maxwrite_g = priv->gamma.maxread_g = 
			1 << priv->var.green.length;
		priv->gamma.maxwrite_b = priv->gamma.maxread_b = 
			1 << priv->var.blue.length;
		
		len = priv->gamma.maxread_r;
		if (len < priv->gamma.maxread_g) 
			len = priv->gamma.maxread_g;
		if (len < priv->gamma.maxread_b)
			len = priv->gamma.maxread_b;
		priv->gamma.len = len;
		priv->gamma.start = 0;
		
		LIBGGI_PAL(vis)->clut.size = len * 2;
		LIBGGI_PAL(vis)->clut.data = calloc(len * 2 /* orig */, sizeof(ggi_color));
		if (LIBGGI_PAL(vis)->clut.data == NULL) return;
		priv->gamma.map = LIBGGI_PAL(vis)->clut.data;
		/* All of the above is moot until we turn it on like so: */
		vis->gamma = &(priv->gamma);
	} else {

		DPRINT("display-fbdev: trying palette.\n");

		len = 1 << priv->var.bits_per_pixel;
		LIBGGI_PAL(vis)->clut.size = len * 2;
		LIBGGI_PAL(vis)->clut.data = calloc(len * 2 /* orig */, sizeof(ggi_color));
		if (LIBGGI_PAL(vis)->clut.data == NULL) return;
	}

	cmap.start = 0;
	cmap.len   = len;
	cmap.red   = calloc(len * 3, 2);
	if (cmap.red == NULL) goto bail;
	cmap.green = cmap.red + len;
	cmap.blue  = cmap.green + len;
	cmap.transp = NULL;
	
	if (ioctl(LIBGGI_FD(vis), FBIOGETCMAP, &cmap) < 0) {
		DPRINT_COLOR("display-fbdev: GETCMAP failed.\n");
		free(cmap.red);
		goto bail;
	} 

	priv->orig_cmap = LIBGGI_PAL(vis)->clut.data + len;

	if (vis->gamma != NULL) {

		DPRINT_COLOR("display-fbdev: Saved gamma (len=%d/%d/%d).\n",
				priv->gamma.maxread_r, priv->gamma.maxread_g,
				priv->gamma.maxread_b);

		while (len--) {
			if (len < priv->gamma.maxread_r) 
				priv->orig_cmap[len].r = cmap.red[len];
			if (len < priv->gamma.maxread_g) 
				priv->orig_cmap[len].g = cmap.green[len];
			if (len < priv->gamma.maxread_b) 
				priv->orig_cmap[len].b = cmap.blue[len];
		}
		vis->opcolor->getgammamap = GGI_fbdev_getgammamap;
		vis->opcolor->setgammamap = GGI_fbdev_setgammamap;
	}
	else {
		DPRINT_COLOR("display-fbdev: Saved palette (len=%d).\n", 
				len);
		while (len--) {
			priv->orig_cmap[len].r = cmap.red[len];
			priv->orig_cmap[len].g = cmap.green[len];
			priv->orig_cmap[len].b = cmap.blue[len];
		}
		if (priv->fix.visual != FB_VISUAL_STATIC_PSEUDOCOLOR) {
			LIBGGI_PAL(vis)->setPalette  = GGI_fbdev_setPalette;
			LIBGGI_PAL(vis)->getPrivSize =  GGI_fbdev_getPrivSize;
		}
	}

	LIBGGI_PAL(vis)->priv = cmap.red;

	priv->reds = cmap.red;
	priv->greens = cmap.green;
	priv->blues = cmap.blue;
	return;

 bail:
	free(LIBGGI_PAL(vis)->clut.data);
	LIBGGI_PAL(vis)->clut.data = NULL;
	vis->gamma = NULL;
	return;

}

static int GGI_fbdev_setPalette(ggi_visual *vis, size_t start, size_t size, 
			       const ggi_color *colormap)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	struct fb_cmap cmap;
	
	int len = (int)size;
	
	ggi_color* src = (ggi_color*)colormap;
	
	DPRINT_COLOR("display-fbdev: SetPalette(%d,%d)\n", start, size);
	
	memcpy(LIBGGI_PAL(vis)->clut.data+start, colormap, size*sizeof(ggi_color));

	if (!priv->ismapped) return 0;

	cmap.start  = start;
	cmap.len    = size;
	cmap.red    = priv->reds + start;
	cmap.green  = priv->greens + start;
	cmap.blue   = priv->blues + start;
	cmap.transp = NULL;

	for (; len > 0; start++, src++, len--) {
		priv->reds[start]   = src->r;
		priv->greens[start] = src->g;
		priv->blues[start]  = src->b;
	}

	if (fbdev_doioctl(vis, FBIOPUTCMAP, &cmap) < 0) {
		DPRINT_COLOR("display-fbdev: PUTCMAP failed.");
		return -1;
	}

	return 0;
}

/* In fbdev the gamma uses the same interface as the palette.
 * Therefore, no simultaneous use of 8-bit LUT and gamma.
 *
 * Since this is the case we reuse some of the priv/vis palette members.
 *
 */
static int GGI_fbdev_setgammamap(ggi_visual *vis, int start, int len, 
				 const ggi_color *colormap)
{
	ggi_fbdev_priv *priv;
	struct fb_cmap gam;
	int i;

	priv = FBDEV_PRIV(vis);
	if (colormap == NULL) return GGI_EARGINVAL;
	if (vis->gamma == NULL) return GGI_ENOMATCH; /* Wrong GT if not hooked */
	if (start < 0 || start >= priv->gamma.len) return GGI_ENOSPACE;
	if (len > (priv->gamma.len - start)) return GGI_ENOSPACE;

	gam.start = start;
	gam.len = len;
	gam.red = priv->reds;
	gam.green = priv->greens;
	gam.blue = priv->blues;
	gam.transp = NULL;

	i = 0;
	do {
		if ((start + i) < priv->gamma.maxwrite_r)
			vis->gamma->map[start + i].r = priv->reds[start + i]
			  = colormap[i].r;
		if ((start + i) < priv->gamma.maxwrite_g)
			vis->gamma->map[start + i].g = priv->greens[start + i]
			  = colormap[i].g;
		if ((start + i) < priv->gamma.maxwrite_b)
			vis->gamma->map[start + i].b = priv->blues[start + i]
			  = colormap[i].b;
	} while (i++ < len);

	if (fbdev_doioctl(vis, FBIOPUTCMAP, &gam) < 0) {
		DPRINT_COLOR("display-fbdev: PUTCMAP failed.");
		return -1;
	}
	return 0;
}

/* This could be moved to default/color as a stub.  It isn't fbdev-local. */
int GGI_fbdev_getgammamap(ggi_visual *vis, int start, int len, 
			  ggi_color *colormap)
{
	ggi_fbdev_priv *priv;
	int i;

	priv = FBDEV_PRIV(vis);
	if (colormap == NULL) return GGI_EARGINVAL;
	if (vis->gamma == NULL) return GGI_ENOMATCH; /* wrong GT if not hooked */
	if (vis->gamma->map == NULL) return GGI_EARGINVAL;
	if (start < 0 || start >= vis->gamma->len) return GGI_ENOSPACE;
	if (len > (vis->gamma->len - start)) return GGI_ENOSPACE;

	i = 0;
	do {
		if ((start + i) < vis->gamma->maxread_r)
			colormap[i].r = vis->gamma->map[start + i].r;
		if ((start + i) < vis->gamma->maxread_g)
			colormap[i].g = vis->gamma->map[start + i].g;
		if ((start + i) < vis->gamma->maxread_b)
			colormap[i].b = vis->gamma->map[start + i].b;
	} while (i++ < len);

	return 0;
}

static size_t GGI_fbdev_getPrivSize(ggi_visual_t vis) 
{
	return (LIBGGI_PAL(vis)->clut.size * 3);
}
