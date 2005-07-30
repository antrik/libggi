/* $Id: copybox.c,v 1.3 2005/07/30 11:39:57 cegger Exp $
******************************************************************************

   LibGGI - ATI Mach64 acceleration for fbdev target

   Copyright (C) 2002 Daniel Mantione	[daniel.mantione@freepascal.org]

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

#include "ati_mach64.h"


int GGI_ati_mach64_copybox(ggi_visual *vis, int x, int y, int w, int h,
		       int dstx, int dsty)
{
	if (w > 0 && h > 0) {	/* 0 width is not OK! */
		struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
		uint32_t direction = DST_LAST_PEL;
    
		y += vis->r_frame_num * LIBGGI_VIRTY(vis);
		dsty += vis->w_frame_num * LIBGGI_VIRTY(vis);
	        if (y < dsty) {
    		    dsty += h - 1;
    		    y += h - 1;
		} else
    		    direction |= DST_Y_TOP_TO_BOTTOM;

		if (x < dstx) {
    		    dstx += w - 1;
    		    x += w - 1;
		} else
    		    direction |= DST_X_LEFT_TO_RIGHT;

		set_dp_src(priv,FRGD_SRC_BLIT);
		set_dst_cntl(priv,direction);
		wait_for_fifo(4,priv);
		aty_st_le32(SRC_Y_X, (unsigned)(x << 16) | y, priv);
		aty_st_le32(SRC_HEIGHT1_WIDTH1, (unsigned)(w << 16) | h, priv);
		aty_st_le32(DST_Y_X, (unsigned)(dstx << 16) | dsty, priv);
    		aty_st_le32(DST_HEIGHT_WIDTH, (unsigned)(w << 16) | h, priv);

		vis->accelactive = 1;
	}
	return 0;
}
