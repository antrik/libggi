/* $Id: line.c,v 1.11 2006/04/28 06:05:37 cegger Exp $
******************************************************************************

   Tile target: clipped-line function in non-DB mode
   From ../../default/common/clip.c

   Copyright (C) 1998 Steve Cheng       [steve@ggi-project.org]
   Copyright (C) 1998 Alexander Larsson [alla@lysator.liu.se]

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
#include <ggi/display/tile.h>


/*
  This is a line-clipper using the algorithm by cohen-sutherland.

  It is modified to do pixel-perfect clipping. This means that it
  will generate the same endpoints that would be drawn if an ordinary
  bresenham line-drawer where used and only visible pixels drawn.

  It can be used with a bresenham-like linedrawer if it is modified to
  start with a correct error-term.
*/

#define OC_LEFT 1
#define OC_RIGHT 2
#define OC_TOP 4
#define OC_BOTTOM 8

/* Outcodes:
+-> x
|       |      |
V  0101 | 0100 | 0110
y ---------------------
   0001 | 0000 | 0010
  ---------------------
   1001 | 1000 | 1010
        |      |
 */
#define outcode(code, xx, yy) \
{\
  code = 0;\
  if ((xx)<cliptl.x)\
    code |= OC_LEFT;\
  else if ((xx)>=clipbr.x)\
    code |= OC_RIGHT;\
  if ((yy)<cliptl.y)\
    code |= OC_TOP;\
  else if ((yy)>=clipbr.y)\
    code |= OC_BOTTOM;\
}

/*
  Calculates |_ a/b _| with mathematically correct floor
  */
static int FloorDiv(int a, int b)
{
	int _floor;
	if (b > 0) {
		if (a > 0) {
			return a / b;
		} else {
			_floor = -((-a) / b);
			if ((-a) % b != 0)
				_floor--;
		}
		return _floor;
	} else {
		if (a > 0) {
			_floor = -(a / (-b));
			if (a % (-b) != 0)
				_floor--;
			return _floor;
		} else {
			return (-a) / (-b);
		}
	}
}

/*
  Calculates |^ a/b ^| with mathematically correct ceil
  */
static int CeilDiv(int a, int b)
{
	if (b > 0)
		return FloorDiv(a - 1, b) + 1;
	else
		return FloorDiv(-a - 1, -b) + 1;
}

static int _ggi_clip2d(ggi_coord cliptl, ggi_coord clipbr,
		       int *_x0, int *_y0, int *_x1, int *_y1)
{
	int first, last, code;
	int x0, y0, x1, y1;
	int x, y;
	int dx, dy;
	int xmajor;
	int slope;

	first = 0;
	last = 0;
	outcode(first, *_x0, *_y0);
	outcode(last, *_x1, *_y1);

	if ((first | last) == 0) {
		return 1;	/* Trivially accepted! */
	}
	if ((first & last) != 0) {
		return 0;	/* Trivially rejected! */
	}

	x0 = *_x0;
	y0 = *_y0;
	x1 = *_x1;
	y1 = *_y1;

	dx = x1 - x0;
	dy = y1 - y0;

	xmajor = (abs(dx) > abs(dy));
	slope = ((dx >= 0) && (dy >= 0)) || ((dx < 0) && (dy < 0));

	for (;;) {
		code = first;
		if (first == 0)
			code = last;

		if (code & OC_LEFT) {
			x = cliptl.x;
			if (xmajor) {
				y = *_y0 + FloorDiv(dy * (x - *_x0) * 2 +
						    dx, 2 * dx);
			} else {
				if (slope) {
					y = *_y0 +
					    CeilDiv(dy *
						    ((x - *_x0) * 2 - 1),
						    2 * dx);
				} else {
					y = *_y0 +
					    FloorDiv(dy *
						     ((x - *_x0) * 2 - 1),
						     2 * dx);
				}
			}
		} else if (code & OC_RIGHT) {
			x = clipbr.x - 1;
			if (xmajor) {
				y = *_y0 + FloorDiv(dy * (x - *_x0) * 2 +
						    dx, 2 * dx);
			} else {
				if (slope) {
					y = *_y0 +
					    CeilDiv(dy *
						    ((x - *_x0) * 2 + 1),
						    2 * dx) - 1;
				} else {
					y = *_y0 +
					    FloorDiv(dy *
						     ((x - *_x0) * 2 + 1),
						     2 * dx) + 1;
				}
			}
		} else if (code & OC_TOP) {
			y = cliptl.y;
			if (xmajor) {
				if (slope) {
					x = *_x0 +
					    CeilDiv(dx *
						    ((y - *_y0) * 2 - 1),
						    2 * dy);
				} else {
					x = *_x0 +
					    FloorDiv(dx *
						     ((y - *_y0) * 2 - 1),
						     2 * dy);
				}
			} else {
				x = *_x0 + FloorDiv(dx * (y - *_y0) * 2 +
						    dy, 2 * dy);
			}
		} else {	/* OC_BOTTOM */
			y = clipbr.y - 1;
			if (xmajor) {
				if (slope) {
					x = *_x0 +
					    CeilDiv(dx *
						    ((y - *_y0) * 2 + 1),
						    2 * dy) - 1;
				} else {
					x = *_x0 +
					    FloorDiv(dx *
						     ((y - *_y0) * 2 + 1),
						     2 * dy) + 1;
				}
			} else {
				x = *_x0 + FloorDiv(dx * (y - *_y0) * 2 +
						    dy, 2 * dy);
			}
		}

		if (first != 0) {
			x0 = x;
			y0 = y;
			outcode(first, x0, y0);
#if 0
			*clip_first = 1;
#endif
		} else {
			x1 = x;
			y1 = y;
			last = code;
			outcode(last, x1, y1);
#if 0
			*clip_last = 1;
#endif
		}

		if ((first & last) != 0) {
			return 0;	/* Trivially rejected! */
		}
		if ((first | last) == 0) {
			*_x0 = x0;
			*_y0 = y0;
			*_x1 = x1;
			*_y1 = y1;
			return 1;	/* Trivially accepted! */
		}
	}
}

int GGI_tile_drawline(struct ggi_visual * vis, int _x, int _y, int _xe, int _ye)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
#define cliptl	(elm->origin)
#define clipbr	(elm->clipbr)
	int x, y, xe, ye;

	tile_FOREACH(priv, elm) {
		x = _x;
		y = _y;
		xe = _xe;
		ye = _ye;

		if (_ggi_clip2d(cliptl, clipbr, &x, &y, &xe, &ye)) {
			/* Clipped */
			ggiDrawLine(elm->vis,
				    x - cliptl.x, y - cliptl.y,
				    xe - cliptl.x, ye - cliptl.y);
		}
	}

	return 0;
}
