/* $Id: color.c,v 1.16 2004/11/27 16:41:52 soyt Exp $
******************************************************************************

   Generic color mapping

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

#include <string.h>
#include "color.h"
#include <ggi/internal/ggi_debug.h>


int GGI_color_getpalvec(ggi_visual *vis, int start, int len, ggi_color *colmap)
{
	if (start < 0 || start+len > COLOR_PALPRIV(vis)->numcols)
		return GGI_ENOSPACE;

	memcpy(colmap, LIBGGI_PAL(vis)->clut.data+start, len*sizeof(ggi_color));

	return 0;
}

int GGI_color_setpalvec(ggi_visual *vis, int start, int len, const ggi_color *colmap)
{
		if (start == GGI_PALETTE_DONTCARE) start = 0;
		
		if ((colmap == NULL)
		   || (start < 0)
		   || (start+len > COLOR_PALPRIV(vis)->numcols)) 
		{
			return GGI_ENOSPACE;
		}

		return LIBGGI_PAL(vis)->setPalette(vis, start, len, colmap);
}

/* ---------------------------------------------------------------------- */

/* This could be much optimized using the hybrid K-D tree:
 * http://www5.informatik.uni-erlangen.de/vampire/stuff/zinsser_vmv03.pdf
 */

ggi_pixel GGI_color_PAL_mapcolor(ggi_visual *vis, const ggi_color *col)
{
	color_palpriv *priv = COLOR_PALPRIV(vis);
	ggi_color *pal;
	ggi_pixel closest = 0;
	uint32 closest_dist;
	int pal_len;
	int i, r, g, b;

	LIB_ASSERT(LIBGGI_PAL(vis) != NULL, "PAL_mapcolor with LIBGGI_PAL(vis)==NULL");

	pal = LIBGGI_PAL(vis)->clut.data;

	LIB_ASSERT(pal != NULL, "PAL_mapcolor with LIBGGI_PAL(vis)->clut.data==NULL");

	r = col->r;
	g = col->g;
	b = col->b;

	/* Check cached value */
	if (priv->prev_col.r == r &&
	    priv->prev_col.g == g &&
	    priv->prev_col.b == b &&
	    pal[priv->prev_val].r == r &&
	    pal[priv->prev_val].g == g &&
	    pal[priv->prev_val].b == b)
	{
		return priv->prev_val;
	}

	pal_len = priv->numcols;
	closest_dist = (1U << 31U);

	for (i=0; i < pal_len; i++) {
#ifndef ABS
#define ABS(val)	((val) < 0 ? -(val) : val)
#endif
		uint32 dist =
			ABS(r - pal[i].r) +
			ABS(g - pal[i].g) +
			ABS(b - pal[i].b);

		if (dist < closest_dist) {
			closest = i;
			if (dist == 0) {
				/* Exact match */
				priv->prev_col.r = r;
				priv->prev_col.g = g;
				priv->prev_col.b = b;
				priv->prev_val = closest;
				break;
			}
			closest_dist = dist;
		}
	}

	GGID2(DPRINT_COLOR(
		"PAL_mapcolor(%p): %04x%04x%04x -> %04x%04x%04x (%d)\n",
		vis, r, g, b, pal[closest].r, pal[closest].g, pal[closest].b,
		closest));

	return closest;
}

/* These formats are common enough to deserve an optimized implementation.
 * Note we use priv->*_unmap to avoid an unnecessary negation, since
 * we know _unmap is positive and is the negative of _map.  These
 * optimizations are for older CPUs and likely don't make much difference 
 * on CPUs which implement branch prediction.
 */
ggi_pixel GGI_color_TRUE16_mapcolor(ggi_visual *vis, const ggi_color *col)
{
	ggi_pixel ret;
	color_truepriv *priv = vis->colorpriv;

	GGID2(DPRINT_COLOR("TRUE16_mapcolor_4to7(%p, "
			      "{r=0x%x, g=0x%x, b=0x%x}) called\n",
			      vis, col->r, col->g, col->b));

	ret = ((col->r >> priv->red_unmap) & priv->red_mask) |
	  ((col->g >> priv->green_unmap) & priv->green_mask) |
	  ((col->b >> priv->blue_unmap) & priv->blue_mask);

	GGID2(DPRINT_COLOR("TRUE16_mapcolor_4to7 returning 0x%x\n", ret));

	return ret;
}

ggi_pixel GGI_color_TRUE_mapcolor(ggi_visual *vis, const ggi_color *col)
{
	ggi_pixel ret;
	color_truepriv *priv = vis->colorpriv;

	GGID2(DPRINT_COLOR("TRUE_mapcolor(%p, "
			      "{r=0x%x, g=0x%x, b=0x%x}) called\n",
			      vis, col->r, col->g, col->b));

	ret = (SSHIFT(col->r, priv->red_map) & priv->red_mask) |
	  (SSHIFT(col->g, priv->green_map) & priv->green_mask) |
	  (SSHIFT(col->b, priv->blue_map) & priv->blue_mask);

	GGID2(DPRINT_COLOR("TRUE_mapcolor returning 0x%x\n", ret));

	return ret;
}

ggi_pixel GGI_color_GREY_mapcolor(ggi_visual *vis, const ggi_color *col)
{
	ggi_pixel ret;

	GGID2(DPRINT_COLOR("GREY_mapcolor(%p, "
			      "{r=0x%x, g=0x%x, b=0x%x}) called\n",
			      vis, col->r, col->g, col->b));

	/* NB: This formula is not very good... */
	ret = (col->r*82 + col->g*124 + col->b*50)
		>> COLOR_GREYPRIV(vis)->shift;

	GGID2(DPRINT_COLOR("GREY_mapcolor returning 0x%x\n", ret));

	return ret;
}


