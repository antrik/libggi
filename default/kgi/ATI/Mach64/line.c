/* $Id: line.c,v 1.3 2006/03/12 23:15:06 soyt Exp $
******************************************************************************

   ATI Mach64 line acceleration

   Copyright (C) 2002 Paul Redmond

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

*******************************************************************************/

#include "mach64_accel.h"

int GGI_kgi_mach64_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	DPRINT_LIBS("drawhline\n");
	GGI_kgi_mach64_drawbox(vis, x, y, w, 1);

	return 0;
}

int GGI_kgi_mach64_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
        DPRINT_LIBS("drawvline\n");
	GGI_kgi_mach64_drawbox(vis, x, y, 1, h);

	return 0;
}

int GGI_kgi_mach64_drawline(struct ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	short dx, dy;
	long minDelta, maxDelta;
	short x_dir, y_dir, y_major;

	dx = abs(x2 - x1);
	dy = abs(y2 - y1);
	minDelta = dx < dy ? dx : dy;
	maxDelta = dx > dy ? dx : dy;

	/* Determine the octant. */
	if (x1 < x2)
		x_dir = 1;
	else
		x_dir = 0;
	if (y1 < y2)
		y_dir = 0x0802;
	else 
		y_dir = 0;
		
	/* Use top/bottom for Bresenham zero sign convention */
	
	if (dx < dy)
		y_major = 4;
	else
		y_major = 0; 
		
	MACH64_CHECK(vis, 12);
	MACH64_WRITE(vis, MACH64_DST_CNTL);
	MACH64_WRITE(vis,(unsigned int)(y_major | y_dir | x_dir));
	MACH64_WRITE(vis, MACH64_DST_Y_X);
	MACH64_WRITE(vis, ((unsigned int)x1 << 16) | y1);
	MACH64_WRITE(vis, MACH64_DST_BRES_ERR);
	MACH64_WRITE(vis, 2 * minDelta - maxDelta);
	MACH64_WRITE(vis, MACH64_DST_BRES_INC);
	MACH64_WRITE(vis, 2 * minDelta);
	MACH64_WRITE(vis, MACH64_DST_BRES_DEC);
	MACH64_WRITE(vis, 2 * (minDelta - maxDelta));
	MACH64_WRITE(vis, MACH64_DST_BRES_LNTH);
	MACH64_WRITE(vis, maxDelta + 1);
	return 0;
}
