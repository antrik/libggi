/* $Id: box.c,v 1.1 2001/05/12 23:01:37 cegger Exp $
******************************************************************************

   LibGGI - kgicon specific overrides for fbcon
   drawbox, fillscreen

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 1999 Andreas Beck	[becka@ggi-project.org]

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

int GGI_genkgi_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	GGIDPRINT_DRAW("GGI_genkgi_drawbox() called\n");
	if (1) 
	{
		struct kgi_box arg;
		arg.x = x;
		arg.y = y + vis->w_frame_num*LIBGGI_VIRTY(vis);
		arg.w = w;
		arg.h = h;
		
		if (vis->opdisplay->kgicommand(vis, (int)ACCEL_DRAWBOX, &arg) == 0) {
			GGIDPRINT_DRAW("GGI_genkgi_drawbox() success!\n");
			return GGI_OK;
		}
		
		vis->opdraw->drawbox = GENKGI_PRIV(vis)->drawbox;
	}

	return GENKGI_PRIV(vis)->drawbox(vis, x, y, w, h);
}


int GGI_genkgi_fillscreen(ggi_visual *vis)
{
	if (vis->opdisplay->kgicommand(vis, (int)ACCEL_FILLSCREEN, NULL) != 0) { 
		vis->opdraw->fillscreen = GENKGI_PRIV(vis)->fillscreen;
		return vis->opdraw->fillscreen(vis);
	}

	return GGI_OK;
}