/* ---------------------------------------------------------------------- */


int GGI_color_PAL_unmappixel(ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	LIB_ASSERT(LIBGGI_PAL(vis) != NULL, 
			"PAL_unmappixel with LIBGGI_PAL(vis)==NULL");
	LIB_ASSERT(LIBGGI_PAL(vis)->clut.data != NULL, 
			"PAL_unmappixel with LIBGGI_PAL(vis)->clut.data==NULL");
	
	if (pixel >= (unsigned)COLOR_PALPRIV(vis)->numcols)
		return GGI_ENOSPACE;
	
	*col = LIBGGI_PAL(vis)->clut.data[pixel];
		
	return 0;
}

/* This format is common enough to deserve an optimized implementation. */
int GGI_color_TRUE16_unmappixel_4to7(ggi_visual *vis, ggi_pixel pixel, 
				     ggi_color *col)
{
	color_truepriv *priv = vis->colorpriv;

	col->r = (pixel & priv->red_mask) << priv->red_unmap;
	col->r |= col->r >> priv->red_nbits;
	col->r |= col->r >> (priv->red_nbits << 1);
	col->g = (pixel & priv->green_mask) << priv->green_unmap;
	col->g |= col->g >> priv->green_nbits;
	col->g |= col->g >> (priv->green_nbits << 1);
	col->b = (pixel & priv->blue_mask) << priv->blue_unmap;
	col->b |= col->b >> priv->blue_nbits;
	col->b |= col->b >> (priv->blue_nbits << 1);

	return 0;
}


int GGI_color_TRUE_unmappixel_gte8(ggi_visual *vis, ggi_pixel pixel, 
	  ggi_color *col)
{
	color_truepriv *priv = vis->colorpriv;

	col->r = SSHIFT(pixel & priv->red_mask, priv->red_unmap);
	col->r |= col->r >> priv->red_nbits;
	col->g = SSHIFT(pixel & priv->green_mask, priv->green_unmap);
	col->g |= col->g >> priv->green_nbits;
	col->b = SSHIFT(pixel & priv->blue_mask, priv->blue_unmap);
	col->b |= col->b >> priv->blue_nbits;

	return 0;
}

int GGI_color_TRUE_unmappixel_gte4(ggi_visual *vis, ggi_pixel pixel, 
				   ggi_color *col)
{
	color_truepriv *priv = vis->colorpriv;

	col->r = SSHIFT(pixel & priv->red_mask, priv->red_unmap);
	col->r |= col->r >> priv->red_nbits;
	col->r |= col->r >> (priv->red_nbits << 1);
	col->g = SSHIFT(pixel & priv->green_mask, priv->green_unmap);
	col->g |= col->g >> priv->green_nbits;
	col->g |= col->g >> (priv->green_nbits << 1);
	col->b = SSHIFT(pixel & priv->blue_mask, priv->blue_unmap);
	col->b |= col->b >> priv->blue_nbits;
	col->b |= col->b >> (priv->blue_nbits << 1);

	return 0;
}


int GGI_color_TRUE_unmappixel_gte2(ggi_visual *vis, ggi_pixel pixel, 
				   ggi_color *col)
{
	color_truepriv *priv = vis->colorpriv;

	col->r = SSHIFT(pixel & priv->red_mask, priv->red_unmap);
	col->r |= col->r >> priv->red_nbits;
	col->r |= col->r >> (priv->red_nbits << 1);
	col->r |= col->r >> (priv->red_nbits << 2);
	col->g = SSHIFT(pixel & priv->green_mask, priv->green_unmap);
	col->g |= col->g >> priv->green_nbits;
	col->g |= col->g >> (priv->green_nbits << 1);
	col->g |= col->g >> (priv->green_nbits << 2);
	col->b = SSHIFT(pixel & priv->blue_mask, priv->blue_unmap);
	col->b |= col->b >> priv->blue_nbits;
	col->b |= col->b >> (priv->blue_nbits << 1);
	col->b |= col->b >> (priv->blue_nbits << 2);

	return 0;
}

/* For the rare but extremely painful cases. */
int GGI_color_TRUE_unmappixel_gte1(ggi_visual *vis, ggi_pixel pixel, 
				   ggi_color *col)
{
	color_truepriv *priv = vis->colorpriv;

	if (priv->red_nbits != 1) {
		col->r = SSHIFT(pixel & priv->red_mask, priv->red_unmap);
		col->r |= col->r >> priv->red_nbits;
		col->r |= col->r >> (priv->red_nbits << 1);
		col->r |= col->r >> (priv->red_nbits << 2);
	} else col->r = (pixel & priv->red_mask) ? 0xffff : 0x0000;

	if (priv->green_nbits != 1) {
		col->g = SSHIFT(pixel & priv->green_mask, priv->green_unmap);
		col->g |= col->g >> priv->green_nbits;
		col->g |= col->g >> (priv->green_nbits << 1);
		col->g |= col->g >> (priv->green_nbits << 2);
	} else col->g = (pixel & priv->green_mask) ? 0xffff : 0x0000;

	if (priv->blue_nbits != 1) {
		col->b = SSHIFT(pixel & priv->blue_mask, priv->blue_unmap);
		col->b |= col->b >> priv->blue_nbits;
		col->b |= col->b >> (priv->blue_nbits << 1);
		col->b |= col->b >> (priv->blue_nbits << 2);
	} else col->b = (pixel & priv->blue_mask) ? 0xffff : 0x0000;

	return 0;
}

int GGI_color_GREY_unmappixel(ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	col->r = col->g = col->b = (pixel << COLOR_GREYPRIV(vis)->shift) >> 8;

	return 0;
}
