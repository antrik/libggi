/* $Id: rect.h,v 1.1 2006/08/27 11:45:17 pekberg Exp $
******************************************************************************

   display-vnc: rectangles

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#ifndef _GGI_RECT_H
#define _GGI_RECT_H

#include <ggi/types.h>
/* need display/vnc.h for ggi_rect typedef, not so pretty... */
#include <ggi/display/vnc.h>

static inline int
ggi_rect_isempty(ggi_rect *rect)
{
	return rect->br.x <= rect->tl.x || rect->br.y <= rect->tl.y;
}

static inline void
ggi_rect_set_xyxy(ggi_rect *rect,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	rect->tl.x = tlx;
	rect->tl.y = tly;
	rect->br.x = brx;
	rect->br.y = bry;
}

static inline void
ggi_rect_union_xyxy_ne(ggi_rect *rect,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	if (tlx <= rect->tl.x)
		rect->tl.x = tlx;
	if (rect->br.x < brx)
		rect->br.x = brx;
	if (tly <= rect->tl.y)
		rect->tl.y = tly;
	if (rect->br.y <= bry)
		rect->br.y = bry;
}

static inline void
ggi_rect_union_xyxy(ggi_rect *rect,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry)
{
	if (ggi_rect_isempty(rect))
		ggi_rect_set_xyxy(rect, tlx, tly, brx, bry);
	else
		ggi_rect_union_xyxy_ne(rect, tlx, tly, brx, bry);
}

static inline void
ggi_rect_union_ne(ggi_rect *r1, ggi_rect *r2)
{
	ggi_rect_union_xyxy_ne(r1, r2->tl.x, r2->tl.y, r2->br.x, r2->br.y);
}

static inline void
ggi_rect_union(ggi_rect *r1, ggi_rect *r2)
{
	if (ggi_rect_isempty(r1))
		*r1 = *r2;
	else
		ggi_rect_union_ne(r1, r2);
}

static inline void
ggi_rect_intersect(ggi_rect *r1, ggi_rect *r2)
{
	if (ggi_rect_isempty(r1))
		return;
	if (ggi_rect_isempty(r2)) {
		r1->tl.x = r1->br.x = 0;
		return;
	}

	if (r1->tl.x < r2->tl.x)
		r1->tl.x = r2->tl.x;
	if (r2->br.x < r1->br.x)
		r1->br.x = r2->br.x;
	if (r1->tl.y < r2->tl.y)
		r1->tl.y = r2->tl.y;
	if (r2->br.y < r1->br.y)
		r1->br.y = r2->br.y;
}

static inline void
ggi_rect_subtract(ggi_rect *r1, ggi_rect *r2)
{
	ggi_rect intersection;

	if (ggi_rect_isempty(r1))
		return;
	if (ggi_rect_isempty(r2))
		return;

	intersection = *r1;
	ggi_rect_intersect(&intersection, r2);

	if (ggi_rect_isempty(&intersection))
		return;

	if (r1->tl.x == intersection.tl.x && r1->tl.y == intersection.tl.y) {
		/* top left corner fits */
		if (r1->br.x == intersection.br.x) {
			/* top edge fits */
			r1->tl.y = intersection.br.y;
			return;
		}
		if (r1->br.y == intersection.br.y) {
			/* left edge fits */
			r1->tl.x = intersection.br.x;
			return;
		}
		return;
	}
	if (r1->br.x == intersection.br.x && r1->br.y == intersection.br.y) {
		/* bottom right corner fits */
		if (r1->tl.x == intersection.tl.x) {
			/* bottom edge fits */
			r1->br.y = intersection.tl.y;
			return;
		}
		if (r1->tl.y == intersection.tl.y) {
			/* right edge fits */
			r1->br.x = intersection.tl.x;
			return;
		}
	}
}

#endif /* _GGI_RECT_H */
