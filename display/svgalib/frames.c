/* $Id: frames.c,v 1.1 2001/06/24 16:45:12 skids Exp $
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

#include <ggi/display/svgalib.h>

int
GGI_svga_setreadframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	vis->r_frame_num = num;
	
	return 0;
}

int
GGI_svga_setwriteframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	vis->w_frame_num = num;
	
	return 0;
}

int
GGI_svga_setdisplayframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	vis->d_frame_num = num;
	
	return(ggiSetOrigin(vis, vis->origin_x, vis->origin_y));
}
