/* $Id: trueemu.c,v 1.10 2006/04/05 04:11:37 cegger Exp $
******************************************************************************

   Display-trueemu : truecolor emulation library.

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
#include <ggi/display/trueemu.h>
#include <ggi/internal/ggi_debug.h>

#include <string.h>


/**************************************************
 ***
 ***  Various Defines
 ***
 **************************************************/


#define NUM_PASTELS	21

#define CUBE_LEVELS	6
#define PASTEL_LEVELS	12	/* not including black */

#define C16_GREYS	6	/* not including black */
#define C16_BROWNS	2
#define C16_GREENS	2

#define NO_F  -1,-1	/* means: not this face */


/***
 ***  Lookup Tables
 ***/


typedef struct pastel
{
	ggi_color color;

	TrueColor RF_min, RF_max;	/* area on red face */
	TrueColor GF_min, GF_max;	/* area on green face */
	TrueColor BF_min, BF_max;	/* area on blue face */

} Pastel;


static Pastel pastel_array[NUM_PASTELS] =
{
	/* extra colors */

	{ T2GGI(0xffbc7c), 0xffa055,0xffe0a0, NO_F, NO_F },
	{ T2GGI(0xffdcb0), 0xffa0a0,0xffe8d8, NO_F, NO_F },

	/* standard 19 points on the outer RGB cube face */

	{ T2GGI(0x0000ff), NO_F, NO_F, 0x0000ff,0x5555ff },
	{ T2GGI(0x0080ff), NO_F, NO_F, 0x0055ff,0x55aaff },
	{ T2GGI(0x8000ff), NO_F, NO_F, 0x5500ff,0xaa55ff },
	{ T2GGI(0x8080ff), NO_F, NO_F, 0x5555ff,0xaaaaff },

	{ T2GGI(0x00ff00), NO_F, 0x00ff00,0x55ff55, NO_F },
	{ T2GGI(0x00ff80), NO_F, 0x00ff55,0x55ffaa, NO_F },
	{ T2GGI(0x80ff00), NO_F, 0x55ff00,0xaaff55, NO_F },
	{ T2GGI(0x80ff80), NO_F, 0x55ff55,0xaaffaa, NO_F },
	{ T2GGI(0x00ffff), NO_F, 0x00ffaa,0x55ffff, 0x00aaff,0x55ffff },
	{ T2GGI(0x80ffff), NO_F, 0x55ffaa,0xaaffff, 0x55aaff,0xaaffff },

	{ T2GGI(0xff0000), 0xff0000,0xff5555, NO_F, NO_F },
	{ T2GGI(0xff0080), 0xff0055,0xff55aa, NO_F, NO_F },
	{ T2GGI(0xff8000), 0xff5500,0xffaa55, NO_F, NO_F },
	{ T2GGI(0xff8080), 0xff5555,0xffaaaa, NO_F, NO_F },
	{ T2GGI(0xff00ff), 0xff00aa,0xff55ff, NO_F, 0xaa00ff,0xff55ff },
	{ T2GGI(0xff80ff), 0xff55aa,0xffaaff, NO_F, 0xaa55ff,0xffaaff },
	{ T2GGI(0xffff00), 0xffaa00,0xffff55, 0xaaff00,0xffff55, NO_F },
	{ T2GGI(0xffff8c), 0xffaa55,0xffffaa, 0xaaff55,0xffffc7, NO_F },
	{ T2GGI(0xffffff), 0xffaaaa,0xffffff, 0xaaffc7,0xffffff,
			   0xaaaaff,0xffffff }
};


