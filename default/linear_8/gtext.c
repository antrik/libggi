/* $Id: gtext.c,v 1.1 2001/05/12 23:01:45 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 MenTaLguY  [mentalg@geocities.com]

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

#include "lin8lib.h"

#include <ggi/internal/font/8x8>

int GGI_lin8_putc(ggi_visual *vis, int x, int y, char c)
{
#define char_width 8
#define char_height 8
	int offset, x_run, y_run;
	uint8 *bitmap;

	bitmap = font + ( c * char_height );
	x_run = char_width;
	y_run = char_height;
	offset = 0;

	{
		int delta;

		delta = LIBGGI_GC(vis)->cliptl.x - x;
		if ( delta > 0 ) {
			if ( delta >= x_run ) {
				return 0;
			} else {
				x_run -= delta;
				offset += delta;
				x += delta;
			}
		}
		delta = ( x + x_run ) - LIBGGI_GC(vis)->clipbr.x;
		if ( delta > 0 ) {
			if ( delta >= x_run ) {
				return 0;
			} else {
				x_run -= delta;
			}
		}
		delta = LIBGGI_GC(vis)->cliptl.y - y;
		if ( delta > 0 ) {
			if ( delta >= y_run ) {
				return 0;
			} else {
				y_run -= delta;
				bitmap += delta;
				y += delta;
			}
		}
		delta = ( y + y_run ) - LIBGGI_GC(vis)->clipbr.y;
		if ( delta > 0 ) {
			if ( delta >= y_run ) {
				return 0;
			} else {
				y_run -= delta;
			}
		}
	}

	{
		int y_iter;
		register uint8 *fb;
		int add_stride;

		PREPARE_FB(vis);

		add_stride = LIBGGI_FB_W_STRIDE(vis);
		fb = (uint8 *)LIBGGI_CURWRITE(vis) + ( y * add_stride ) + x;
		add_stride -= x_run;
		y_run += y; x_run += x;

		for ( y_iter = y ; y_iter < y_run ; y_iter++, bitmap++,
		      fb += add_stride )
		{
			register int x_iter;
			register uint8 row;

			row = *bitmap << offset;

			for ( x_iter = x ; x_iter < x_run ; x_iter++,
			      row <<= 1, fb++ )
			{
				*fb = ( row & 128 )
				      ? (uint8)LIBGGI_GC_FGCOLOR(vis)
				      : (uint8)LIBGGI_GC_BGCOLOR(vis);
			}
		}
	}

	return 0;
#undef char_width
#undef char_height
}
