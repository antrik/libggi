/* $Id: gtext.c,v 1.1 2001/05/12 23:01:50 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Andrew Apted     [andrew@ggi-project.org]

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

#include "t16lib.h"


#define PIXEL_ADDR(vis, x, y)  \
        ((uint16 *) LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis)/sizeof(uint16) + x)

int GGI_t16_getcharsize(ggi_visual *vis, int *width, int *height)
{
	/* In text mode, the charsize is just 1x1 pixels */

	*width = *height = 1;
	return 0;
}

int GGI_t16_putc(ggi_visual *vis, int x, int y, char c)
{
	CHECKXY(vis, x, y);
	PREPARE_FB(vis);

	*PIXEL_ADDR(vis, x, y) = 
		((LIBGGI_GC_BGCOLOR(vis) & 0x0f00) << 4) |
		 (LIBGGI_GC_FGCOLOR(vis) & 0x0f00) | (c & 0xff);

	return 0;
}