static ggi_color col16_palette[16] =
{
	T2GGI(0x000000),	/* standard 2x2x2 cube */
	T2GGI(0x0000ff),
	T2GGI(0x00ff00),
	T2GGI(0x00ffff),
	T2GGI(0xff0000),
	T2GGI(0xff00ff),
	T2GGI(0xffff00),
	T2GGI(0xffffff),

	T2GGI(0x2a2a2a),	/* five extra greys */
	T2GGI(0x555555),
	T2GGI(0x7f7f7f),
	T2GGI(0xaaaaaa),
	T2GGI(0xd5d5d5),

	T2GGI(0x007f00),	/* extra green */

	T2GGI(0x7f5839),	/* extra brown & orange */
	T2GGI(0xffb172)
};

static int col16_greys [C16_GREYS+1]  = { 0,8,9,10,11,12,7 };
static int col16_browns[C16_BROWNS+1] = { 0,14,15 };
static int col16_greens[C16_GREENS+1] = { 0,13,2 };

static ggi_color black = { 0,0,0 };


/**************************************************
 ***
 ***  Internal functions
 ***
 **************************************************/


static int calc_default_flags(int flags, ggi_graphtype graphtype)
{
	int default_dither  = TRUEEMU_F_DITHER_4;
	int default_palette = TRUEEMU_F_RGB;

	if (GT_SCHEME(graphtype) == GT_PALETTE) {

		if (GT_DEPTH(graphtype) > 4) {
			default_palette = TRUEEMU_F_CUBE;
		} else {
			default_palette = TRUEEMU_F_PASTEL;
		}
	}

	if ((flags & TE_DITHER_MASK) == 0) {
		flags |= default_dither;
	}

	if ((flags & TE_PALETTE_MASK) == 0) {
		flags |= default_palette;
	}

	return flags;
}


/***
 ***  Palette routines
 ***/


static void load_332_palette(ggi_color *colormap)
{
	int r, g, b;

	for (r = 0; r < 8; r++)
	for (g = 0; g < 8; g++)
	for (b = 0; b < 4; b++) {

		int _index = (r << 5) | (g << 2) | b;

		ggi_color col;

		col.r = r * 0xffff / (8-1);
		col.g = g * 0xffff / (8-1);
		col.b = b * 0xffff / (4-1);

		memcpy(&colormap[_index], &col, sizeof(ggi_color));
	}
}


static void load_cube_palette(ggi_color *colormap)
{
	int r, g, b;

	for (r = 0; r < CUBE_LEVELS; r++)
	for (g = 0; g < CUBE_LEVELS; g++)
	for (b = 0; b < CUBE_LEVELS; b++) {

		int _index = (r * CUBE_LEVELS + g) * CUBE_LEVELS + b;

		ggi_color col;

		col.r = r * 0xffff / (CUBE_LEVELS-1);
		col.g = g * 0xffff / (CUBE_LEVELS-1);
		col.b = b * 0xffff / (CUBE_LEVELS-1);

		memcpy(&colormap[_index], &col, sizeof(ggi_color));
	}
}


static void load_pastel_palette(ggi_color *colormap)
{
	int _index;
	int pastel;


	/* color 0 is black */

	colormap[0] = black;


	for (pastel = 0; pastel < NUM_PASTELS; pastel++)
	{
		int pl;

		int tr = pastel_array[pastel].color.r;
		int tg = pastel_array[pastel].color.g;
		int tb = pastel_array[pastel].color.b;

		_index = 1 + pastel * PASTEL_LEVELS;

		for (pl=1; pl <= PASTEL_LEVELS; pl++) {

			ggi_color col;

			col.r = pl * tr / PASTEL_LEVELS;
			col.g = pl * tg / PASTEL_LEVELS;
			col.b = pl * tb / PASTEL_LEVELS;

			memcpy(&colormap[_index++], &col, sizeof(ggi_color));
		}
	}
}


static void load_col16_palette(ggi_color *colormap)
{
	int i;

	for (i=0; i < 16; i++) {
		colormap[i] = col16_palette[i];
	}
}


