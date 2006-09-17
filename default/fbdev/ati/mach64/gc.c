/* $Id: gc.c,v 1.4 2006/09/17 12:21:50 cegger Exp $
******************************************************************************

   LibGGI - Mach 64 / Rage Pro acceleration for fbdev target

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


void GGI_ati_mach64_gcchanged(struct ggi_visual *vis, int mask)
{
	struct ggi_gc *gc;
	struct ati_mach64_priv *priv;

	priv = ATI_MACH64_PRIV(vis);
	gc = LIBGGI_GC(vis);

	/* Set new foreground colour if it's changed */
	if (gc->fg_color != priv->oldfgcol) {
		wait_for_fifo(1, priv);
		aty_st_le32(DP_FRGD_CLR, (unsigned) gc->fg_color, priv);
		priv->oldfgcol = gc->fg_color;
	};
	/* Set new background colour if it's changed */
	if (gc->bg_color != priv->oldbgcol) {
		wait_for_fifo(1, priv);
		aty_st_le32(DP_BKGD_CLR, (unsigned) gc->bg_color, priv);
		priv->oldbgcol = gc->bg_color;
	};
	/* Set new horizontal clipping if it's changed */
	if ((gc->cliptl.x != priv->oldtl.x)
	    || (gc->clipbr.x != priv->oldbr.x)) {
		wait_for_fifo(1, priv);
		aty_st_le32(SC_LEFT_RIGHT,
			    (unsigned)gc->clipbr.x << 16 | gc->cliptl.x,
			    priv);
		priv->oldtl.x = gc->cliptl.x;
		priv->oldbr.x = gc->clipbr.x;
	};
	/* Set new vertical clipping if it's changed */
	if ((gc->cliptl.y != priv->oldtl.y)
	    || (gc->clipbr.y != priv->oldbr.y)) {
		wait_for_fifo(1, priv);
		aty_st_le32(SC_TOP_BOTTOM,
			    (unsigned)gc->clipbr.y << 16 | gc->cliptl.y, 
			    priv);
		priv->oldtl.y = gc->cliptl.y;
		priv->oldbr.y = gc->clipbr.y;
	};
	vis->accelactive = 1;
}
