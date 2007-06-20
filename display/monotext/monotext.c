/* $Id: monotext.c,v 1.17 2007/06/20 07:31:08 cegger Exp $
******************************************************************************

   Display-monotext

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

#include "config.h"
#include <ggi/display/monotext.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "font_data.h"


/**************************************************
 ***
 ***  Internal functions 
 ***
 **************************************************/


static uint8_t ascii_template[256*4*4];

static uint8_t greyblock_to_ascii[65536];


static inline int count_bits(unsigned char *ch, int x, int y, int w, int h)
{
	int cx, cy;
	int result=0;

	for (cy = y; cy < (y+h); cy++)
	for (cx = x; cx < (x+w); cx++) {

		if (ch[cy] & (1 << (7-cx))) {
			result++;
		}
	}

	return result;
}

static void setup_templates(ggi_coord accuracy)
{
	int box_w = 8 / accuracy.x;
	int box_h = 8 / accuracy.y;

	int a, x, y;

	DPRINT("Calculating character templates...\n");

	for (a=32; a <= 126; a++)
	for (y=0; y < accuracy.y; y++)
	for (x=0; x < accuracy.x; x++) {

		ascii_template[(a-32)*16 + y*accuracy.x + x] =
			count_bits(&(font_data[(a-32)*8]), x*box_w, y*box_h,
				   box_w, box_h) * 255 / (box_w*box_h);
	}
}

static inline int sum_of_squared_diffs(uint8_t *ta, uint8_t *tb, 
					ggi_coord accuracy)
{
	int n, diff;
	int result = 0;

	for (n=0; n < (accuracy.x * accuracy.y); n++) {

		diff = (int) *ta++ - (int) *tb++;
		
		result += diff * diff;
	}

	return result;
}

static int find_closest_char(uint8_t *templ, ggi_coord accuracy)
{
	int a, val;

	int min_result = 0x70000000;
	int min_char = 32;

	for (a=32; a <= 126; a++) {

		val = sum_of_squared_diffs(templ, 
				& ascii_template[(a-32)*16], accuracy);

		if (val < min_result) {
			min_char = a;
			min_result = val;
		}
	}

	return min_char;
}

static void setup_rgb2grey_table(uint8_t *map)
{
	int r, g, b, i;

	DPRINT("Calculating rgb->greyscale table...\n");

	for (r=0; r < 32; r++)
	for (g=0; g < 32; g++)
	for (b=0; b < 32; b++) {

		i = (int) sqrt((double) (r*r*30 + g*g*50 + b*b*20));

		map[(r << 10) | (g << 5) | b] = i * 256 / 311;
	}
}

static void calc_accuracy_1x1(int _index, ggi_coord acc)
{
	uint8_t templ[16];

	templ[0] = _index;

	greyblock_to_ascii[_index] = find_closest_char(templ, acc);
}

static void calc_accuracy_1x2(int _index, ggi_coord acc)
{
	uint8_t templ[16];

	templ[0] = _index >> 8;
	templ[1] = _index & 0xff;

	greyblock_to_ascii[_index] = find_closest_char(templ, acc);
}

static void calc_accuracy_2x2(int _index, ggi_coord acc)
{
	uint8_t templ[16];

	templ[0] = ((_index >> 12) & 0xf) * 255 / 15;
	templ[1] = ((_index >>  8) & 0xf) * 255 / 15;
	templ[2] = ((_index >>  4) & 0xf) * 255 / 15;
	templ[3] = ((_index      ) & 0xf) * 255 / 15;

	greyblock_to_ascii[_index] = find_closest_char(templ, acc);
}