static void load_121_palette(ggi_color *colormap)
{
	int r, g, b;

	for (r = 0; r < 2; r++)
	for (g = 0; g < 4; g++)
	for (b = 0; b < 2; b++) {

		int _index = (r << 3) | (g << 1) | b;

		ggi_color col;

		col.r = r * 0xffff / (2-1);
		col.g = g * 0xffff / (4-1);
		col.b = b * 0xffff / (2-1);

		memcpy(&colormap[_index], &col, sizeof(ggi_color));
	}
}


static void setup_palette(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	ggi_color colormap[256];


	if (GT_SCHEME(priv->mode.graphtype) != GT_PALETTE) {
		return;
	}

	switch (GT_DEPTH(priv->mode.graphtype))
	{
		case 8:
		{
			if (priv->flags & TRUEEMU_F_PASTEL) {
				load_pastel_palette(colormap);

			} else if (priv->flags & TRUEEMU_F_CUBE) {
				load_cube_palette(colormap);

			} else {
				load_332_palette(colormap);
			}

			ggiSetPalette(priv->parent->stem, 0, 256, colormap);
			ggiFlush(priv->parent->stem);
			return;
		}

		case 4:
		{
			if (priv->flags & TRUEEMU_F_PASTEL) {
				load_col16_palette(colormap);
			} else {
				load_121_palette(colormap);
			}

			ggiSetPalette(priv->parent->stem, 0, 16, colormap);
			ggiFlush(priv->parent->stem);
			return;
		}
	}

	fprintf(stderr, "trueemu: INTERNAL ERROR\n");
}


/***
 ***  Dithering calculations
 ***/


static int lookup_pastel(int r, int g, int b)
{
	int tr, tg, tb;
	int pastel;

	int max = MAX(r, MAX(g, b));


	if (max == 0) {

		/* Completely black, therefore it doesn't matter which
		 * pastel we choose.  Just choose the first one.
		 */

		return 0;
	}


	/* project color onto the RGB cube face */

	tr = (r * 0xff) / max;
	tg = (g * 0xff) / max;
	tb = (b * 0xff) / max;


	/* find match in pastel array */

	for (pastel=0; pastel < NUM_PASTELS; pastel++) {

		Pastel *p = & pastel_array[pastel];


		if ((tr == 0xff) && ! (p->RF_min < 0)) {

			/* try the red face */

			if ((TC_GREEN(p->RF_min) <= tg) &&
				(tg <= TC_GREEN(p->RF_max)) &&
				(TC_BLUE(p->RF_min) <= tb) &&
				(tb <= TC_BLUE(p->RF_max))) {

				return pastel;
			}
		}

		if ((tg == 0xff) && ! (p->GF_min < 0)) {

			/* try the green face */

			if ((TC_RED(p->GF_min) <= tr) &&
				(tr <= TC_RED(p->GF_max)) &&
				(TC_BLUE(p->GF_min) <= tb) &&
				(tb <= TC_BLUE(p->GF_max))) {

				return pastel;
			}
		}

		if ((tb == 0xff) && ! (p->BF_min < 0)) {

			/* try the blue face */

			if ((TC_RED(p->BF_min) <= tr) &&
				(tr <= TC_RED(p->BF_max)) &&
				(TC_GREEN(p->BF_min) <= tg) &&
				(tg <= TC_GREEN(p->BF_max))) {

				return pastel;
			}
		}
	}


	/* Oops, pastel array has a hole in it */

	ggPanic("Pastel array has a hole in it.\n");
	return 0;
}


