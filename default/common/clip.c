/* $Id: clip.c,v 1.10 2004/10/18 07:59:00 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Alexander Larsson   [alla@lysator.liu.se]
   Copyright (C) 2004 Peter Ekberg        [peda@lysator.liu.se]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/triple-int.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

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
  if ((xx)<(LIBGGI_GC(vis)->cliptl.x))\
    code |= OC_LEFT;\
  else if ((xx)>=(LIBGGI_GC(vis)->clipbr.x))\
    code |= OC_RIGHT;\
  if ((yy)<(LIBGGI_GC(vis)->cliptl.y))\
    code |= OC_TOP;\
  else if ((yy)>=(LIBGGI_GC(vis)->clipbr.y))\
    code |= OC_BOTTOM;\
}

/*
  Calculates _floor = |_ a/b _| with mathematically correct floor
  */
static void FloorDiv_3(unsigned _floor[3], unsigned a[3], unsigned b[3])
{
	unsigned neg_a[3], neg_b[3], r[3];
	if (gt0_3(b)) {
		if (gt0_3(a))
			divmod_3(a, b, _floor, r);
		else {
			assign_3(neg_a, a);
			negate_3(neg_a);
			divmod_3(neg_a, b, _floor, r);
			negate_3(_floor);
			if (!eq0_3(r))
				dec_3(_floor);
		}
	}
	else {
		if (gt0_3(a)) {
			assign_3(neg_b, b);
			negate_3(neg_b);
			divmod_3(a, neg_b, _floor, r);
			negate_3(_floor);
			if (!eq0_3(r))
				dec_3(_floor);
		}
		else
			/* don't bother to negate both a and b,
			 * divmod_3 does if for us.
			 */
			divmod_3(a, b, _floor, r);
	}
}

/*
  Calculates _ceil = |^ a/b ^| with mathematically correct ceil
  */
static void CeilDiv_3(unsigned _ceil[3], unsigned a[3], unsigned b[3])
{
	unsigned _a[3], _b[3];
	assign_3(_a, a);
	if (gt0_3(b)) {
		dec_3(_a);
		FloorDiv_3(_ceil, _a, b);
		inc_3(_ceil);
	} else {
		assign_3(_b, b);
		invert_3(_a); /* ~a == -a-1 */
		negate_3(_b);
		FloorDiv_3(_ceil, _a, _b);
		inc_3(_ceil);
	}
}

