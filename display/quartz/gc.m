/* $Id: gc.m,v 1.2 2004/10/31 14:25:03 cegger Exp $
******************************************************************************

   Display quartz : color management

   Copyright (C) 2002 Christoph Egger

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
#include <string.h>

#include <ggi/display/quartz.h>
#include <ggi/internal/ggi_debug.h>


void GGI_quartz_gcchanged(ggi_visual *vis, int mask)
{
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

#if 0
	if ((mask & GGI_GCCHANGED_CLIP)) {
		ggiSetGCClipping(vis,
				LIBGGI_GC(vis)->cliptl.x,
				LIBGGI_GC(vis)->cliptl.y,
				LIBGGI_GC(vis)->clipbr.x,
				LIBGGI_GC(vis)->clipbr.y);
	}	/* if */

	if ((mask & GGI_GCCHANGED_FG)) {
		ggiSetGCForeground(vis, LIBGGI_GC_FGCOLOR(vis));
	}	/* if */
	if ((mask & GGI_GCCHANGED_BG)) {
		ggiSetGCBackground(vis, LIBGGI_GC_BGCOLOR(vis));
	}	/* if */
#endif

}	/* GGI_quartz_gcchanged */