static void calc_accuracy_2x4(int _index, ggi_coord acc)
{
	uint8_t templ[16];

	templ[0] = ((_index >> 14) & 0x3) * 255 / 3;
	templ[1] = ((_index >> 12) & 0x3) * 255 / 3;
	templ[2] = ((_index >> 10) & 0x3) * 255 / 3;
	templ[3] = ((_index >>  8) & 0x3) * 255 / 3;

	templ[4] = ((_index >>  6) & 0x3) * 255 / 3;
	templ[5] = ((_index >>  4) & 0x3) * 255 / 3;
	templ[6] = ((_index >>  2) & 0x3) * 255 / 3;
	templ[7] = ((_index      ) & 0x3) * 255 / 3;

	greyblock_to_ascii[_index] = find_closest_char(templ, acc);
}

static void calc_accuracy_4x4(int _index, ggi_coord acc)
{
	uint8_t templ[16];

	templ[ 0] = (_index & 0x8000) ? 255 : 0;
	templ[ 1] = (_index & 0x4000) ? 255 : 0;
	templ[ 2] = (_index & 0x2000) ? 255 : 0;
	templ[ 3] = (_index & 0x1000) ? 255 : 0;

	templ[ 4] = (_index & 0x0800) ? 255 : 0;
	templ[ 5] = (_index & 0x0400) ? 255 : 0;
	templ[ 6] = (_index & 0x0200) ? 255 : 0;
	templ[ 7] = (_index & 0x0100) ? 255 : 0;

	templ[ 8] = (_index & 0x0080) ? 255 : 0;
	templ[ 9] = (_index & 0x0040) ? 255 : 0;
	templ[10] = (_index & 0x0020) ? 255 : 0;
	templ[11] = (_index & 0x0010) ? 255 : 0;

	templ[12] = (_index & 0x0008) ? 255 : 0;
	templ[13] = (_index & 0x0004) ? 255 : 0;
	templ[14] = (_index & 0x0002) ? 255 : 0;
	templ[15] = (_index & 0x0001) ? 255 : 0;

	greyblock_to_ascii[_index] = find_closest_char(templ, acc);
}

static void blitter_1x1(ggi_monotext_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t *)  src;
	uint16_t *d = (uint16_t *) dest;

	for (; w > 0; w--, s++) {
	
		if (greyblock_to_ascii[*s] == 255) {
			calc_accuracy_1x1(*s, priv->accuracy);
		}

		*d++ = 0x0700 | greyblock_to_ascii[*s]; 
	}
}

static void blitter_1x2(ggi_monotext_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t *)  src;
	uint16_t *d = (uint16_t *) dest;

	int stride = priv->size.x * priv->accuracy.x * priv->squish.x;
	int _index;

	for (; w > 0; w--, s++) {
	
		_index = (s[0] << 8) | s[stride];
		
		if (greyblock_to_ascii[_index] == 255) {
			calc_accuracy_1x2(_index, priv->accuracy);
		}

		*d++ = 0x0700 | greyblock_to_ascii[_index]; 
	}
}

static void blitter_2x2(ggi_monotext_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t *)  src;
	uint16_t *d = (uint16_t *) dest;

	int stride = priv->size.x * priv->accuracy.x * priv->squish.x;
	int _index;

	for (; w > 1; w -= 2, s += 2) {
	
		_index = ((s[stride*0 + 0] & 0xf0) << 8) |
		        ((s[stride*0 + 1] & 0xf0) << 4) |
		        ((s[stride*1 + 0] & 0xf0)     ) |
		        ((s[stride*1 + 1] & 0xf0) >> 4);
		
		if (greyblock_to_ascii[_index] == 255) {
			calc_accuracy_2x2(_index, priv->accuracy);
		}

		*d++ = 0x0700 | greyblock_to_ascii[_index];
	}
}