static int _ggi_clip2d_3(ggi_visual *vis,
			     int *_x0, int *_y0,
			     int *_x1, int *_y1,
			     int *clip_first, int *clip_last)
{
	int first,last, code;
	int x0,y0,x1,y1;
	int x,y;
	unsigned dx[3], dy[3], tmp[3];
	unsigned int absdx, absdy;
	int xmajor;
	int slope;
	int i;

	*clip_first = first = 0;
	*clip_last = last = 0;
	outcode(first,*_x0,*_y0);
	outcode(last,*_x1,*_y1);

	if ((first | last) == 0) {
		return 1; /* Trivially accepted! */
	}
	if ((first & last) != 0) {
		return 0; /* Trivially rejected! */
	}

	x0=*_x0; y0=*_y0;
	x1=*_x1; y1=*_y1;
	
	assign_int_3(dx, x1);
	assign_int_3(tmp, x0);
	sub_3(dx, tmp);

	assign_int_3(dy, y1);
	assign_int_3(tmp, y0);
	sub_3(dy, tmp);

	absdx = x0 < x1 ? x1 - x0 : x0 - x1;
	absdy = y0 < y1 ? y1 - y0 : y0 - y1;
	xmajor = absdx > absdy;
	slope = ((x1>=x0) && (y1>=y0)) || ((x1<x0) && (y1<y0));

	for (i = 0; i < 4; i++) {
		code = first;
		if (first==0)
			code = last;

		if (code&OC_LEFT) {
			x = LIBGGI_GC(vis)->cliptl.x;
			if (xmajor) {
				/* y = *_y0 + FloorDiv(dy*(x - *_x0)*2 + dx,
							2*dx); */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				mul_3(_x, dy);
				add_3(_x, dx);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _x, tmp);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			} else if (slope) {
				/* y = *_y0 + CeilDiv(dy*((x - *_x0)*2 - 1),
							2*dx); */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				dec_3(_x);
				mul_3(_x, dy);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				CeilDiv_3(res, _x, tmp);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			} else {
				/* y = *_y0 + FloorDiv(dy*((x - *_x0)*2 - 1),
							2*dx); */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				dec_3(_x);
				mul_3(_x, dy);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _x, tmp);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			}
		} else if (code&OC_RIGHT) {
			x = LIBGGI_GC(vis)->clipbr.x - 1;
			if (xmajor) {
				/* y = *_y0 + FloorDiv(dy*(x - *_x0)*2 + dx,
							2*dx); */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				mul_3(_x, dy);
				add_3(_x, dx);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _x, tmp);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			} else if (slope) {
				/* y = *_y0 + CeilDiv(dy*((x - *_x0)*2 + 1),
							2*dx)-1; */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				inc_3(_x);
				mul_3(_x, dy);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				CeilDiv_3(res, _x, tmp);
				dec_3(res);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			} else {
				/* y = *_y0 + FloorDiv(dy*((x - *_x0)*2 + 1),
							2*dx)+1; */
				unsigned _x[3], res[3];
				assign_int_3(_x, x);
				assign_int_3(tmp, *_x0);
				sub_3(_x, tmp);
				lshift_3(_x, 1);
				inc_3(_x);
				mul_3(_x, dy);
				assign_3(tmp, dx);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _x, tmp);
				inc_3(res);
				assign_int_3(tmp, *_y0);
				add_3(res, tmp);
				y = res[0];
			}
		} else if (code&OC_TOP) {
			y = LIBGGI_GC(vis)->cliptl.y;
			if (!xmajor) {
				/* x = *_x0 + FloorDiv(dx*(y - *_y0)*2 + dy,
							2*dy); */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				mul_3(_y, dx);
				add_3(_y, dy);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _y, tmp);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			} else if (slope) {
				/* x = *_x0 + CeilDiv(dx*((y - *_y0)*2 - 1),
							2*dy); */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				dec_3(_y);
				mul_3(_y, dx);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				CeilDiv_3(res, _y, tmp);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			} else {
				/* x = *_x0 + FloorDiv(dx*((y - *_y0)*2 - 1),
							2*dy); */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				dec_3(_y);
				mul_3(_y, dx);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _y, tmp);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			}
		} else { /* OC_BOTTOM */
			LIBGGI_ASSERT((code & OC_BOTTOM), "unknown outcode\n");
			y = LIBGGI_GC(vis)->clipbr.y - 1;
			if (!xmajor) {
				/* x = *_x0 + FloorDiv(dx*(y - *_y0)*2 + dy,
							2*dy); */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				mul_3(_y, dx);
				add_3(_y, dy);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _y, tmp);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			} else if (slope) {
				/* x = *_x0 + CeilDiv(dx*((y - *_y0)*2 + 1),
							2*dy)-1; */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				inc_3(_y);
				mul_3(_y, dx);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				CeilDiv_3(res, _y, tmp);
				dec_3(res);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			} else {
				/* x = *_x0 + FloorDiv(dx*((y - *_y0)*2 + 1),
							2*dy)+1; */
				unsigned _y[3], res[3];
				assign_int_3(_y, y);
				assign_int_3(tmp, *_y0);
				sub_3(_y, tmp);
				lshift_3(_y, 1);
				inc_3(_y);
				mul_3(_y, dx);
				assign_3(tmp, dy);
				lshift_3(tmp, 1);
				FloorDiv_3(res, _y, tmp);
				inc_3(res);
				assign_int_3(tmp, *_x0);
				add_3(res, tmp);
				x = res[0];
			}
		}

		if (first!=0) {
			x0 = x;
			y0 = y;
			outcode(first,x0,y0);
			*clip_first = 1;
		} else {
			x1 = x;
			y1 = y;
			last = code;
			outcode(last,x1,y1);
			*clip_last = 1;
		}

		if ((first & last) != 0) {
			return 0; /* Trivially rejected! */
		}
		if ((first | last) == 0) {
			*_x0=x0; *_y0=y0;
			*_x1=x1; *_y1=y1;
			return 1; /* Trivially accepted! */
		}
	}
	return 0; /* Aieee! Failed to clip, clip whole line... */
}

/*
  Calculates |_ a/b _| with mathematically correct floor
  */
static int FloorDiv(int a, int b)
{
	int _floor;
	if (b>0) {
		if (a>0) {
			return a /b;
		} else {
			_floor = -((-a)/b);
			if ((-a)%b != 0)
				_floor--;
		}
		return _floor;
	} else {
		if (a>0) {
			_floor = -(a/(-b));
			if (a%(-b) != 0)
				_floor--;
			return _floor;
		} else {
			return (-a)/(-b);
		}
	}
}

