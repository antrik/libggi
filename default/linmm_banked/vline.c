/* $Id: vline.c,v 1.3 2004/12/01 23:08:05 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Brian S. Julin   [bri@calyx.com]
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

/********************************/
/* draw/get/put a vertical line */
/********************************/
int GGIdrawvline(ggi_visual *vis,int x,int y,int h)
{
	uint8 *pixpt;
	unsigned int w=LIBGGI_FB_W_STRIDE(vis);
	ggi_pixel color = LIBGGI_GC_FGCOLOR(vis);

	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y     +=diff;
		h-=diff;
	}
	if (y+h>(LIBGGI_GC(vis)->clipbr.y)) {
		h=(LIBGGI_GC(vis)->clipbr.y)-y;
	}  

	pixpt = BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	
	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	
	for(;h--;pixpt+=w << LOGBYTPP) 
		memcpy(pixpt, &color, 1 << LOGBYTPP);
  
  return 0;
}

int GGIdrawvline_nc(ggi_visual *vis,int x,int y,int h)
{
	uint8 *pixpt;
	unsigned int w=LIBGGI_FB_W_STRIDE(vis);
	ggi_pixel color = LIBGGI_GC_FGCOLOR(vis);
	
	pixpt = BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
	
	/* Supports only 8, 16, 32 bit modes -- 
	   impossible to pixel nonaligned */ 
	
	for(;h--;pixpt+=w << LOGBYTPP) 
		memcpy(pixpt, &color, 1 << LOGBYTPP);
	
	return 0;
}



int GGIputvline(ggi_visual *vis,int x,int y,int h,const void *buff)
{
  uint8 *pixpt;
  const uint8 *buffer=(const uint8 *)buff;
  unsigned int w=LIBGGI_FB_W_STRIDE(vis);


	/* Clipping */
	if (x< (LIBGGI_GC(vis)->cliptl.x) ||
	    x>=(LIBGGI_GC(vis)->clipbr.x)) return 0;
	if (y< (LIBGGI_GC(vis)->cliptl.y)) {
		int diff=(LIBGGI_GC(vis)->cliptl.y)-y;
		y     +=diff;
		h-=diff;
	}
	if (y+h>(LIBGGI_GC(vis)->clipbr.y)) {
		h=(LIBGGI_GC(vis)->clipbr.y)-y;
	}  

  pixpt = BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
  
  /* Supports only 8, 16, 32 bit modes -- 
     impossible to pixel nonaligned */   
  for(;h--;pixpt+=w << LOGBYTPP) { 
    memcpy(pixpt, buffer, 1 << LOGBYTPP);
    buffer += (1 << LOGBYTPP);
  }
  
  return 0;
}


int GGIputvline_nc(ggi_visual *vis,int x,int y,int h,const void *buff)
{
  uint8 *pixpt;
  const uint8 *buffer=(const uint8 *)buff;
  unsigned int w=LIBGGI_FB_W_STRIDE(vis);

  pixpt = BANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
  
  /* Supports only 8, 16, 32 bit modes -- 
     impossible to pixel nonaligned */   
  for(;h--;pixpt+=w << LOGBYTPP) { 
    memcpy(pixpt, buffer, 1 << LOGBYTPP);
    buffer += (1 << LOGBYTPP);
  }
  
  return 0;
}

int GGIgetvline(ggi_visual *vis,int x,int y,int h,void *buff)
{
  uint8 *pixpt,*buffer=(uint8 *)buff;
  unsigned int w=LIBGGI_FB_W_STRIDE(vis);
  
  CHECKXYH(vis,x,y,h);
  
  pixpt = RBANKFB + ((y * LIBGGI_FB_W_STRIDE(vis) + x) << LOGBYTPP);
  
  /* Supports only 8, 16, 32 bit modes -- 
     impossible to pixel nonaligned */ 
  
  for(;h--;pixpt+=w << LOGBYTPP) { 
    memcpy(buffer, pixpt, 1 << LOGBYTPP);
    buffer += (1 << LOGBYTPP);
  }
  return 0;
}

