/* $Id: mmio.h,v 1.2 2006/01/01 09:50:36 cegger Exp $
******************************************************************************

   LibGGI - 3DLabs Permedia2 acceleration for fbdev target

   Copyright (C) 2005 Christoph Egger

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

/* This file is included by 3dlabs_pm2.h */

static inline void
pm2_out8(volatile uint8_t *mmioaddr, uint8_t value, uint32_t reg)
{
	*((volatile uint8_t*)(mmioaddr+reg)) = value;
}

static inline void
pm2_out16(volatile uint8_t *mmioaddr, uint16_t value, uint32_t reg)
{
	*((volatile uint16_t*)(mmioaddr+reg)) = value;
}

static inline void
pm2_out32(volatile uint8_t *mmioaddr, uint32_t value, uint32_t reg)
{
	*((volatile uint32_t*)(mmioaddr+reg)) = value;
}

static inline volatile uint8_t
pm2_in8(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint8_t*)(mmioaddr+reg));
}

static inline volatile uint16_t
pm2_in16(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint16_t*)(mmioaddr+reg));
}

static inline volatile uint32_t
pm2_in32(volatile uint8_t *mmioaddr, uint32_t reg)
{
	return *((volatile uint32_t*)(mmioaddr+reg));
}

/* Wait for fifo space */
static inline void
pm2_waitfifo(volatile uint8_t *mmioaddr, unsigned int space)
{
	while (pm2_in32(mmioaddr, PM2R_IN_FIFO_SPACE) < space);
}


/* Wait for idle accelerator */
static inline void
pm2_waitidle(volatile uint8_t *mmioaddr)
{
	while (pm2_in32(mmioaddr, PM2R_DMA_COUNT) != 0);
	pm2_waitfifo(mmioaddr, 2);
	pm2_out32(mmioaddr, 0x400, PM2R_FILTER_MODE);
	pm2_out32(mmioaddr, 0, PM2R_SYNC);
	do {
		while (pm2_in32(mmioaddr, PM2R_OUT_FIFO_WORDS) == 0);
	} while (pm2_in32(mmioaddr, PM2R_OUT_FIFO) != 0x0188);
}

/* Set the accelerator's foreground color */
static inline void
pm2_setcol(volatile uint8_t *mmioaddr, ggi_mode *mode, ggi_pixel pixelfg,
	   uint32_t reg)
{
	/* The register format is the raw data format of the framebuffer.
	 *
	 * If the framebuffer is used in 8bit packed mode, then data
	 * should be repeated in all 4 bytes of the register.
	 * If the framebuffer is used in 16bit packed mode, then data
	 * must be repeated in both halves of the register.
	 */

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
	}

	pm2_waitfifo(mmioaddr, 2);
	pm2_out32(mmioaddr, UNIT_ENABLE, PM2R_COLOR_DDA_MODE);
	pm2_out32(mmioaddr, pixelfg, reg);
}


/* Set the accelerator's clipping rectangle */
static inline void
pm2_setclip(volatile uint8_t *mmioaddr, ggi_gc *gc)
{
	DPRINT_MISC("pm2_setclip(%p, %p) entered\n",
		mmioaddr, gc);

	pm2_waitfifo(mmioaddr, 3);
	pm2_out32(mmioaddr, ((gc->cliptl.y & 0x0fff) << 16)
			| (gc->cliptl.x & 0x0fff),
			PM2R_SCISSOR_MINXY);
	pm2_out32(mmioaddr, ((gc->clipbr.y & 0x0fff) << 16)
			| (gc->clipbr.x & 0x0fff),
			PM2R_SCISSOR_MAXXY);
	pm2_out32(mmioaddr, 1, PM2R_SCISSOR_MODE);
}

static inline void
pm2_loadcoord(volatile uint8_t *mmioaddr, int x, int y, int w, int h)
{
	/* pm2_waitfifo(mmioaddr, 2); */
	pm2_out32(mmioaddr, ((h & 0x0fff) << 16) | (w & 0x0fff),
		PM2R_RECTANGLE_SIZE);
	pm2_out32(mmioaddr, ((y & 0x0fff) << 16) | (x & 0x0fff),
		PM2R_RECTANGLE_ORIGIN);
}

static inline void
pm2_loadrop(volatile uint8_t *mmioaddr, int rop)
{
	/* pm2_waitfifo(mmioaddr, 1); */
	pm2_out32(mmioaddr, rop << 1 | UNIT_ENABLE, PM2R_LOGICAL_OP_MODE);
}


static inline void
pm2_move32(uint32_t *dest,
	const uint32_t *src,
	unsigned int size)
{
	while (size--) {
		*dest++ = *src++;
	}
}
