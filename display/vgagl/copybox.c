/* $Id: copybox.c,v 1.3 2005/07/30 11:38:51 cegger Exp $
******************************************************************************

   SVGAlib target vgagl helper: copybox.

   Copyright (C) 1998 Marcus Sundberg   [marcus@ggi-project.org]
   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>

int GGI_vgagl_copybox(ggi_visual *vis,int x,int y,int w,int h,int nx,int ny)
{
	int line;

	/* Clipping */
	if (nx< (LIBGGI_GC(vis)->cliptl.x)) {
		int diff=(LIBGGI_GC(vis)->cliptl.x)-nx;
		nx+=diff;
		x +=diff;
		w -=diff;
	}
	if (nx+w>=(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-nx;
	}
	if (ny< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-ny;
		ny+=diff;
		y +=diff;
		h -=diff;
	}
	if (ny+h>(LIBGGI_GC(vis)->clipbr.y)) {
		h=(LIBGGI_GC(vis)->clipbr.y)-ny;
	}

	/* If nothing to copy or the source leaves the virtual screen, bail out. */
	if (w<=0||h<=0||x<0||y<0||x+w>LIBGGI_VIRTX(vis)||y+h>LIBGGI_VIRTY(vis))
		return 0;
	
	if (ny < y) {
		obox=(uint8_t *)(LIBGGI_CURREAD(vis))+y*LIBGGI_FB_R_STRIDE(vis)+x;
		nbox=(uint8_t *)(LIBGGI_CURWRITE(vis))+ny*LIBGGI_FB_W_STRIDE(vis)+nx;
		for (line=0; line<h; line++,obox+=LIBGGI_FB_R_STRIDE(vis),
			     nbox+=LIBGGI_FB_W_STRIDE(vis))
			memmove(nbox,obox,w);
	} else {
		obox=(uint8_t *)(LIBGGI_CURREAD(vis))+(y+h-1)*LIBGGI_FB_R_STRIDE(vis)+x;
		nbox=(uint8_t *)(LIBGGI_CURWRITE(vis))+(ny+h-1)*LIBGGI_FB_W_STRIDE(vis)+nx;
		for (line=0;line<h; line++,obox-=LIBGGI_FB_R_STRIDE(vis),
			     nbox-=LIBGGI_FB_W_STRIDE(vis))
			memmove(nbox,obox,w);
	}
	return 0;
}
