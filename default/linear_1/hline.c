/* $Id: hline.c,v 1.3 2005/07/30 11:39:59 cegger Exp $
******************************************************************************

   Linear 1 horizontal lines.

   Copyright (C) 1995 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]

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

#include "lin1lib.h"


int GGI_lin1_drawhline_nc(ggi_visual *vis,int x,int y,int w)
{
	uint8_t *adr;
	int i,j,mask,color;

	PREPARE_FB(vis);

	color=(LIBGGI_GC_FGCOLOR(vis)&1)*0xFF;

	adr=((uint8_t *)(LIBGGI_CURWRITE(vis)));
	adr+=(x/8+y*LIBGGI_FB_W_STRIDE(vis));

	/* Draw 'front' pixels */ 
	j=w;
	if ((i=x&7)) {
		if ((j=j+i-8)<=0) {
			mask=(0xff>>i)&(0xff<<-j); 
			*adr   = (*adr & ~mask) | (color & mask);
			return 0;
		} else {
			mask=(0xff>>i);
			*adr = (*adr & ~mask) | (color & mask);
			adr++;
		}
	}
	
	while ((j-=8)>=0) {
		*adr = color;
		adr++;
	}
    
	/* Draw `back` pixels if necessary */
	mask=~(0xff>>(j&7));
	*adr = (*adr & ~mask) | (color & mask);

	return 0;
}

int GGI_lin1_puthline(ggi_visual *vis,int x,int y,int w,const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff=(const uint8_t *)buffer;
	int mask,i,j,diff=0;
	uint8_t foo,color;

	/* Clipping */
	if (y<(LIBGGI_GC(vis)->cliptl.y) || y>=(LIBGGI_GC(vis)->clipbr.y)) return 0;
	if (x< (LIBGGI_GC(vis)->cliptl.x)) {
		diff=(LIBGGI_GC(vis)->cliptl.x)-x;
		x+=diff;w-=diff;
		buff+=(diff>>3);diff&=7;
	}
	if (x+w>(LIBGGI_GC(vis)->clipbr.x)) {
		w=(LIBGGI_GC(vis)->clipbr.x)-x;
	}
	if (w<=0) return 0;

	PREPARE_FB(vis);

	adr=((uint8_t *)(LIBGGI_CURWRITE(vis)));
	adr+=(x/8+y*LIBGGI_FB_W_STRIDE(vis));

	color=buff[0];
	if ((i=x&7)) {
		if ((j=w+i-8)<0) {
			mask=(0xff>>i)&(0xff<<-j); 
		} else {
			mask=0xff>>i;
		}

		foo = (*adr & ~mask);
		*adr = foo | ((color >> (i+=diff)) & mask);

		if (j<0)
			return 0;
		else
			adr++;
	} else
		j=w;i+=diff;

	while ((j-=8)>=0) {
		color <<= (8-i);
		color |= *(++buff) >> i;
		*adr = color;
	}

    
	if (j&=7) {
		color <<= (8-i);
		color |= *(++buff) >> i;

		mask=~(0xff>>j);
		foo = (*adr & ~mask);
		*adr = foo | ((color >> i) & mask);
	}
  
	return 0;
}

int GGI_lin1_gethline(ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8_t *adr,*buff=(uint8_t *)buffer;
	int mask,i,j;
	uint8_t color;

	PREPARE_FB(vis);

	adr=((uint8_t *)(LIBGGI_CURREAD(vis)));
	adr+=(x/8+y*LIBGGI_FB_R_STRIDE(vis));

	if ((i=x&7)) {
		if ((j=w+i-8)<0) {
			mask=(0xff>>i)&(0xff<<-j); 
		} else {
			mask=0xff>>i;
		}

		*buff = (*adr & mask) << (8-i);

		if (j<0)
			return 0;
		else
			adr++;
	} else
		j=w;
	

	while ((j-=8)>=0) {
		color = *adr;
		*(buff++) |= color >> i;
		*(buff) = (color << (8-i));
	}

    
	if (j&=7) {
		mask=~(0xff>>j);
		*buff |= (*adr & mask) >> i;
	}
  
	return 0;
}
