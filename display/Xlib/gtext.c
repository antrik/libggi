/* $Id: gtext.c,v 1.2 2002/09/08 21:37:45 soyt Exp $
******************************************************************************

   Graphics library for GGI. Textfunctions for Xlib.

   Copyright (C) 1998 Marcus Sundberg [marcus@ggi-project.org]

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
#include <ggi/display/xlib.h>

int GGI_Xlib_getcharsize(ggi_visual *vis, int *width, int *height)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	*width = priv->textfont->max_bounds.width;
	*height = priv->textfont->max_bounds.ascent
		+ priv->textfont->max_bounds.descent;

	return 0;
}

int GGI_Xlib_putc(ggi_visual *vis, int x, int y, char c)
{
	ggi_xlib_priv *priv = LIBGGI_PRIVATE(vis);

	XDrawString(priv->xwin.x.display, priv->xwin.window,
		    priv->xwin.x.gc,
		    x, y+priv->textfont->max_bounds.ascent, &c, 1);
	
	XLIB_DOSYNC(vis);
	return 0;
}


