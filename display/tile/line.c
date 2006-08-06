/* $Id: line.c,v 1.12 2006/08/06 06:47:59 pekberg Exp $
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
#include "../../default/common/clip.c"

int GGI_tile_drawline(struct ggi_visual * vis, int _x, int _y, int _xe, int _ye)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	struct multi_vis *elm;
	int x, y, xe, ye;
	int first, last;

	tile_FOREACH(priv, elm) {
		/* This shift introduces clipping errors when the original
		 * variables are near INT_MIN.
		 */
		x = _x - elm->origin.x;
		y = _y - elm->origin.y;
		xe = _xe - elm->origin.x;
		ye = _ye - elm->origin.y;

		if (_ggi_clip2d(GGI_VISUAL(elm->vis), &x, &y, &xe, &ye, &first, &last)) {
			/* Clipped */
			ggiDrawLine(elm->vis, x, y, xe, ye);
		}
	}

	return 0;
}
