/* $Id: gc.c,v 1.1 2002/10/05 18:39:06 fspacek Exp $
******************************************************************************

   ATI Mach64 gc acceleration

   Copyright (C) 2002 Filip Spacek

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

#include "mach64_accel.h"

void GGI_kgi_mach64_gcchanged(ggi_visual *vis, int mask)
{
	if(mask & GGI_GCCHANGED_FG){
		MACH64_CHECK(vis, 2);
		MACH64_WRITE(vis, MACH64_DP_FRGD_CLR);
		MACH64_WRITE(vis, LIBGGI_GC_FGCOLOR(vis));
	}
	if(mask & GGI_GCCHANGED_BG){
		MACH64_CHECK(vis, 2);
		MACH64_WRITE(vis, MACH64_DP_BKGD_CLR);
		MACH64_WRITE(vis, LIBGGI_GC_BGCOLOR(vis));
	}
}