/*
  Calculates |^ a/b ^| with mathematically correct ceil
  */
static int CeilDiv(int a,int b)
{
	if (b>0)
		return FloorDiv(a-1,b)+1;
	else
		return FloorDiv(-a-1,-b)+1;
}

#define MAX_DIFF (INT_MAX >> (sizeof(int)*4))

static int _ggi_clip2d(ggi_visual *vis,int *_x0, int *_y0, int *_x1, int *_y1,
		       int *clip_first, int *clip_last)
{
	int first,last, code;
	int x0,y0,x1,y1;
	int x,y;
	int dx,dy;
	unsigned int absdx, absdy;
	int xmajor;
	int slope;
	int i;

	*clip_first = first = 0;
	*clip_last = last = 0;
	outcode(first,*_x0,*_y0);
	outcode(last,*_x1,*_y1);

	if ((first | last) == 0) {
		return 1; /* Trivially accepted! */
	}
	if ((first & last) != 0) {
		return 0; /* Trivially rejected! */
	}

	x0=*_x0; y0=*_y0;
	x1=*_x1; y1=*_y1;

	dx = x1 - x0;
	dy = y1 - y0;

	absdx = x0 < x1 ? x1 - x0 : x0 - x1;
	absdy = y0 < y1 ? y1 - y0 : y0 - y1;
	xmajor = absdx > absdy;
	slope = ((x1>=x0) && (y1>=y0)) || ((x1<x0) && (y1<y0));

	if ((absdx > MAX_DIFF) || (absdy > MAX_DIFF)) {
		return _ggi_clip2d_3(vis, _x0, _y0, _x1, _y1,
			clip_first, clip_last);
	}

	for (i = 0; i < 4; i++) {
		code = first;
		if (first==0)
			code = last;

		if (code&OC_LEFT) {
			x = LIBGGI_GC(vis)->cliptl.x;
			if (xmajor) {
				y = *_y0 +  FloorDiv(dy*(x - *_x0)*2 + dx,
						      2*dx);
			} else {
				if (slope) {
					y = *_y0 + CeilDiv(dy*((x - *_x0)*2
							       - 1), 2*dx);
				} else {
					y = *_y0 + FloorDiv(dy*((x - *_x0)*2
								- 1), 2*dx);
				}
			}
		} else if (code&OC_RIGHT) {
			x = LIBGGI_GC(vis)->clipbr.x - 1;
			if (xmajor) {
				y = *_y0 +  FloorDiv(dy*(x - *_x0)*2 + dx, 2*dx);
			} else {
				if (slope) {
					y = *_y0 + CeilDiv(dy*((x - *_x0)*2
							       + 1), 2*dx)-1;
				} else {
					y = *_y0 + FloorDiv(dy*((x - *_x0)*2
								+ 1), 2*dx)+1;
				}
			}
		} else if (code&OC_TOP) {
			y = LIBGGI_GC(vis)->cliptl.y;
			if (xmajor) {
				if (slope) {
					x = *_x0 + CeilDiv(dx*((y - *_y0)*2
							       - 1), 2*dy);
				} else {
					x = *_x0 + FloorDiv(dx*((y - *_y0)*2
								- 1), 2*dy);
				}
			} else {
				x = *_x0 +  FloorDiv( dx*(y - *_y0)*2 + dy,
						      2*dy);
			}
		} else { /* OC_BOTTOM */
			LIBGGI_ASSERT((code & OC_BOTTOM), "unknown outcode\n");
			y = LIBGGI_GC(vis)->clipbr.y - 1;
			if (xmajor) {
				if (slope) {
					x = *_x0 + CeilDiv(dx*((y - *_y0)*2
							       + 1), 2*dy)-1;
				} else {
					x = *_x0 + FloorDiv(dx*((y - *_y0)*2
								+ 1), 2*dy)+1;
				}
			} else {
				x = *_x0 +  FloorDiv(dx*(y - *_y0)*2 + dy,
						     2*dy);
			}
		}

		if (first!=0) {
			x0 = x;
			y0 = y;
			outcode(first,x0,y0);
			*clip_first = 1;
		} else {
			x1 = x;
			y1 = y;
			last = code;
			outcode(last,x1,y1);
			*clip_last = 1;
		}

		if ((first & last) != 0) {
			return 0; /* Trivially rejected! */
		}
		if ((first | last) == 0) {
			*_x0=x0; *_y0=y0;
			*_x1=x1; *_y1=y1;
			return 1; /* Trivially accepted! */
		}
	}
	return 0; /* Aieee! Failed to clip, clip whole line... */
}