static void calc_hi16_dither(ggi_trueemu_priv *priv, int shift)
{
	int i, n;

	int num = 1 << shift;

	int R_bands = num * 32;
	int G_bands = num * 64;
	int B_bands = num * 32;


	/* allocate lookup tables */

	if (priv->R == NULL) {
		priv->R = _ggi_malloc(256 * 4 * 2);
		priv->G = _ggi_malloc(256 * 4 * 2);
		priv->B = _ggi_malloc(256 * 4 * 2);
	}


	for (i=0; i < 256; i++)     /* intensity */
	for (n=0; n < num; n++) {

	 	int hr = ((i * R_bands) >> 8) - (num-1) + n;
	 	int hg = ((i * G_bands) >> 8) - (num-1) + n;
	 	int hb = ((i * B_bands) >> 8) - (num-1) + n;

		hr = (hr < 0) ? 0 : (hr >> shift);
		hg = (hg < 0) ? 0 : (hg >> shift);
		hb = (hb < 0) ? 0 : (hb >> shift);

		priv->R[i][n] = (hr << 11);
		priv->G[i][n] = (hg << 5);
		priv->B[i][n] = (hb);
	}
}


static void calc_hi15_dither(ggi_trueemu_priv *priv, int shift)
{
	int i, n;

	int num = 1 << shift;
	int bands = num * 32;


	/* allocate lookup tables */

	if (priv->R == NULL) {
		priv->R = _ggi_malloc(256 * 4 * 2);
		priv->G = _ggi_malloc(256 * 4 * 2);
		priv->B = _ggi_malloc(256 * 4 * 2);
	}


	for (i=0; i < 256; i++)     /* intensity */
	for (n=0; n < num; n++) {

	 	int h = ((i * bands) >> 8) - (num-1) + n;

		h = (h < 0) ? 0 : (h >> shift);

		priv->R[i][n] = (h << 10);
		priv->G[i][n] = (h << 5);
		priv->B[i][n] = (h << 0);
	}
}


static void calc_332_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int R_bands = 1 + num * (8-1);
	int G_bands = 1 + num * (8-1);
	int B_bands = 1 + num * (4-1);

	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}


	for (r=0; r < 32; r++)
	for (g=0; g < 32; g++)
	for (b=0; b < 32; b++)

	for (n=0; n < num; n++) {

		int cr = (((r * R_bands) >> 5) + n) >> shift;
		int cg = (((g * G_bands) >> 5) + n) >> shift;
		int cb = (((b * B_bands) >> 5) + n) >> shift;

		priv->T[(r << 10) | (g << 5) | b][n] =
			(cr << 5) | (cg << 2) | cb;
	}
}


static void calc_cube_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int bands = 1 + num * (CUBE_LEVELS-1);


	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}


	for (r=0; r < 32; r++)
	for (g=0; g < 32; g++)
	for (b=0; b < 32; b++)

	for (n=0; n < num; n++) {

		int cr = (((r * bands) >> 5) + n) >> shift;
		int cg = (((g * bands) >> 5) + n) >> shift;
		int cb = (((b * bands) >> 5) + n) >> shift;

		priv->T[(r << 10) | (g << 5) | b][n] =
			(cr * CUBE_LEVELS + cg) * CUBE_LEVELS + cb;
	}
}


static void calc_pastel_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int bands = 1 + num * PASTEL_LEVELS;

	/* Note: black acts as an extra pastel level */

	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}

	for (r=0; r < 32; r++) {
	for (g=0; g < 32; g++) {
	for (b=0; b < 32; b++) {
		int _index = (r << 10) | (g << 5) | b;
		int tr = r * 0xff / 31;
		int tg = g * 0xff / 31;
		int tb = b * 0xff / 31;
		int pastel = lookup_pastel(tr, tg, tb);
		int max = MAX(r, MAX(g, b));

		for (n=0; n < num; n++) {
			int pl = (((max * bands) >> 5) + n) >> shift;

			priv->T[_index][n] = (pl == 0) ? 0 :
				1 + (pastel * PASTEL_LEVELS) + (pl - 1);
		}
	}
	}
	}
}


