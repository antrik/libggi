/* $Id: hline.c,v 1.10 2007/05/06 05:02:50 pekberg Exp $
******************************************************************************

   Linear 1 horizontal lines.

   Copyright (C) 1995 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1998 Andrew Apted   [andrew@ggi-project.org]
   Copyright (C) 2007 Peter Rosin    [peda@lysator.liu.se]

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
#include "lin1lib.h"


static inline void
do_drawhline(struct ggi_visual *vis, int x, int y, int w)
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
			return;
		}

		mask=(0xff>>i);
		*adr = (*adr & ~mask) | (color & mask);
		adr++;
	}
	
	while ((j-=8)>=0) {
		*adr = color;
		adr++;
	}
    
	/* Draw `back` pixels if necessary */
	mask=~(0xff>>(j&7));
	*adr = (*adr & ~mask) | (color & mask);
}

int GGI_lin1_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	do_drawhline(vis, x, y, w);
		
	return 0;
}

int GGI_lin1_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	do_drawhline(vis, x, y, w);
		
	return 0;
}

int
GGI_lin1_unpacked_puthline(struct ggi_visual *vis,
	int x, int y, int w, const void *buffer)
{
	uint8_t *adr;
	const uint8_t *buff = (const uint8_t *)buffer;
	int i;
	uint8_t bm;

	LIBGGICLIP_XYW_BUFMOD(vis, x, y, w, buff, >> 3);
	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURWRITE(vis) +
		x / 8 + y * LIBGGI_FB_W_STRIDE(vis);

	bm = 0x80 >> (x & 7);
	for (i = 0; i < w; ++i, bm >>= 1) {
		if (!bm) {
			bm = 0x80;
			++adr;
		}
		if (*buff++ & 1)
			*adr |= bm;
		else
			*adr &= ~bm;
	}

	return 0;
}

int
GGI_lin1_packed_puthline(struct ggi_visual *vis,
	int x, int y, int w, const void *buffer)
{ 
	uint8_t *adr;
	const uint8_t *buff=(const uint8_t *)buffer;
	int mask,i,j,diff=0;
	uint8_t foo;
	uint16_t color;

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

		i -= diff;
		if (i <= 0) {
			i += 8;
			color <<= 8;
			if (diff + w > 8)
				color |= *(++buff);
		}
		
		foo = *adr & ~mask;
		*adr = foo | ((color >> i) & mask);

		if (j<0)
			return 0;
		else
			adr++;
	} else {
		j = w;
		i = 8 - diff;
	}

	while ((j -= 8) > 0) {
		color <<= 8;
		color |= *(++buff);
		*adr++ = color >> i;
	}

	color <<= 8;
	if (!j) {
		if (i != 8)
			color |= *(++buff);
		*adr = color >> i;
		return 0;
	}

	if (i < j + 8)
		color |= *(++buff);

	mask = 0xff << -j;
	foo = *adr & ~mask;
	*adr = foo | ((color >> i) & mask);

	return 0;
}

int
GGI_lin1_unpacked_gethline(struct ggi_visual *vis,
	int x, int y, int w, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int i, bm;

	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURREAD(vis) +
		x / 8 + y * LIBGGI_FB_R_STRIDE(vis);

	bm = 0x80 >> (x & 7);
	for (i = 0; i < w; ++i, bm >>= 1) {
		if (!bm) {
			bm = 0x80;
			++adr;
		}
		*buff++ = !!(*adr & bm);
	}

  	return 0;
}

int
GGI_lin1_packed_gethline_nc(struct ggi_visual *vis,
	int x, int y, int w, void *buffer)
{ 
	uint8_t *adr,*buff=(uint8_t *)buffer;
	int i,j;
	uint8_t color;

	if (w <= 0)
		return 0;
	
	PREPARE_FB(vis);

	adr=((uint8_t *)(LIBGGI_CURREAD(vis)));
	adr+=(x/8+y*LIBGGI_FB_R_STRIDE(vis));

	i = x & 7;
	if (!i) {
		memcpy(buff, adr, (w + 7) >> 3);
		return 0;
	}

	if (x < 0)
		--adr;

	j = i + w - 8;
	*buff = *adr++ << i;

	if (j <= 0)
		return 0;
	
	for (; j > 0; j -= 8) {
		color = *adr++;
		*buff++ |= color >> (8 - i);
		*buff = color << i;
	}

	if (j&=7)
		*buff |= *adr >> (8 - i);
  
	return 0;
}

int
GGI_lin1_packed_gethline(struct ggi_visual *vis,
	int x, int y, int w, void *buffer)
{ 
	uint8_t *adr;
	uint8_t *buff = (uint8_t *)buffer;
	int i, j;
	uint8_t pixels;
	uint8_t mask;

	if (y < 0 || y >= LIBGGI_VIRTY(vis))
		return 0;

	i = x & 7;
	if (x < 0) {
		w -= -x;
		buff += -x >> 3;
		x = 0;
	}
	if (x + w > LIBGGI_VIRTX(vis))
		w = LIBGGI_VIRTX(vis) - x;
	if (w <= 0)
		return 0;
	
	PREPARE_FB(vis);

	adr = (uint8_t *)LIBGGI_CURREAD(vis) +
		(x >> 3) + y * LIBGGI_FB_R_STRIDE(vis);

	if (!i) {
		j = w & 7;
		memcpy(buff, adr, w >> 3);
		if (j) {
			buff += w >> 3;
			adr += w >> 3;
			mask = 0xff >> j;
			*buff &= mask;
			*buff |= *adr & ~mask;
		}
		return 0;
	}

	if (!x) {
		/* unaligned clipping to the left */
		j = w - 8;
		mask = 0xff << i;
		if (j + 8 <= i) {
			mask ^= 0xff << (i - j - 8);
			*buff &= ~mask;
			*buff |= (*adr >> (8 - i)) & mask;
			return 0;
		}
		*buff &= mask;
		pixels = *adr++;
		*buff++ |= (pixels >> (8 - i)) & ~mask;
	}
	else {
		j = i + w - 8;
		pixels = *adr++;
	}

	for (; j >= i; j -= 8) {
		*buff = pixels << i;
		pixels = *adr++;
		*buff++ |= pixels >> (8 - i);
	}

	if (j + 8 == i)
		return 0;

	mask = 0xff << (i - j);
	*buff &= ~mask;
	*buff |= (pixels << i) & mask;

	if (j > 0)
		*buff |= (*adr >> (8 - i)) & mask;

	return 0;
}
