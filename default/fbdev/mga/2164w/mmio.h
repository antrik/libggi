/* $Id: mmio.h,v 1.6 2005/07/31 15:30:34 soyt Exp $
******************************************************************************

   LibGGI - Millennium II acceleration for fbdev target

   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

/* This file is included by m2164w.h */

static inline void
mga_out8(volatile uint8_t *mmioaddr, uint8_t value, uint32_t reg)
{
	*((volatile uint8_t*)(mmioaddr+reg)) = value;
}

static inline void
mga_out16(volatile uint8_t *mmioaddr, uint16_t value, uint32_t reg)
{
	*((volatile uint16_t*)(mmioaddr+reg)) = value;
}

static inline void
mga_out32(volatile uint8_t *mmioaddr, uint32_t value, uint32_t reg)
{
	*((volatile uint32_t*)(mmioaddr+reg)) = value;
}

static inline volatile uint8_t
mga_in8(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint8_t*)(mmioaddr+reg));
}

static inline volatile uint16_t
mga_in16(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint16_t*)(mmioaddr+reg));
}

static inline volatile uint32_t
mga_in32(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint32_t*)(mmioaddr+reg));
}

/* Wait for fifo space */
static inline void
mga_waitfifo(volatile uint8_t *mmioaddr, int space)
{
#ifdef GGI_LITTLE_ENDIAN
	while (mga_in8(mmioaddr, FIFOSTATUS) < (unsigned)space) {
#else
	while ((mga_in32(mmioaddr, FIFOSTATUS) & 0xff) < (unsigned)space) {
#endif
	}
}


/* Wait for idle accelerator */
static inline void
mga_waitidle(volatile uint8_t *mmioaddr)
{
	while (mga_in32(mmioaddr, STATUS) & 0x10000) {
	}
}

/* Set the accelerator's foreground color */
static inline void
mga_setcol(volatile uint8_t *mmioaddr, ggi_mode *mode, ggi_pixel pixelfg,
	   uint32_t reg)
{
	switch (GT_SIZE(mode->graphtype)) {
	case 8:
		pixelfg &= 0xff;
		pixelfg |= (pixelfg << 8) |
			(pixelfg << 16) | (pixelfg << 24);
		break;
	case 16:
		pixelfg &= 0xffff;
		pixelfg |= pixelfg << 16;
		break;
#ifdef GGI_LITTLE_ENDIAN
	case 24:
		pixelfg |= ((pixelfg << 24) & 0xff000000);
		break;
#endif
	case 32:
		pixelfg |= 0xff000000;
		break;
	}

	mga_waitfifo(mmioaddr, 1);
	mga_out32(mmioaddr, pixelfg, reg);
}


/* Set the accelerator's clipping rectangle */
static inline void
mga_setclip(volatile uint8_t *mmioaddr, ggi_gc *gc, int virtx, int yadd)
{
	int topy = gc->cliptl.y + yadd;
	int boty = (gc->clipbr.y-1) + yadd;

	mga_waitfifo(mmioaddr, 3);
	mga_out32(mmioaddr, ((unsigned)gc->cliptl.x & 0x07FF)
		  | ((((unsigned)gc->clipbr.x-1) & 0x07FF) << 16), CXBNDRY);
	mga_out32(mmioaddr, (unsigned)(virtx * topy)
		  & 0x00FFFFFF, YTOP);
	mga_out32(mmioaddr, (unsigned)(virtx * boty)
		  & 0x00FFFFFF, YBOT);
}


/* Set dwgctl */
static inline void
mga_setdwgctl(volatile uint8_t *mmioaddr, struct m2164w_priv *priv,
	      uint32_t dwgctl)
{
	mga_out32(mmioaddr, dwgctl, DWGCTL);
	priv->dwgctl = dwgctl;
}
