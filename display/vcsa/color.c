/* $Id: color.c,v 1.5 2004/11/25 16:56:40 cegger Exp $
******************************************************************************

   Display-VCSA: color mapping

   Copyright (C) 1998-1999  Andrew Apted    [andrew@ggi-project.org]

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
#include <ggi/display/vcsa.h>


#ifndef MAX
#define MAX(a, b)  ((a) > (b) ? (a) : (b))
#endif


static ggi_color ansi_16_colors[16] =
{
	{ 0x0000, 0x0000, 0x0000 },
	{ 0x0000, 0x0000, 0xaaaa },
	{ 0x0000, 0xaaaa, 0x0000 },
	{ 0x0000, 0xaaaa, 0xaaaa },
	{ 0xaaaa, 0x0000, 0x0000 },
	{ 0xaaaa, 0x0000, 0xaaaa },
	{ 0xaaaa, 0xaaaa, 0x0000 },
	{ 0xaaaa, 0xaaaa, 0xaaaa },

	{ 0x5555, 0x5555, 0x5555 },
	{ 0x5555, 0x5555, 0xffff },
	{ 0x5555, 0xffff, 0x5555 },
	{ 0x5555, 0xffff, 0xffff },
	{ 0xffff, 0x5555, 0x5555 },
	{ 0xffff, 0x5555, 0xffff },
	{ 0xffff, 0xffff, 0x5555 },
	{ 0xffff, 0xffff, 0xffff }
};

static ggi_color outer_7_colors[7] =
{
	/* Note: we accentuate the "pure" colors (red, green and blue)
	 * by moving them closer to the center of their face.  [We also
	 * accentuate the other non-white colors, but not as much].
	 */
	
	{ 0x2000, 0x2000, 0xffff },
	{ 0x2000, 0xffff, 0x2000 },
	{ 0x1000, 0xffff, 0xffff },
	{ 0xffff, 0x2000, 0x2000 },
	{ 0xffff, 0x1000, 0xffff },
	{ 0xffff, 0xffff, 0x1000 },
	{ 0xffff, 0xffff, 0xffff }
};

#define NUM_INTENS  7

static ggi_pixel vcsa_normal_pixels[7 * NUM_INTENS] =
{
	0x0020, 0x01b0, 0x01b1, 0x01b2, 0x01db, 0x09b2, 0x09db,
	0x0020, 0x02b0, 0x02b1, 0x02b2, 0x02db, 0x0ab2, 0x0adb,
	0x0020, 0x03b0, 0x03b1, 0x03b2, 0x03db, 0x0bb2, 0x0bdb,
	0x0020, 0x04b0, 0x04b1, 0x04b2, 0x04db, 0x0cb2, 0x0cdb,
	0x0020, 0x05b0, 0x05b1, 0x05b2, 0x05db, 0x0db2, 0x0ddb,
	0x0020, 0x06b0, 0x06b1, 0x06b2, 0x06db, 0x0eb2, 0x0edb,
	0x0020, 0x07b0, 0x07b1, 0x07b2, 0x07db, 0x0fb2, 0x0fdb
};

static ggi_pixel vcsa_ascii_pixels[7 * NUM_INTENS] =
{
	0x0020, 0x012e, 0x013a, 0x0125, 0x014d, 0x0925, 0x094d,
	0x0020, 0x022e, 0x023a, 0x0225, 0x024d, 0x0a25, 0x0a4d,
	0x0020, 0x032e, 0x033a, 0x0325, 0x034d, 0x0b25, 0x0b4d,
	0x0020, 0x042e, 0x043a, 0x0425, 0x044d, 0x0c25, 0x0c4d,
	0x0020, 0x052e, 0x053a, 0x0525, 0x054d, 0x0d25, 0x0d4d,
	0x0020, 0x062e, 0x063a, 0x0625, 0x064d, 0x0e25, 0x0e4d,
	0x0020, 0x072e, 0x073a, 0x0725, 0x074d, 0x0f25, 0x0f4d
};


ggi_pixel GGI_vcsa_mapcolor(ggi_visual *vis, const ggi_color *col)
{
	ggi_vcsa_priv *priv = VCSA_PRIV(vis);
	ggi_color outer;
	int intens, _index;

	if (! (priv->flags & VCSA_FLAG_SHADE)) {
		
		_index = _ggi_match_palette(ansi_16_colors, 16, col);

		if (priv->flags & VCSA_FLAG_ASCII) {
			return (_index << 8) | '#';
		}

		return (_index << 8) | 0xdb;
	}
	
	outer.r = col->r >> 4;
	outer.g = col->g >> 4;
	outer.b = col->b >> 4;

	intens = MAX(outer.r, MAX(outer.g, outer.b));

	if (intens < 0x0100) {
		/* virtually black */
		return 0x0020;
	}

	outer.r = outer.r * 0xffff / intens;
	outer.g = outer.g * 0xffff / intens;
	outer.b = outer.b * 0xffff / intens;

	_index = _ggi_match_palette(outer_7_colors, 7, &outer);

	intens = intens * NUM_INTENS / 0x1000;

	if (priv->flags & VCSA_FLAG_ASCII) {
		return vcsa_ascii_pixels[_index * NUM_INTENS + intens];
	}

	return vcsa_normal_pixels[_index * NUM_INTENS + intens];
}

int GGI_vcsa_unmappixel(ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	*col = ansi_16_colors[(pixel & 0x0f00) >> 8];

	return 0;
}
