/* $Id: fb.c,v 1.2 2002/09/08 21:37:45 soyt Exp $
******************************************************************************

   LibGGI GLIDE target - Framebuffer handling

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/glide.h>


int
GGI_glide_setdisplayframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	if (num != vis->d_frame_num) {
		vis->d_frame_num = num;
		grBufferSwap(1);
	}
	if (num == vis->r_frame_num) {
		GLIDE_PRIV(vis)->readbuf = GR_BUFFER_FRONTBUFFER;
	} else {
		GLIDE_PRIV(vis)->readbuf = GR_BUFFER_BACKBUFFER;
	}
	if (num == vis->w_frame_num) {
		GLIDE_PRIV(vis)->writebuf = GR_BUFFER_FRONTBUFFER;
		grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	} else {
		GLIDE_PRIV(vis)->writebuf = GR_BUFFER_BACKBUFFER;
		grRenderBuffer(GR_BUFFER_BACKBUFFER);
	}
	
	return 0;
}

int
GGI_glide_setreadframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	if (num == vis->d_frame_num) {
		GLIDE_PRIV(vis)->readbuf = GR_BUFFER_FRONTBUFFER;
	} else {
		GLIDE_PRIV(vis)->readbuf = GR_BUFFER_BACKBUFFER;
	}
	vis->r_frame_num = num;
	
	return 0;
}

int
GGI_glide_setwriteframe(ggi_visual *vis, int num)
{
	if (num < 0 || num >= LIBGGI_MODE(vis)->frames) {
		return -1;
	}
	if (num == vis->d_frame_num) {
		GLIDE_PRIV(vis)->writebuf = GR_BUFFER_FRONTBUFFER;
		grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	} else {
		GLIDE_PRIV(vis)->writebuf = GR_BUFFER_BACKBUFFER;
		grRenderBuffer(GR_BUFFER_BACKBUFFER);
	}
	vis->w_frame_num = num;
	
	return 0;
}
