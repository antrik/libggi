/* $Id: hline.c,v 1.1 2001/05/12 23:01:46 cegger Exp $
******************************************************************************

   Graphics library for GGI. Horizontal lines.

   Copyright (C) 1995 Andreas Beck [becka@ggi-project.org]

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

#include "lin8lib.h"


int GGI_lin8_drawhline(ggi_visual *vis,int x,int y,int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);
	PREPARE_FB(vis);

	memset((uint8*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x,
	       LIBGGI_GC_FGCOLOR(vis), w);

	return 0;
}

int GGI_lin8_drawhline_nc(ggi_visual *vis,int x,int y,int w)
{
	PREPARE_FB(vis);

        memset((uint8*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x,
	       LIBGGI_GC_FGCOLOR(vis), w);

	return 0;
}

int GGI_lin8_puthline(ggi_visual *vis,int x,int y,int w,void *buffer)
{
	uint8 *buf8 = buffer, *mem;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buf8, );
	PREPARE_FB(vis);

	mem = (uint8*)LIBGGI_CURWRITE(vis) + y*LIBGGI_FB_W_STRIDE(vis) + x;
	memcpy(mem, buf8, w);

	return 0;
}

int GGI_lin8_gethline(ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8 *mem;

	PREPARE_FB(vis);

	mem = (uint8 *)LIBGGI_CURREAD(vis) + y*LIBGGI_FB_R_STRIDE(vis) + x;

	memcpy(buffer, mem, w);

	return 0;
}
