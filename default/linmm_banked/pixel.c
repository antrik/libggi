/* $Id: pixel.c,v 1.2 2002/09/08 21:37:43 soyt Exp $
******************************************************************************

   Generic Banked 8,16,32Bpp MMAP Graphics library for GGI. Pixels.

   Copyright (C) 1997 Brian S. Julin   [bri@forcade.calyx.com]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]

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

#include "linmm_banked.h"

/*******************************/
/* draw/get/put a single pixel */
/*******************************/

int GGIdrawpixel(ggi_visual *vis,int x,int y)
{
        /* This already clips right */
	CHECKXY(vis,x,y);

	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	memcpy(BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP),
	       &LIBGGI_GC_FGCOLOR(vis), (1 << LOGBYTPP));

	return 0;
}

int GGIdrawpixel_nc(ggi_visual *vis,int x,int y)
{
	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	memcpy(BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP),
	       &LIBGGI_GC_FGCOLOR(vis), (1 << LOGBYTPP));

	return 0;
}

int GGIputpixel_nc(ggi_visual *vis,int x, int y, ggi_pixel color)
{ 
	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	memcpy(BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP),
	       &color, (1 << LOGBYTPP));

	return 0;
}

int GGIputpixel(ggi_visual *vis,int x, int y, ggi_pixel color)
{ 
	CHECKXY(vis,x,y);

	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	memcpy(BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP),
	       &color, (1 << LOGBYTPP));

	return 0;
}

int GGIgetpixel(ggi_visual *vis,int x,int y,ggi_pixel *pixel)
{ 
	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */
	*pixel = 0;
	memcpy(pixel, RBANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP),
	       (1 << LOGBYTPP));

	return 0;
	
}
