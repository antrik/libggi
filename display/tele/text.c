/* $Id: text.c,v 1.1 2001/05/12 23:02:27 cegger Exp $
******************************************************************************

   TELE target.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/font/8x8>

#include "libtele.h"
#include <ggi/display/tele.h>


int GGI_tele_getcharsize(ggi_visual *vis, int *width, int *height)
{
	*width = *height = 8;
	return 0;
}

int GGI_tele_putc(ggi_visual *vis, int x, int y, char c)
{
	/* !!! use CMD_PUTSTR */

	uint8 pixbuf[8*8];

	int cx, cy;

	uint8 *ch_data = & font[((int) c & 0xff) << 3];

	for (cy=0; cy < 8; cy++)
	for (cx=0; cx < 8; cx++) {
		
		pixbuf[(cy << 3) + cx] = (ch_data[cy] & (1 << (7-cx))) ?
			LIBGGI_GC_FGCOLOR(vis) : LIBGGI_GC_BGCOLOR(vis);
	}

	return ggiPutBox(vis, x, y, 8, 8, pixbuf);
}

/* !!! Implement GGIputs() with CMD_PUTSTR */