static void calc_col16_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int cube_bands  = 1 + num;
	int grey_bands  = 1 + num * C16_GREYS;
	int brown_bands = 1 + num * C16_BROWNS;
	int green_bands = 1 + num * C16_GREENS;

	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}

	for (r=0; r < 32; r++) {
	for (g=0; g < 32; g++) {
	for (b=0; b < 32; b++) {
	for (n=0; n < num; n++) {
		int _index = (r << 10) | (g << 5) | b;
		int max = MAX(r, MAX(g, b));
		int tr,tg,tb;
		int cr,cg,cb;
		int pl;

		if (max == 0) {
			priv->T[_index][n] = 0;
			continue;
		}

		tr = (r * 0xff) / max;
		tb = (b * 0xff) / max;
		tg = (g * 0xff) / max;

		/* handle greens */
		if ((tg == 0xff) && (tr <= 0xe0) && (tb <= 0xe0)) {

			pl = (((max * green_bands) >> 5) + n) >> shift;

			priv->T[_index][n] = col16_greens[pl];
			continue;
		}

		/* handle browns */
		if ((tr == 0xff) && (tb <= 0xd8) &&
			(0x90 <= tg) && (tg <= 0xe8)) {

			pl = (((max * brown_bands) >> 5) + n) >> shift;

			priv->T[_index][n] = col16_browns[pl];
			continue;
		}

		/* handle greys */
		if (((tr == 0xff) && (tb >= 0xb0) && (tg >= 0xb0)) ||
		    ((tb == 0xff) && (tr >= 0xb0) && (tg >= 0xb0)) ||
		    ((tg == 0xff) && (tr >= 0xb0) && (tb >= 0xb0))) {

			pl = (((max * grey_bands) >> 5) + n) >> shift;

			priv->T[_index][n] = col16_greys[pl];
			continue;
		}

		/* handle all other colors */
		cr = (((r * cube_bands) >> 5) + n) >> shift;
		cg = (((g * cube_bands) >> 5) + n) >> shift;
		cb = (((b * cube_bands) >> 5) + n) >> shift;

		priv->T[_index][n] = ((cr*2) + cg)*2 + cb;
	}
	}
	}
	}
}


static void calc_121_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int R_bands = 1 + num * (2-1);
	int G_bands = 1 + num * (4-1);
	int B_bands = 1 + num * (2-1);

	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}

	for (r=0; r < 32; r++) {
	for (g=0; g < 32; g++) {
	for (b=0; b < 32; b++) {
		for (n=0; n < num; n++) {
			int cr = (((r * R_bands) >> 5) + n) >> shift;
			int cg = (((g * G_bands) >> 5) + n) >> shift;
			int cb = (((b * B_bands) >> 5) + n) >> shift;

			priv->T[(r << 10) | (g << 5) | b][n] =
				(cr << 3) | (cg << 1) | cb;
		}
	}
	}
	}
}

#if 0
static void calc_bw_dither(ggi_trueemu_priv *priv, int shift)
{
	int r, g, b, n;
	int num = 1 << shift;
	int bands = 1 + num * (2-1);

	/* allocate lookup table */
	if (priv->T == NULL) {
		priv->T = _ggi_malloc(32768 * 4);
	}

	for (r=0; r < 32; r++) {
	for (g=0; g < 32; g++) {
	for (b=0; b < 32; b++) {
		for (n=0; n < num; n++) {
			int max = MAX(r, MAX(g, b));
			int lev = (((max * bands) >> 5) + n) >> shift;
			priv->T[(r << 10) | (g << 5) | b][n] = lev;
		}
	}
	}
	}
}
#endif

