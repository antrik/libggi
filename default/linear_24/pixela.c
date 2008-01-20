/* $Id: pixela.c,v 1.4 2008/01/20 19:26:25 pekberg Exp $
******************************************************************************

   Graphics library for GGI. Pixels.

   Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]
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

#include "lin24lib.h"

/*******************************/
/* draw/get/put a single pixel */
/*******************************/

int GGI_lin24_drawpixela(struct ggi_visual *vis,int x,int y)
{
	uint8_t *adr;ggi_pixel col;

	CHECKXY(vis,x,y);
	PREPARE_FB(vis);
 
	adr=(uint8_t *)LIBGGI_CURWRITE(vis)+(y*LIBGGI_FB_R_STRIDE(vis)+x*3);
	col=LIBGGI_GC_FGCOLOR(vis);

	adr[0]=col;
	adr[1]=(col>>=8);
	adr[2]=col>>8;

	return 0;
}

int GGI_lin24_drawpixel_nca(struct ggi_visual *vis,int x,int y)
{
	uint8_t *adr;ggi_pixel col;
 
	PREPARE_FB(vis);

	adr=(uint8_t *)LIBGGI_CURWRITE(vis)+(y*LIBGGI_FB_R_STRIDE(vis)+x*3);
	col=LIBGGI_GC_FGCOLOR(vis);

	adr[0]=col;
	adr[1]=(col>>=8);
	adr[2]=col>>8;

	return 0;
}

int GGI_lin24_putpixel_nca(struct ggi_visual *vis,int x,int y,ggi_pixel col)
{ 
	uint8_t *adr;

	PREPARE_FB(vis);

	adr=(uint8_t *)LIBGGI_CURWRITE(vis)+(y*LIBGGI_FB_R_STRIDE(vis)+x*3);

	adr[0]=col;
	adr[1]=(col>>=8);
	adr[2]=(col>>8);

	return 0;
}

int GGI_lin24_putpixela(struct ggi_visual *vis,int x,int y,ggi_pixel col)
{ 
	uint8_t *adr;

	CHECKXY(vis,x,y);
	PREPARE_FB(vis);

	adr=(uint8_t *)LIBGGI_CURWRITE(vis)+(y*LIBGGI_FB_R_STRIDE(vis)+x*3);

	adr[0]=col;
	adr[1]=(col>>=8);
	adr[2]=(col>>8);

	return 0;
}

int
GGI_lin24_getpixel_nca(struct ggi_visual *vis,int x,int y,ggi_pixel *pixel)
{ 
	uint8_t *adr;

	PREPARE_FB(vis);

	adr=(uint8_t *)LIBGGI_CURREAD(vis)+(y*LIBGGI_FB_R_STRIDE(vis)+x*3);

	*pixel=adr[0]+(adr[1]<<8)+(adr[2]<<16);

	return 0;
}
