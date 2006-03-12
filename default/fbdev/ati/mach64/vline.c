/* $Id: vline.c,v 1.2 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   LibGGI - ATI Mach64 acceleration for fbdev target

   Copyright (C) 1999 Daniel Mantione	[daniel.mantione@freepascal.org]

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


int GGI_ati_mach64_drawvline(struct ggi_visual *vis, int x, int y, int h)
{
	struct ati_mach64_priv *priv = ATI_MACH64_PRIV(vis);
	int yadd = vis->w_frame_num * LIBGGI_VIRTY(vis);

	y += yadd;
	draw_rect(priv,x,y,1,h);
	vis->accelactive = 1;

	return 0;
}