static void setup_dithering(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	TrueemuBlits *B;
	int shift=0;

	if (GT_SIZE(LIBGGI_GT(vis)) == 32) {
		B = &_ggi_trueemu_blit32_table;
	} else {
		B = &_ggi_trueemu_blit24_table;
	}

	if (priv->flags & TRUEEMU_F_DITHER_2) {
		shift=1;
	} else if (priv->flags & TRUEEMU_F_DITHER_4) {
		shift=2;
	}

	switch (GT_SIZE(priv->mode.graphtype)) {

		case 32:
			priv->blitter_even = B->blitter_b32_d0;
			priv->blitter_odd  = B->blitter_b32_d0;
			return;

		case 24:
			priv->blitter_even = B->blitter_b24_d0;
			priv->blitter_odd  = B->blitter_b24_d0;
			return;

		case 16:
			if (shift == 1) {
				priv->blitter_even = B->blitter_b16_d2_ev;
				priv->blitter_odd  = B->blitter_b16_d2_od;

			} else if (shift == 2) {
				priv->blitter_even = B->blitter_b16_d4_ev;
				priv->blitter_odd  = B->blitter_b16_d4_od;

			} else {
				priv->blitter_even = B->blitter_b16_d0;
				priv->blitter_odd  = B->blitter_b16_d0;
			}

			if (GT_DEPTH(priv->mode.graphtype) == 15) {
				calc_hi15_dither(priv, shift);
			} else {
				calc_hi16_dither(priv, shift);
			}
			return;

		case 8:
			if (shift == 1) {
				priv->blitter_even = B->blitter_b8_d2_ev;
				priv->blitter_odd  = B->blitter_b8_d2_od;

			} else if (shift == 2) {
				priv->blitter_even = B->blitter_b8_d4_ev;
				priv->blitter_odd  = B->blitter_b8_d4_od;

			} else {
				priv->blitter_even = B->blitter_b8_d0;
				priv->blitter_odd  = B->blitter_b8_d0;
			}

			/* !!! assumes palette */

			if (priv->flags & TRUEEMU_F_PASTEL) {
				calc_pastel_dither(priv, shift);

			} else if (priv->flags & TRUEEMU_F_CUBE) {
				calc_cube_dither(priv, shift);

			} else {
				calc_332_dither(priv, shift);
			}
			return;

		case 4:
			if (shift == 1) {
				priv->blitter_even = B->blitter_b4_d2_ev;
				priv->blitter_odd  = B->blitter_b4_d2_od;

			} else if (shift == 2) {
				priv->blitter_even = B->blitter_b4_d4_ev;
				priv->blitter_odd  = B->blitter_b4_d4_od;

			} else {
				priv->blitter_even = B->blitter_b4_d0;
				priv->blitter_odd  = B->blitter_b4_d0;
			}

			/* !!! assumes palette */

			if (priv->flags & TRUEEMU_F_PASTEL) {
				calc_col16_dither(priv, shift);
			} else {
				calc_121_dither(priv, shift);
			}
			return;
	}

	fprintf(stderr, "trueemu: INTERNAL ERROR\n");
}


/**************************************************
 ***
 ***  Exported functions
 ***
 **************************************************/


int _ggi_trueemu_Transfer(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	/* round down x if odd */
	if (x & 1) {
		x--; w++;
	}

	/* do transfer, one horizontal line at a time */
	for (; h > 0; h--, y++) {
		/* !!! FIX for FRAMES */
		ggiGetHLine(vis->stem, x, y, w, priv->src_buf);

		if (y & 1) {
			priv->blitter_odd(priv,  priv->dest_buf,
					  priv->src_buf, w);
		} else {
			priv->blitter_even(priv, priv->dest_buf,
					   priv->src_buf, w);
		}

		ggiPutHLine(priv->parent->stem, x, y, w, priv->dest_buf);
	}

	return 0;
}

int _ggi_trueemu_Flush(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	int sx = priv->dirty_tl.x; int sy = priv->dirty_tl.y;
	int ex = priv->dirty_br.x; int ey = priv->dirty_br.y;

	/* clear the 'dirty region' */
	priv->dirty_tl.x = LIBGGI_VIRTX(vis);
	priv->dirty_tl.y = LIBGGI_VIRTY(vis);
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;

	/* !!! optimization when write_frame != display_frame */
	if ((sx < ex) && (sy < ey)) {

		return _ggi_trueemu_Transfer(vis, sx, sy, ex-sx, ey-sy);
	}

	return 0;
}


