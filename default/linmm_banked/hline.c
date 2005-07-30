/* $Id: hline.c,v 1.4 2005/07/30 11:40:02 cegger Exp $
******************************************************************************

   Generic 8, 16, 32 Banked Graphics library for GGI. Horizontal lines.

   Copyright (C) 1998 Brian S. Julin   [bri@forcade.calyx.com]
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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include "linmm_banked.h"

/**********************************/
/* draw/get/put a horizontal line */
/**********************************/

int GGIdrawhline(ggi_visual *vis,int x,int y,int w)
{
	ggi_pixel color;
	unsigned int align;
	uint8_t *pixpt;

	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) 
	        return 0;
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		int diff=(LIBGGI_GC(vis)->cliptl.x)-x;
		x+=diff;
		w-=diff;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	CHECKXYW(vis,x,y,w);

	color = LIBGGI_GC_FGCOLOR(vis);

	switch(LOGBYTPP) {
	case 0: 
	  color &= 0xff;
	  color |= color << 8;
	case 1: 
	  color &= 0xffff;
	  color |= color << 16;
	}

	/* Bank shadowing or ASM may speed this up --
           cannot memcpy() strips because memcpy() aligns with the 
           write buffer, not the mmap region (ick.) */
	align = ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	pixpt = BANKFB + align;
	if (!(align &= 0x3)) align = 4;

	w = w << LOGBYTPP;

	switch (align) {
	case 3:
	  memset(pixpt++, (int) color, 1);
	  break;
	case 1:
	  memset(pixpt++, (int) color, 1);
	  if (w == 2) { memset(pixpt, (int) color, 1) ; return 0; };
	  if (w == 1) return 0;
	case 2:
	  if (w == 1) { memset(pixpt, (int) color, 1) ; return 0; };
	  memcpy(pixpt, &color, 2);
	  pixpt += 2;
	}	  
	while (w > 7 - align) {
	  memcpy(pixpt, &color, 4);
	  w -= 4;
	  pixpt += 4;
	}
	if ( w > 5 - align) {
	  memcpy(pixpt, &color, 2);
	  w -= 2;
	  pixpt += 2;
	}
	if (w > 4 - align) memset(pixpt, (int)color, 1);
	return 0;
}

int GGIdrawhline_nc(ggi_visual *vis,int x,int y,int w)
{
	ggi_pixel color;
	unsigned int align;
	uint8_t *pixpt;

	color = LIBGGI_GC_FGCOLOR(vis);

	switch(LOGBYTPP) {
	case 0: 
	  color &= 0xff;
	  color |= color << 8;
	case 1: 
	  color &= 0xffff;
	  color |= color << 16;
	}

	/* Bank shadowing or ASM may speed this up --
           cannot memcpy() strips because memcpy() aligns with the 
           write buffer, not the mmap region (ick.) */
	align = ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	pixpt = BANKFB + align;
	if (!(align &= 0x3)) align = 4;

	w = w << LOGBYTPP;

	switch (align) {
	case 3:
	  memset(pixpt++, (int) color, 1);
	  break;
	case 1:
	  memset(pixpt++, (int) color, 1);
	  if (w == 2) { memset(pixpt, (int) color, 1) ; return 0; };
	  if (w == 1) return 0;
	case 2:
	  if (w == 1) { memset(pixpt, (int) color, 1) ; return 0; };
	  memcpy(pixpt, &color, 2);
	  pixpt += 2;
	}	  
	while (w > 7 - align) {
	  memcpy(pixpt, &color, 4);
	  w -= 4;
	  pixpt += 4;
	}
	if ( w > 5 - align) {
	  memcpy(pixpt, &color, 2);
	  w -= 2;
	  pixpt += 2;
	}
	if (w > 4 - align) memset(pixpt, (int)color, 1);
	return 0;
}

int GGIputhline(ggi_visual *vis,int x,int y,int w,const void *buff)
{ 
	uint8_t *pixpt;
	const uint8_t *buffer=(const uint8_t *)buff;
	unsigned int align;

	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) 
                return 0;
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		int diff=(LIBGGI_GC(vis)->cliptl.x)-x;
		x+=diff;
		w-=diff;
		buffer=((uint8_t *)buffer)+diff;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	CHECKXYW(vis,x,y,w);

	/* Hitting mapped banks first is silly for hline since banks
           are usually bigger than one line ==> single segv */

	align = ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	pixpt = BANKFB + align;
	if (!(align &= 0x3)) align = 4;

	w = w << LOGBYTPP;

	switch (align) {
	case 3:
	  memcpy(pixpt++, buffer++, 1);
	  break;
	case 1:
	  memcpy(pixpt++, buffer++, 1);
	  if (w == 2) { memcpy(pixpt, buffer, 1) ; return 0; };
	  if (w == 1) return 0;
	case 2:
	  if (w == 1) { memcpy(pixpt, buffer, 1) ; return 0; };
	  memcpy(pixpt, buffer, 2);
	  pixpt += 2;
	  buffer += 2;
	}	  
	if (w > 4 - align) memcpy(pixpt, buffer, w - 4 + align);

	return 0;
}

int GGIgethline(ggi_visual *vis,int x,int y,int w,void *buff)
{ 
	uint8_t *pixpt,*buffer=(uint8_t *)buff;
	unsigned int align;

	CHECKXYW(vis,x,y,w);

	/* Bank shadowing or ASM may speed this up --
           cannot memcpy() strips because memcpy() aligns with the 
           write buffer, not the mmap region (ick.) */

	align = ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	pixpt = RBANKFB + align;
	if (!(align &= 0x3)) align = 4;

	w = w << LOGBYTPP;

	switch (align) {
	case 3:
	  memcpy(buffer++, pixpt++, 1);
	  break;
	case 1:
	  memcpy(buffer++, pixpt++, 1);
	  if (w == 2) { memcpy(buffer, pixpt, 1) ; return 0; };
	  if (w == 1) return 0;
	case 2:
	  if (w == 1) { memcpy(buffer, pixpt, 1) ; return 0; };
	  memcpy(buffer, pixpt, 2);
	  pixpt += 2;
	  buffer += 2;
	}	  
	while (w > 7 - align) {
	  memcpy(buffer, pixpt, 4);
	  w -= 4;
	  buffer += 4;
	  pixpt += 4;
	}
	if ( w > 5 - align) {
	  memcpy(buffer, pixpt, 2);
	  w -= 2;
	  buffer += 2;
	  pixpt += 2;
	}
	if (w > 4 - align) memcpy(buffer, pixpt, 1);

	return 0;
}
