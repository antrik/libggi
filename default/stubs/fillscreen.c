/* $Id: fillscreen.c,v 1.3 2006/03/14 18:15:30 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]

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

#include "stublib.h"

/*************************************/
/* fill (erase) the (virtual) screen */
/*************************************/

int GGI_stubs_fillscreen(struct ggi_visual *vis)
{
	/* Clipping is simple here */
	return ggiDrawBox(vis->stem,
		LIBGGI_GC(vis)->cliptl.x,
		LIBGGI_GC(vis)->cliptl.y,
		LIBGGI_GC(vis)->clipbr.x-LIBGGI_GC(vis)->cliptl.x,
		LIBGGI_GC(vis)->clipbr.y-LIBGGI_GC(vis)->cliptl.y);
}

