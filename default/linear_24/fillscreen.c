/* $Id: fillscreen.c,v 1.1 2001/05/12 23:01:40 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]

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

#include "lin24lib.h"


#warning FIXME: LIBGGI_FB_W_STRIDE(vis)%12 != 0 case not supported
int GGI_lin24_fillscreen(ggi_visual *vis)
{
  	if (LIBGGI_GC(vis)->cliptl.x==0 &&
	    LIBGGI_GC(vis)->cliptl.y==0 &&
	    LIBGGI_GC(vis)->clipbr.x==LIBGGI_VIRTX(vis) &&
	    LIBGGI_GC(vis)->clipbr.y==LIBGGI_VIRTY(vis)) {
		int col[3];
		uint32 *buf;
		int x,y;
		uint8 *colp = (uint8 *)col;

		PREPARE_FB(vis);
		buf = LIBGGI_CURWRITE(vis);

		for (x=0;x<4;x++) {
			*colp++=(LIBGGI_GC_FGCOLOR(vis))&0xFF;
			*colp++=(LIBGGI_GC_FGCOLOR(vis)>>8)&0xFF;
			*colp++=(LIBGGI_GC_FGCOLOR(vis)>>16)&0xFF;
		}
		for (y=0; y < LIBGGI_VIRTY(vis); y++) {
			for (x=0; x < LIBGGI_VIRTX(vis); x += 3) {
				*buf++=col[0];
				*buf++=col[1];
				*buf++=col[2];
			}
			buf = (uint32 *)((uint8 *)buf
					 + (LIBGGI_FB_W_STRIDE(vis)
					    - LIBGGI_VIRTX(vis)*4));
		}
	} else
		ggiDrawBox(vis,
			   LIBGGI_GC(vis)->cliptl.x, LIBGGI_GC(vis)->cliptl.y,
			   LIBGGI_GC(vis)->clipbr.x,LIBGGI_GC(vis)->clipbr.y);
	return 0;
}
