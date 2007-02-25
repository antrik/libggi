/* $Id: visual.c,v 1.13 2007/02/25 17:21:06 cegger Exp $
******************************************************************************

   Generic color handling library

   Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]

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

#include <stdlib.h>

#include "config.h"
#include "color.h"


static int calc_total(ggi_pixel mask)
{
	int total;

	for (total=0; mask != 0; mask >>= 1, total++) {
	}

	return total;
}


static int calc_nbits(ggi_pixel mask)
{
	int nbits;

	while (!(mask & 0x0001)) mask >>= 1;

	for (nbits=0; mask != 0; mask >>= 1, nbits++) {
	}

	return nbits;
}


static void do_setup_color_info(struct ggi_visual *vis)
{
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TRUECOLOR) {
		color_truepriv *priv = vis->colorpriv;
		int redtot   = calc_total(LIBGGI_PIXFMT(vis)->red_mask);
		int greentot = calc_total(LIBGGI_PIXFMT(vis)->green_mask);
		int bluetot  = calc_total(LIBGGI_PIXFMT(vis)->blue_mask);

		priv->red_map     = redtot - 16;
		priv->red_unmap   = 16 - redtot;
		priv->red_mask    = LIBGGI_PIXFMT(vis)->red_mask;
		priv->red_nbits     = calc_nbits(priv->red_mask);
		priv->green_map   = greentot - 16;
		priv->green_unmap = 16 - greentot;
		priv->green_mask  = LIBGGI_PIXFMT(vis)->green_mask;
		priv->green_nbits   = calc_nbits(priv->green_mask);
		priv->blue_map    = bluetot - 16;
		priv->blue_unmap  = 16 - bluetot;
		priv->blue_mask   = LIBGGI_PIXFMT(vis)->blue_mask;
		priv->blue_nbits    = calc_nbits(priv->blue_mask);
	} else if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE ||
		   GT_SCHEME(LIBGGI_GT(vis)) == GT_STATIC_PALETTE) {
		color_palpriv *priv = vis->colorpriv;

		priv->numcols = 1 << GT_DEPTH(LIBGGI_GT(vis));
		/* prev_col doesn't need to be initialized */
		priv->prev_val = 0;
	} else if (GT_SCHEME(LIBGGI_GT(vis)) == GT_GREYSCALE) {
		color_greypriv *priv = vis->colorpriv;

		priv->shift = 24 - GT_DEPTH(LIBGGI_GT(vis));
	}
}


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	vis->colorpriv = malloc(sizeof(color_priv));
	if (vis->colorpriv == NULL) return GGI_ENOMEM;

	do_setup_color_info(vis);
	
	/* Color mapping
	 */
	switch (GT_SCHEME(LIBGGI_GT(vis))) {
		case GT_PALETTE:
		case GT_STATIC_PALETTE:
			vis->opcolor->mapcolor   = GGI_color_PAL_mapcolor;
			vis->opcolor->unmappixel = GGI_color_PAL_unmappixel;
			vis->opcolor->getpalvec  = GGI_color_getpalvec;
			vis->opcolor->setpalvec  = GGI_color_setpalvec;
			break;

		case GT_TRUECOLOR:
			vis->opcolor->mapcolor   = GGI_color_TRUE_mapcolor;
			if (COLOR_TRUEPRIV(vis)->red_nbits >= 8 &&
			    COLOR_TRUEPRIV(vis)->green_nbits >= 8 &&
			    COLOR_TRUEPRIV(vis)->blue_nbits >= 8)
				vis->opcolor->unmappixel = 
					GGI_color_TRUE_unmappixel_gte8;
			else if (COLOR_TRUEPRIV(vis)->red_nbits >= 4 &&
				 COLOR_TRUEPRIV(vis)->green_nbits >= 4 &&
				 COLOR_TRUEPRIV(vis)->blue_nbits >= 4)
				vis->opcolor->unmappixel = 
					GGI_color_TRUE_unmappixel_gte4;
			else if (COLOR_TRUEPRIV(vis)->red_nbits >= 2 &&
				 COLOR_TRUEPRIV(vis)->green_nbits >= 2 &&
				 COLOR_TRUEPRIV(vis)->blue_nbits >= 2)
					vis->opcolor->unmappixel = 
						GGI_color_TRUE_unmappixel_gte2;
			else vis->opcolor->unmappixel = 
				GGI_color_TRUE_unmappixel_gte1;
			if (GT_SIZE(LIBGGI_GT(vis)) == 16) {
			  vis->opcolor->mapcolor   = GGI_color_TRUE16_mapcolor;
			  if (vis->opcolor->unmappixel == 
			      GGI_color_TRUE_unmappixel_gte4)
			  	vis->opcolor->unmappixel = 
					GGI_color_TRUE16_unmappixel_4to7;
			}

			break;

		case GT_GREYSCALE:
			vis->opcolor->mapcolor   = GGI_color_GREY_mapcolor;
			vis->opcolor->unmappixel = GGI_color_GREY_unmappixel;
			break;
	}

	if (! (GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT)) {
		switch (GT_ByPP(LIBGGI_GT(vis))) {
		case 1: vis->opcolor->packcolors   = GGI_color_L1_packcolors;
			vis->opcolor->unpackpixels = GGI_color_L1_unpackpixels;
			break;

		case 2: vis->opcolor->packcolors   = GGI_color_L2_packcolors;
			vis->opcolor->unpackpixels = GGI_color_L2_unpackpixels;
			break;

		case 3: vis->opcolor->packcolors   = GGI_color_L3_packcolors;
			vis->opcolor->unpackpixels = GGI_color_L3_unpackpixels;
			break;

		case 4: vis->opcolor->packcolors   = GGI_color_L4_packcolors;
			vis->opcolor->unpackpixels = GGI_color_L4_unpackpixels;
			break;
		}
	}

	/* Gamma mapping 
	 */
	vis->opcolor->getgamma = GGI_color_getgamma;	
	vis->opcolor->setgamma = GGI_color_setgamma;	

	*dlret = GGI_DL_OPCOLOR;
	return 0;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(vis->colorpriv);

	return 0;
}


EXPORTFUNC
int GGIdl_color(int func, void **funcptr);

int GGIdl_color(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
