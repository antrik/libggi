/* $Id: color.c,v 1.6 2004/11/14 15:47:48 cegger Exp $
******************************************************************************

   Terminfo target

   Copyright (C) 1998 MenTaLguY         [mentalg@geocities.com]

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

#include "TIvisual.h"

#define GGI_COLOR_COMPONENT_TYPE uint16

#define THREE_THIRDS (GGI_COLOR_COMPONENT_TYPE)((1<<(GGI_COLOR_PRECISION+1))-1)
#define TWO_THIRDS (GGI_COLOR_COMPONENT_TYPE)((1<<(GGI_COLOR_PRECISION+2))/3)
#define ONE_THIRD (GGI_COLOR_COMPONENT_TYPE)((1<<(GGI_COLOR_PRECISION+1))/3) 


#if 0
static const ggi_color vga16_palette[16] = {
	{ 0, 0, 0 },
	{ 0, 0, TWO_THIRDS },
	{ 0, TWO_THIRDS, 0 },
	{ 0, TWO_THIRDS, TWO_THIRDS },
	{ TWO_THIRDS, 0, 0 },
	{ TWO_THIRDS, 0, TWO_THIRDS },
	{ TWO_THIRDS, ONE_THIRD, 0 }, 
	{ TWO_THIRDS, TWO_THIRDS, TWO_THIRDS },
	{ ONE_THIRD, ONE_THIRD, ONE_THIRD },
	{ ONE_THIRD, ONE_THIRD, THREE_THIRDS },
	{ ONE_THIRD, THREE_THIRDS, THREE_THIRDS },
	{ THREE_THIRDS, ONE_THIRD, ONE_THIRD },
	{ THREE_THIRDS, ONE_THIRD, THREE_THIRDS },
	{ THREE_THIRDS, THREE_THIRDS, ONE_THIRD },
	{ THREE_THIRDS, THREE_THIRDS, THREE_THIRDS }
};
#endif

#undef ONE_THIRD
#undef TWO_THIRDS
#undef THREE_THIRDS

#undef GGI_COLOR_COMPONENT_TYPE

static inline chtype map_char_to_chtype(ggi_visual *vis, unsigned int c) {
	struct TIhooks *tiinfo;
	tiinfo = TERMINFO_PRIV(vis);
	switch (c) {
		case 0x00: return ' ';
		default:   return tiinfo->charmap[c];
	}
}

static inline chtype get_color_pair(ggi_visual *vis, int fg, int bg) {
	if ( COLOR_PAIRS == 0 ) return 0;
	return COLOR_PAIR(((COLORS-fg%COLORS-1)+(bg%COLORS*COLORS))%COLOR_PAIRS);
}

static inline chtype map_text16_to_ncurses(ggi_visual *vis, uint16 pixel)
{
	struct TIhooks *tiinfo = TERMINFO_PRIV(vis);

	int fg = (pixel >> 8)  & 0x0F;
	int bg = (pixel >> 12) & 0x0F;

	return map_char_to_chtype(vis, pixel & 0xFFU)
		| tiinfo->color16_table[fg+(bg<<4)];
}

static inline chtype map_text32_to_ncurses(ggi_visual *vis, uint32 pixel)
{
	chtype attributes = A_NORMAL;

	int fg = (pixel & ATTR_FGCOLOR) >> 8;
	int bg = (pixel & ATTR_BGCOLOR);

	attributes |= ( pixel & ATTR_HALF )      ? A_DIM : 0;
	attributes |= ( pixel & ATTR_BRIGHT )    ? A_STANDOUT : 0;
	attributes |= ( pixel & ATTR_UNDERLINE ) ? A_UNDERLINE : 0;
	attributes |= ( pixel & ATTR_BOLD )      ? A_BOLD : 0;
	attributes |= ( pixel & ATTR_ITALIC )    ? A_STANDOUT : 0;
	attributes |= ( pixel & ATTR_REVERSE )   ? A_REVERSE : 0;
	attributes |= ( pixel & ATTR_BLINK )     ? A_BLINK : 0;
	attributes |= ( ( 1 << 23 ) & pixel )    ? A_ALTCHARSET : 0;

	return map_char_to_chtype(vis, (pixel & ATTR_FONT) >> 24) | 
		attributes | get_color_pair(vis, fg, bg);
}

static int paint_ncurses_window16(ggi_visual *vis, WINDOW *win, int width,
                                  int height)
{
	uint16 *fb_walk;
	int splitline;
	int x_limit, y_limit;
	int fb_width;
	int x, y;
	chtype *linebuffer;
	size_t buffer_size;

	{
		int fb_height, vis_width, vis_height;

		fb_walk = LIBGGI_CURREAD(vis);
		fb_width = LIBGGI_VIRTX(vis);
		fb_height = LIBGGI_VIRTY(vis);

		vis_width = LIBGGI_X(vis);
		vis_height = LIBGGI_Y(vis);

		x_limit = ( width < vis_width ) ? width : vis_width;
		y_limit = ( height < vis_height ) ? height : vis_height;

		fb_walk += ( vis->origin_x + vis->origin_y * fb_width );

		splitline = TERMINFO_PRIV(vis)->splitline;

		buffer_size = sizeof(chtype) * width;
		linebuffer = malloc(buffer_size);	
		memset(linebuffer, 0, buffer_size);
	}

	for ( y = 0 ; y < y_limit ; y++ ) {
		if ( y == splitline ) {
			fb_walk = LIBGGI_CURREAD(vis);
		}
		for ( x = 0 ; x < x_limit ; x++ ) {
			linebuffer[x] = map_text16_to_ncurses(vis, fb_walk[x]);
		}
		fb_walk += fb_width;
		mvwaddchnstr(win, y, 0, linebuffer, width);
	}

	if ( y < height ) {
		memset(linebuffer, 0, buffer_size);
		for ( ; y < height ; y++ ) {
			mvwaddchnstr(win, y, 0, linebuffer, width);
		}
	}
	
	free(linebuffer);
	return 0;
}

static int paint_ncurses_window32(ggi_visual *vis, WINDOW *win, int width,
                                  int height)
{
	uint32 *fb_walk;
	int splitline;
	int x_limit, y_limit;
	int fb_width;
	int x, y;
	chtype *linebuffer;
	size_t buffer_size;

	{
		int fb_height, vis_width, vis_height;

		fb_walk = LIBGGI_CURREAD(vis);
		fb_width = LIBGGI_VIRTX(vis);
		fb_height = LIBGGI_VIRTY(vis);

		vis_width = LIBGGI_X(vis);
		vis_height = LIBGGI_Y(vis);

		x_limit = ( width < vis_width ) ? width : vis_width;
		y_limit = ( height < vis_height ) ? height : vis_height;

		fb_walk += vis->origin_x + vis->origin_y * fb_width;
		splitline = TERMINFO_PRIV(vis)->splitline;

		buffer_size = sizeof(chtype) * width;
		linebuffer = malloc(buffer_size);	
		memset(linebuffer, 0, buffer_size);
	}

	for ( y = 0 ; y < y_limit ; y++ ) {
		if ( y == splitline ) {
			fb_walk = LIBGGI_CURREAD(vis);
		}
		for ( x = 0 ; x < x_limit ; x++ ) {
			linebuffer[x] = map_text32_to_ncurses(vis,
					fb_walk[x]);
		}
		fb_walk += fb_width;
		mvwaddchnstr(win, y, 0, linebuffer, width);
	}

	if ( y < height ) {
		memset(linebuffer, 0, buffer_size);
		for ( ; y < height ; y++ ) {
			mvwaddchnstr(win, y, 0, linebuffer, width);
		}
	} 
	free(linebuffer);

	return 0;
}

int paint_ncurses_window(ggi_visual *vis, WINDOW *win, int width, int height)
{
	switch (LIBGGI_GT(vis)) {
	case GT_TEXT16: return paint_ncurses_window16(vis, win, width, height);
	case GT_TEXT32: return paint_ncurses_window32(vis, win, width, height);
	default: return GGI_ENOMATCH;
	}
}

#if 0
ggi_pixel GGI_terminfo_mapcolor(ggi_visual *vis, ggi_color *col)
{
	switch (LIBGGI_GT(vis)) {
	case GT_TEXT16: {
		long distance;
		int current;
		long closest_distance=(1<<(GGI_COLOR_PRECISION+1))*3;
		int closest=0;
		for ( current = 0 ; current < 16 ; current++ ) {
			distance = abs(col->r - vga16_palette[current].r);
			distance += abs(col->g - vga16_palette[current].g);
			distance += abs(col->b - vga16_palette[current].b);
			if ( distance <= closest_distance ) {
				closest = current;
			}
		}
		return (closest<<8)|219;
	}
	/* FIXME text32 support needed, but text32 pixel bitmapping may change soon */
	default:
		return GGI_ENOMATCH;
	}
}

int GGI_terminfo_unmappixel(ggi_visual *vis, ggi_pixel pix, ggi_color *col)
{
	switch (LIBGGI_GT(vis)) {
	case GT_TEXT16: {
		pix = ( pix >> 8 ) & 0x0F;
		memcpy(col, &vga16_palette[pix%16], sizeof(ggi_color));
		return 0;
	}
	/* FIXME text32 support needed, but text32 pixel bitmapping may change soon */
	default:
		return GGI_ENOMATCH;
	}
}
#endif
