/* $Id: copybox.c,v 1.1 2001/05/12 23:01:37 cegger Exp $
******************************************************************************

   LibGGI - kgicon specific overrides for fbcon
   copybox

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <ggi/default/genkgi.h>


int GGI_genkgi_copybox(ggi_visual *vis, int x, int y, int w, int h,
			int x2, int y2)
{
	if (1) {
		struct kgi_cbox arg;
		arg.x = x;
		arg.y = y + vis->r_frame_num*LIBGGI_VIRTY(vis);
		arg.w = w;
		arg.h = h;
		arg.x2 = x2;
		arg.y2 = y2 + vis->w_frame_num*LIBGGI_VIRTY(vis);
		
		if (vis->opdisplay->kgicommand(vis, (int)ACCEL_COPYBOX, &arg) == 0) {
			return GGI_OK;
		}
		
		vis->opdraw->copybox = GENKGI_PRIV(vis)->copybox;
	}
	return GENKGI_PRIV(vis)->copybox(vis, x, y, w, h, x2, y2);
}
