/* $Id: pixel.c,v 1.1 2001/05/12 23:01:45 cegger Exp $
******************************************************************************

   Graphics library for GGI. Pixels.

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

#include "lin4lib.h"

/*******************************/
/* draw/get/put a single pixel */
/*******************************/

int GGI_lin4_drawpixel(ggi_visual *vis,int x,int y)
{
	uint8 pel;
/*	uint8 clr;*/
	uint8 *fb;
	uint8 xs;
	
	CHECKXY(vis,x,y);
 	
	/* Read-modify-write. */
	
	fb = (uint8 *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>1);
	pel = *fb;

	/* This does:
	 
	 * if (x & 1)
	 *    clr = (pel & 0xF0) | (LIBGGI_GC_FGCOLOR(vis) & 0x0f);
	 * else clr = (pel & 0x0F) | ((LIBGGI_GC_FGCOLOR(vis) & 0x0f) << 4);
	 */
	xs = (x & 1) << 2;
	*fb = (pel & (0x0F << xs)) | ((LIBGGI_GC_FGCOLOR(vis) & 0x0f) << (xs ^ 4));

	return 0;
}

int GGI_lin4_drawpixel_nc(ggi_visual *vis,int x,int y)
{
	uint8 pel;
/*	uint8 clr;*/
	uint8 *fb;
	uint8 xs;
	
	/* Read-modify-write. */
	
	fb = (uint8 *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>1);
	pel = *fb;

	/* This does:
	 
	 * if (x & 1)
	 *    clr = (pel & 0xF0) | (LIBGGI_GC_FGCOLOR(vis) & 0x0f);
	 * else clr = (pel & 0x0F) | ((LIBGGI_GC_FGCOLOR(vis) & 0x0f) << 4);
	 */
	xs = (x & 1) << 2;
	*fb = (pel & (0x0F << xs)) | ((LIBGGI_GC_FGCOLOR(vis) & 0x0f) << (xs ^ 4));

	return 0;
}

int GGI_lin4_putpixel_nc(ggi_visual *vis,int x,int y,ggi_pixel col)
{ 
	uint8 pel;
	uint8 *fb;
	uint8 xs;
	
	fb = (uint8 *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>1);
	pel = *fb;

	xs = (x & 1) << 2;
	
	*fb=(pel & (0x0F << xs)) | ((col & 0x0f) << (xs ^ 4));

	return 0;
}

int GGI_lin4_putpixel(ggi_visual *vis,int x,int y,ggi_pixel col)
{ 
	uint8 pel;
	uint8 *fb;
	uint8 xs;
	
	CHECKXY(vis,x,y);
 	
	fb = (uint8 *)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x>>1);
	pel = *fb;

	xs = (x & 1) << 2;
	
	*fb=(pel & (0x0F << xs)) | ((col & 0x0f) << (xs ^ 4));

	return 0;
}

int GGI_lin4_getpixel(ggi_visual *vis,int x,int y,ggi_pixel *pixel)
{ 
	int pel;
	uint8 xs;
	
	pel = *(uint8 *)LIBGGI_CURREAD(vis)+y*LIBGGI_FB_R_STRIDE(vis)+(x>>1);

	xs = (x & 1) << 2;
	*pixel = (ggi_pixel)((pel & (0x0f << xs)) >> (xs ^ 4)); 

	return 0;
}