int _ggi_trueemu_Close(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	if (priv->src_buf != NULL) {
		free(priv->src_buf);
	}
	if (priv->dest_buf != NULL) {
		free(priv->dest_buf);
	}

	if (priv->R != NULL) {
		free(priv->R);
		free(priv->G);
		free(priv->B);
		priv->R = NULL;
	}
	if (priv->T != NULL) {
		free(priv->T);
		priv->T = NULL;
	}

	return 0;
}


int _ggi_trueemu_Open(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	size_t bufsize;
	int err;

	/* Free any previously allocated resources. */
	_ggi_trueemu_Close(vis);

	priv->flags = calc_default_flags(priv->flags, priv->mode.graphtype);

	/* NOTE: Going straight through to the parent when parent_gt ==
	 * child_gt isn't implemented yet, and probably never will be.
	 */

	/* Set the parent mode */
	err = ggiSetMode(priv->parent->stem, &priv->mode);
	if (err < 0) {
		DPRINT_MODE("display-trueemu: Couldn't set parent mode.\n");
		return err;
	}

	DPRINT_MODE("display-trueemu: parent is %d/%d\n",
		       GT_DEPTH(priv->mode.graphtype),
		       GT_SIZE(priv->mode.graphtype));

	/* allocate buffers */
	bufsize = LIBGGI_VIRTX(vis) * 4;

	priv->src_buf  = _ggi_malloc(bufsize);
	priv->dest_buf = _ggi_malloc(bufsize);

	/* now sort out the dither/palette mode */
	setup_dithering(vis);
	setup_palette(vis);

	/* clear the 'dirty region' */
	priv->dirty_tl.x = LIBGGI_VIRTX(vis);
	priv->dirty_tl.y = LIBGGI_VIRTY(vis);
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;

	return 0;
}


#if 0
int _ggi_trueemu_NewMode(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	priv->flags = calc_default_flags(priv->flags, priv->mode.graphtype);

	/* For palettized modes, we first blank the parent visual to
	 * prevent a nasty colored image during the transition.
	 */

	if (GT_SCHEME(priv->mode.graphtype) == GT_PALETTE) {

		ggi_color black = { 0,0,0 };

		ggiSetGCForeground(priv->parent,
			ggiMapColor(priv->parent, &black));
		ggiFillscreen(priv->parent);
		ggiFlush(priv->parent);
	}

	setup_dithering(vis);
	setup_palette(vis);

	UPDATE_MOD(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	return ggiFlush(vis);
}

static void cycle_dither(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);

	int old_dith = priv->flags & TE_DITHER_MASK;

	priv->flags &= ~TE_DITHER_MASK;

	switch (old_dith)
	{
		case TRUEEMU_F_DITHER_0:
			priv->flags |= TRUEEMU_F_DITHER_2;
			break;

		case TRUEEMU_F_DITHER_2:
			priv->flags |= TRUEEMU_F_DITHER_4;
			break;

		default:
			priv->flags |= TRUEEMU_F_DITHER_0;
			break;
	}

	_ggi_trueemu_NewMode(vis);
}

static void cycle_palette(struct ggi_visual *vis)
{
	ggi_trueemu_priv *priv = TRUEEMU_PRIV(vis);
	int old_pal = priv->flags & TE_PALETTE_MASK;

	priv->flags &= ~TE_PALETTE_MASK;

	switch (old_pal) {
	case TRUEEMU_F_RGB:
		priv->flags |= TRUEEMU_F_CUBE;
		break;

	case TRUEEMU_F_CUBE:
		priv->flags |= TRUEEMU_F_PASTEL;
		break;

	default:
		priv->flags |= TRUEEMU_F_RGB;
		break;
	}

	_ggi_trueemu_NewMode(vis);
}
#endif
