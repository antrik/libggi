/* $Id: color.c,v 1.5 2006/03/12 23:15:13 soyt Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Andrew Apted     [andrew.apted@ggi-project.org]

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

#include "t32lib.h"

	
/* Standard 16 colors */
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

/* Do closest-match-mapping.
 * We use the sum of the squares for distance checking.
 *
 * !!! FIXME: TEXT32 can really do 256 colors.
 */

ggi_pixel GGI_t32_mapcolor(struct ggi_visual *vis, const ggi_color *col)
{
	int i;
	int closest=0;

	uint32_t curdist;


	curdist = (1 << 24) * 4;
	
	for (i=0; i < 16; i++) {

		int dr = (col->r - ansi_16_colors[i].r) >> 4;
		int dg = (col->g - ansi_16_colors[i].g) >> 4;
		int db = (col->b - ansi_16_colors[i].b) >> 4;

		uint32_t dist = (dr*dr) + (dg*dg) + (db*db);

		if (dist == 0) {
			/* direct hit */
			break;
		}

		if (dist < curdist) {
			closest = i;
			curdist = dist;
		}
	}

	/* Now put this color into the *foreground* part of the text
	 * pixel, and choose a character that is as solid as possible.
	 */

	return (closest == 0) ? (0x20 << 24) /* space */ :
		((closest << 8) | (0xdb << 24) /* block */ );

}

/* Map pixel to color
 */

int GGI_t32_unmappixel(struct ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	int fg = (pixel & ATTR_FGCOLOR) >> 8;

	*col = ansi_16_colors[fg & 0x0f];   /* !!! FIXME */

	return 0;
}
