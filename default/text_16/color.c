/* $Id: color.c,v 1.1 2001/05/12 23:01:50 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Andrew Apted     [andrew@ggi-project.org]

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

#include "t16lib.h"


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

ggi_pixel GGI_t16_mapcolor(ggi_visual *vis, ggi_color *col)
{
	int index;

	index = _ggi_match_palette(ansi_16_colors, 16, col);

	/* Now put this color into the *foreground* part of the text
	 * pixel, and choose a character that is as solid as possible.
	 */

	return (index == 0) ? 0x20 /* space */ :
		((index << 8) | 0xdb /* block */ );
}

/* Map pixel to color
 */

int GGI_t16_unmappixel(ggi_visual *vis, ggi_pixel pixel, ggi_color *col)
{
	*col = ansi_16_colors[(pixel & 0x0f00) >> 8];

	return 0;
}
