/* $Id: box.c,v 1.1 2002/10/05 18:39:06 fspacek Exp $
******************************************************************************

   ATI Mach64 box acceleration

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

******************************************************************************
*/

#include "mach64_accel.h"

int GGI_kgi_mach64_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	MACH64_CHECK(vis, 6);
	MACH64_WRITE(vis, MACH64_DST_CNTL);
	MACH64_WRITE(vis, 0x00000003);
	MACH64_WRITE(vis, MACH64_DST_X_Y);
	MACH64_WRITE(vis, ((x << MACH64_BA_DST_XShift) & MACH64_BA_DST_XMask) |
		((y << MACH64_BA_DST_YShift) & MACH64_BA_DST_YMask));
	MACH64_WRITE(vis, MACH64_DST_WIDTH_HEIGHT);
	MACH64_WRITE(vis, ((w << MACH64_BB_DST_WIDTHShift) & MACH64_BB_DST_WIDTHMask) |
		((h << MACH64_BB_DST_HEIGHTShift) & MACH64_BB_DST_HEIGHTMask));

	return 0;
}

int GGI_kgi_mach64_copybox(ggi_visual *vis, int x, int y, int w, int h,
			   int nx, int ny)
{
	uint32 direction = MACH64_4C_DST_LAST_PEL;
	
	if ((w < 1) || (h < 1))
		return 0;

	if (y < ny) {
		y += h-1; ny += h-1;
	} else
		direction |= MACH64_4C_DST_Y_DIR;
	
	if (x < nx) {
		x += w-1; nx += w-1;
	} else
		direction |= MACH64_4C_DST_X_DIR;
	

	MACH64_CHECK(vis, 12);
	MACH64_WRITE(vis, MACH64_DP_SRC);
	MACH64_WRITE(vis, 0x3 << MACH64_B6_DP_FRGD_SRCShift);
	MACH64_WRITE(vis, MACH64_DST_CNTL);
	MACH64_WRITE(vis, direction);
	MACH64_WRITE(vis, MACH64_SRC_Y_X);
	MACH64_WRITE(vis, (x << MACH64_63_SRC_XShift) | y);
	MACH64_WRITE(vis, MACH64_SRC_HEIGHT1_WIDTH1);
	MACH64_WRITE(vis, (w << MACH64_66_SRC_WIDTH1Shift) | h);
	MACH64_WRITE(vis, MACH64_DST_Y_X);
	MACH64_WRITE(vis, (nx << MACH64_43_DST_XShift) | ny);
	MACH64_WRITE(vis, MACH64_DST_HEIGHT_WIDTH);
	MACH64_WRITE(vis, (w << MACH64_46_DST_WIDTHShift) | h);
	
	return 0;
}