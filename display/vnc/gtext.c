/* $Id: gtext.c,v 1.4 2007/03/11 00:48:59 soyt Exp $
******************************************************************************

   display-vnc: text

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#include <ggi/display/vnc.h>
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>

int
GGI_vnc_putc(struct ggi_visual *vis, int x, int y, char c)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int char_w, char_h;

	int res = _ggiPutc(priv->fb, x, y, c);
	_ggiGetCharSize(priv->fb, &char_w, &char_h);
	GGI_vnc_invalidate_xyxy(vis, x, y, x + char_w, y + char_h);

	return res;
}

int
GGI_vnc_puts(struct ggi_visual *vis, int x, int y, const char *str)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);
	int count;
	int char_w, char_h;

	_ggiGetCharSize(priv->fb, &char_w, &char_h);
	
	/* vertically out of the clipping area ? */

	if ((y+char_h < LIBGGI_GC(vis)->cliptl.y) ||
	    (y >= LIBGGI_GC(vis)->clipbr.y)) {

		return 0;
	}
	    
	for (count=0; *str && (x < LIBGGI_VIRTX(vis));
	     str++, x += char_w) {

		/* horizontally within the clipping area ? */

		if ((x+char_w >= LIBGGI_GC(vis)->cliptl.x) &&
		    (x < LIBGGI_GC(vis)->clipbr.x)) {
			ggiPutc(vis->instance.stem, x, y, *str); 
			count++;
		}
	}

	return count;
}

int
GGI_vnc_getcharsize(struct ggi_visual *vis, int *width, int *height)
{
	ggi_vnc_priv *priv = VNC_PRIV(vis);

	return _ggiGetCharSize(priv->fb, width, height);
}