static void blitter_2x4(ggi_monotext_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t *)  src;
	uint16_t *d = (uint16_t *) dest;

	int stride = priv->size.x * priv->accuracy.x * priv->squish.x;
	int _index;

	for (; w > 1; w -= 2, s += 2) {
	
		_index = ((s[stride*0 + 0] & 0xc0) << 8) |
		        ((s[stride*0 + 1] & 0xc0) << 6) |
		        ((s[stride*1 + 0] & 0xc0) << 4) |
		        ((s[stride*1 + 1] & 0xc0) << 2) |
	
		        ((s[stride*2 + 0] & 0xc0)     ) |
		        ((s[stride*2 + 1] & 0xc0) >> 2) |
		        ((s[stride*3 + 0] & 0xc0) >> 4) |
		        ((s[stride*3 + 1] & 0xc0) >> 6);
		
		if (greyblock_to_ascii[_index] == 255) {
			calc_accuracy_2x4(_index, priv->accuracy);
		}

		*d++ = 0x0700 | greyblock_to_ascii[_index];
	}
}

static void blitter_4x4(ggi_monotext_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t *)  src;
	uint16_t *d = (uint16_t *) dest;

	int stride = priv->size.x * priv->accuracy.x * priv->squish.x;
	int _index;

	for (; w > 3; w -= 4, s += 4) {
	
		_index = ((s[stride*0 + 0] & 0x80) << 8) |
		        ((s[stride*0 + 1] & 0x80) << 7) |
		        ((s[stride*0 + 2] & 0x80) << 6) |
		        ((s[stride*0 + 3] & 0x80) << 5) |

		        ((s[stride*1 + 0] & 0x80) << 4) |
		        ((s[stride*1 + 1] & 0x80) << 3) |
		        ((s[stride*1 + 2] & 0x80) << 2) |
		        ((s[stride*1 + 3] & 0x80) << 1) |

		        ((s[stride*2 + 0] & 0x80)     ) |
		        ((s[stride*2 + 1] & 0x80) >> 1) |
		        ((s[stride*2 + 2] & 0x80) >> 2) |
		        ((s[stride*2 + 3] & 0x80) >> 3) |

		        ((s[stride*3 + 0] & 0x80) >> 4) |
		        ((s[stride*3 + 1] & 0x80) >> 5) |
		        ((s[stride*3 + 2] & 0x80) >> 6) |
		        ((s[stride*3 + 3] & 0x80) >> 7);

		if (greyblock_to_ascii[_index] == 255) {
			calc_accuracy_4x4(_index, priv->accuracy);
		}

		*d++ = 0x0700 | greyblock_to_ascii[_index];
	}
}


/**************************************************
 ***
 ***  Exported functions 
 ***
 **************************************************/


static uint8_t src_buf[8192];
static uint8_t dest_buf[8192];


static inline void get_source_lines(struct ggi_visual *vis, int x, int y, int w,
				   uint8_t *src)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	int stride = priv->size.x * priv->accuracy.x * priv->squish.x;

	int num_w  = w / priv->squish.x;
	int i, j;

	for (j=0; j < priv->accuracy.y; j++, y+=priv->squish.y, src+=stride) {

		ggiGetHLine(vis->instance.stem, x, y, w, src);
		
		for (i=0; i < num_w; i++) {
			src[i] = priv->greymap[src[i * priv->squish.x]];
		}
	}
}

int _ggi_monotext_update(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	int step_x = priv->accuracy.x * priv->squish.x;
	int step_y = priv->accuracy.y * priv->squish.y;
	
	/* round */

	if (y % step_y) {
		h += (y % step_y);
		y -= (y % step_y);
	}

	if (x % step_x) {
		w += (x % step_x);
		x -= (x % step_x);
	}

	/* do transfer */

	for (; h >= step_y; h -= step_y, y += step_y) {

		get_source_lines(vis, x, y, w, src_buf);

		(* priv->do_blit)(priv, dest_buf, src_buf, w);

		ggiPutHLine(priv->parent, x / step_x, y / step_y, w / step_x, 
			    dest_buf);
	}

	if (! (LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) {
		ggiFlush(priv->parent);
	}
	
	return 0;
}

int _ggi_monotext_flush(struct ggi_visual *vis)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	int sx = MAX(priv->dirty_tl.x, vis->gc->cliptl.x);
	int sy = MAX(priv->dirty_tl.y, vis->gc->cliptl.y);

	int ex = MIN(priv->dirty_br.x, vis->gc->clipbr.x);
	int ey = MIN(priv->dirty_br.y, vis->gc->clipbr.y);


	/* clear the 'dirty region' */

	priv->dirty_tl.x = priv->size.x;
	priv->dirty_tl.y = priv->size.y;
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;


	if ((sx < ex) && (sy < ey)) {
		return _ggi_monotextUpdate(vis, sx, sy, ex-sx, ey-sy);
	}

	return 0;
}

