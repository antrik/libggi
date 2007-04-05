/* $Id: hline.c,v 1.13 2007/04/05 12:46:45 pekberg Exp $
******************************************************************************

   Graphics library for GGI. Horizontal lines.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#include "lin4rlib.h"

/**********************************/
/* draw/get/put a horizontal line */
/**********************************/

static inline void
do_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	uint8_t *fb;
	uint8_t fg;

	fb = (uint8_t *) LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x/2);
	fg = (uint8_t) LIBGGI_GC_FGCOLOR(vis) | (LIBGGI_GC_FGCOLOR(vis)<<4);
	
	PREPARE_FB(vis);

	/* x is odd.  Read-modify-write right pixel. */
	if (x & 0x01) {
		*fb = (*fb & 0x0f) | (fg & 0xf0);
		fb++;
		w--;
	}
	
	memset(fb, fg, (size_t)(w/2));

	/* Dangling right pixel. */
	if (w & 0x01) {
		fb += w/2;
		*fb = (*fb & 0xf0) | (fg & 0x0f);
	}
}


int GGI_lin4r_drawhline(struct ggi_visual *vis, int x, int y, int w)
{
	LIBGGICLIP_XYW(vis, x, y, w);

	do_drawhline(vis, x, y, w);
		
	return 0;
}

int GGI_lin4r_drawhline_nc(struct ggi_visual *vis, int x, int y, int w)
{
	do_drawhline(vis, x, y, w);
		
	return 0;
}

int GGI_lin4r_packed_puthline(struct ggi_visual *vis, int x, int y, int w, const void *buffer)
{ 
	uint8_t *fb;
	uint16_t color;
	const uint8_t *buf = (const uint8_t *)buffer;
	int diff = 0;

	if (y < LIBGGI_GC(vis)->cliptl.y ||
	    y >= LIBGGI_GC(vis)->clipbr.y) {
		return 0;
	}
	if (x < LIBGGI_GC(vis)->cliptl.x) {
		diff = LIBGGI_GC(vis)->cliptl.x - x;
		x   += diff;
		buf += diff >> 1;
		w -= diff;
		diff &= 1;
	}
	if (x+w > LIBGGI_GC(vis)->clipbr.x) {
		w = LIBGGI_GC(vis)->clipbr.x - x;
	}
	if (w <= 0) return 0;
	PREPARE_FB(vis);
	
	fb = (uint8_t*)LIBGGI_CURWRITE(vis)+y*LIBGGI_FB_W_STRIDE(vis)+(x/2);
	
	/* Might as well take the memcpy speed up (multiple byte copy). */
	if (!((diff + x) & 1)) {
		if (diff & 1) {
			// x and diff both odd, fixup initial pixel */
			*fb &= 0x0f;
			*fb++ |= *buf++ & 0xf0;
			if (!--w) return 0;
		}
		memcpy(fb, buf, (size_t)(w>>1));
		if (w & 1)
			*(fb+(w>>1)) = ( *(fb+(w>>1)) & 0xf0)
				| (*(buf+(w>>1)) & 0x0f);
		return 0;
	}
	
	if (diff & 1) {
		/* diff-odd, x-even -> unite with the diff-even, x-odd case */
		*fb &= 0xf0;
		*fb |= *buf++ >> 4;
		if (!--w) return 0;
	}

	/* x is odd. */
	/* Could be optimized for multiple-byte copy. */

	color = *fb & 0x0f;
	
	for (; w > 0; w -= 2) {
		color |= *(buf++) << 4;
		/* Color is now a 12-bit quantity.  Lower 8 bits hold the
		 * memory to copy.  Upper 4 bits hold the upper 4 bits of the
		 * buffer byte read; we'll need it next loop. */
		*(fb++) = color;
		color >>= 8;
	}
	
	if (!w) {
		*fb = color | (*fb & 0xf0);
	}
	
	return 0;
}


int GGI_lin4r_packed_gethline_nc(struct ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8_t *fb;
	uint16_t color;
	uint8_t *buf = (uint8_t *)buffer;

	PREPARE_FB(vis);

	fb = (uint8_t*)LIBGGI_CURREAD(vis)+y*LIBGGI_FB_R_STRIDE(vis)+(x/2);
	
	/* If width is odd, the last 4 bits of the buffer's last
	 * byte will hold an extraneous pixel.  No big deal. */
	
	if (!(x & 0x01)) {
		memcpy(buf, fb, (size_t)((w/2) + (w & 0x01)));
		return 0;
	}
	
	/* x is odd. */
	color = (*fb & 0xf0)>>4;
	
	for (; w > 1; w -= 2) {
		color |= *(++fb) << 4;
		*(buf++) = color;
		color >>= 8;
	}

	if (w) {
		*(buf++) = color;
	}

	return 0;
}

int GGI_lin4r_packed_gethline(struct ggi_visual *vis,int x,int y,int w,void *buffer)
{ 
	uint8_t *fb;
	uint16_t color;
	uint8_t *buf = (uint8_t *)buffer;
	int odd=0;

	PREPARE_FB(vis);

	if (y<0||y>=LIBGGI_VIRTY(vis)) return 0;
	if (x<0) {
		w+=x;   /* x is negative. w will _de_crease */
		buf-=x/2;
		odd=x&1;
		x=0;
	}
	if (x+w>LIBGGI_VIRTX(vis)) {
		w=LIBGGI_VIRTX(vis)-x;
	}
	if (w<=0) return 0;

	fb = (uint8_t*)LIBGGI_CURREAD(vis)+y*LIBGGI_FB_R_STRIDE(vis)+(x/2);
	
	/* If memory is aligned just memcpy. 
	 * Note, that if odd&1, x=0 is guaranteed.
	 */
	if (!((x+odd) & 0x01)) {
		memcpy(buf, fb, (size_t)(w>>1));
		/* If length is odd, we need to copy one more nibble */
		if (w&1) {
			buf[(w>>1)]&=0xf0;
			buf[(w>>1)]|=fb[(w>>1)]&0x0f;
		}
		return 0;
	}

	if (odd) {	
		/* we start at an odd place in the buffer. Thus we need 
		 * to copy the low nibble of the first fb byte to the high
		 * nibble of the first buffer byte.
		 */
		buf[0]&=0x0f;
		buf[0]|=((fb[0]&0x0f)<<4);
		x++;buf++;w--;
	}

	/* Now the buffer is aligned, but x is odd. 
	 * We use a 12 bit shift register to copy to the buffer.
	 */
	
	color = (*fb & 0xF0) >> 4;

	for (; w > 1; w -= 2) {
		color |= (*(++fb)<<4);
		*(buf++) = color;
		color >>= 8;
	}

	if (w) {
		*buf&=0xf0;
		*buf|= color &0x0f;
	}
	
	return 0;
}
