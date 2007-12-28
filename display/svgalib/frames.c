/* $Id: frames.c,v 1.5 2007/12/28 15:48:57 cegger Exp $
******************************************************************************

   SVGA target: frame handling functions

   Copyright (C) 2001 Brian S. Julin   [skids@ggi-project.org]

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
#include <ggi/display/svgalib.h>

int
GGI_svga_setreadframe(struct ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return GGI_ENOSPACE;
	}
	vis->r_frame_num = num;
	
	return 0;
}

int
GGI_svga_setwriteframe(struct ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return GGI_ENOSPACE;
	}
	vis->w_frame_num = num;
	
	return 0;
}

int
GGI_svga_setdisplayframe(struct ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return GGI_ENOSPACE;
	}
	vis->d_frame_num = num;
	
	return (_ggiSetOrigin(vis, vis->origin_x, vis->origin_y));
}