int _ggi_monotext_open(struct ggi_visual *vis)
{
	int rc;
	ggi_mode sug_mode;
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	ggi_coord child_size;

	priv->size = LIBGGI_MODE(vis)->visible;

	DPRINT("display-monotext: Open (size=%dx%d accuracy=%dx%d "
		"squish=%dx%d)\n", priv->size.x, priv->size.y,
		priv->accuracy.x, priv->accuracy.y,
		  priv->squish.x, priv->squish.y);

	priv->colormap    = _ggi_malloc(256 * sizeof(ggi_color));
	priv->greymap     = _ggi_malloc(256);
	priv->rgb_to_grey = _ggi_malloc(32 * 32 * 32);

	child_size.x = priv->size.x / priv->accuracy.x / priv->squish.x;
	child_size.y = priv->size.y / priv->accuracy.y / priv->squish.y;

	priv->red_gamma = priv->green_gamma = priv->blue_gamma = 1.0;

	/* set the parent mode */
	rc = ggiCheckTextMode(priv->parent, child_size.x, child_size.y, 
		child_size.x, child_size.y, GGI_AUTO, GGI_AUTO,
		priv->parent_gt.graphtype, &sug_mode);
	rc = ggiSetMode(priv->parent, &sug_mode);
	if (rc < 0) {
		DPRINT("Couldn't set child graphic mode.\n");
		return rc;
	}

  	/* now calculate the conversion */
	setup_rgb2grey_table(priv->rgb_to_grey);
	setup_templates(priv->accuracy);

	/* setup tables and choose blitter function */
	if ((priv->accuracy.x == 1) && (priv->accuracy.y == 1)) {
		priv->do_blit = &blitter_1x1;
		
	} else if ((priv->accuracy.x == 1) && (priv->accuracy.y == 2)) {
		priv->do_blit = &blitter_1x2;
		
	} else if ((priv->accuracy.x == 2) && (priv->accuracy.y == 2)) {
		priv->do_blit = &blitter_2x2;
		
	} else if ((priv->accuracy.x == 2) && (priv->accuracy.y == 4)) {
		priv->do_blit = &blitter_2x4;
		
	} else if ((priv->accuracy.x == 4) && (priv->accuracy.y == 4)) {
		priv->do_blit = &blitter_4x4;

	} else {
		ggPanic("display-monotext: INTERNAL ERROR: "
			"ACCURACY %dx%d not supported.\n", 
			priv->accuracy.x, priv->accuracy.y);
		exit(1);
	}

	
	/* Setup the greyblock_to_ascii[] array.  We are using lazy
	 * evaluation, which means that we do the expensive "template ->
	 * character" mapping on-the-fly.  Thus saying goodbye to the
	 * long delays at startup.
	 */

	memset(greyblock_to_ascii, 255, sizeof(greyblock_to_ascii));


	/* clear the 'dirty region' */

	priv->dirty_tl.x = priv->size.x;
	priv->dirty_tl.y = priv->size.y;
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;

	return 0;
}

int _ggi_monotext_close(struct ggi_visual *vis)
{
	ggi_monotext_priv *priv = MONOTEXT_PRIV(vis);

	if (priv->colormap != NULL) {
		free(priv->colormap);
	}
	if (priv->greymap != NULL) {
		free(priv->greymap);
	}
	if (priv->rgb_to_grey != NULL) {
		free(priv->rgb_to_grey);
	}

	ggiClose(priv->parent);

	return 0;
}
