/* $Id: genblit.c,v 1.1 2001/05/12 23:02:37 cegger Exp $
******************************************************************************

   Display-trueemu : genblit

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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


/**************************************************
 ***
 ***  Generic blit functions
 ***
 ***  (this file is #included by others)
 ***
 **************************************************/


#define HICOL_DITHER(dn)  \
	priv->R[src[R_OFF]][dn] | priv->G[src[G_OFF]][dn] |  \
	priv->B[src[B_OFF]][dn]

#define PAL_DITHER(dn,off)  \
	priv->T[(((int) src[R_OFF+(off)] & 0xf8) << 7) |  \
	      (((int) src[G_OFF+(off)] & 0xf8) << 2) |  \
	      (((int) src[B_OFF+(off)] & 0xf8) >> 3)] [dn]


/* NOTE:
 *	We guarantee that these blitting routines always start on an
 *	even pixel column.
 *
 * NOMENCLATURE:
 *	b32 (for example) refers to 32 bits per pixel.
 *	d4  (for example) refers to four pixel dithering.
 *	od == odd and ev == even (the dither row).
 */


/* ==== 32 bit ==== */

static BLITFUNC(_ggi_trueemu_blit_b32_d0)
{
	uint8 *dest = dest_raw;

	for (; width > 0; width--) {
		*dest++ = src[B_OFF];  /* blue */
		*dest++ = src[G_OFF];  /* green */
		*dest++ = src[R_OFF];  /* red */
		*dest++ = 0;
		src += SRC_STEP;
	}
}


/* ==== 24 bit ==== */

static BLITFUNC(_ggi_trueemu_blit_b24_d0)
{
	uint8 *dest = dest_raw;

	for (; width > 0; width--) {
		*dest++ = src[B_OFF];  /* blue */
		*dest++ = src[G_OFF];  /* green */
		*dest++ = src[R_OFF];  /* red */
		src += SRC_STEP;
	}
}


/* ==== 16 bit ==== */

static BLITFUNC(_ggi_trueemu_blit_b16_d0)
{
	uint16 *dest = dest_raw;

	for (; width > 0; width--) {
		*dest++ = HICOL_DITHER(0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b16_d2_ev)
{
	uint16 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = HICOL_DITHER(0); src += SRC_STEP;
		*dest++ = HICOL_DITHER(1); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = HICOL_DITHER(0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b16_d2_od)
{
	uint16 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = HICOL_DITHER(1); src += SRC_STEP;
		*dest++ = HICOL_DITHER(0); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = HICOL_DITHER(1); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b16_d4_ev)
{
	uint16 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = HICOL_DITHER(0); src += SRC_STEP;
		*dest++ = HICOL_DITHER(2); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = HICOL_DITHER(0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b16_d4_od)
{
	uint16 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = HICOL_DITHER(3); src += SRC_STEP;
		*dest++ = HICOL_DITHER(1); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = HICOL_DITHER(3); src += SRC_STEP;
	}
}


/* ==== 8 bit ==== */

static BLITFUNC(_ggi_trueemu_blit_b8_d0)
{
	uint8 *dest = dest_raw;

	for (; width > 0; width--) {
		*dest++ = PAL_DITHER(0, 0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b8_d2_ev)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = PAL_DITHER(0, 0); src += SRC_STEP;
		*dest++ = PAL_DITHER(1, 0); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = PAL_DITHER(0, 0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b8_d2_od)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = PAL_DITHER(1, 0); src += SRC_STEP;
		*dest++ = PAL_DITHER(0, 0); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = PAL_DITHER(1, 0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b8_d4_ev)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = PAL_DITHER(0, 0); src += SRC_STEP;
		*dest++ = PAL_DITHER(2, 0); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = PAL_DITHER(0, 0); src += SRC_STEP;
	}
}

static BLITFUNC(_ggi_trueemu_blit_b8_d4_od)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = PAL_DITHER(3, 0); src += SRC_STEP;
		*dest++ = PAL_DITHER(1, 0); src += SRC_STEP;
	}

	if (width == 1) {
		*dest = PAL_DITHER(3, 0); src += SRC_STEP;
	}
}


/* ==== 4 bit ==== */

static BLITFUNC(_ggi_trueemu_blit_b4_d0)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = (PAL_DITHER(0, 0)) | 
			  (PAL_DITHER(0, SRC_STEP) << 4);
		src += SRC_STEP*2;
	}

	if (width == 1) {
		*dest = PAL_DITHER(0, 0);
	}
}

static BLITFUNC(_ggi_trueemu_blit_b4_d2_ev)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = (PAL_DITHER(0, 0)) | 
			  (PAL_DITHER(1, SRC_STEP) << 4);
		src += SRC_STEP*2;
	}

	if (width == 1) {
		*dest = PAL_DITHER(0, 0);
	}
}

static BLITFUNC(_ggi_trueemu_blit_b4_d2_od)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = (PAL_DITHER(1, 0)) | 
			  (PAL_DITHER(0, SRC_STEP) << 4);
		src += SRC_STEP*2;
	}

	if (width == 1) {
		*dest = PAL_DITHER(1, 0);
	}
}

static BLITFUNC(_ggi_trueemu_blit_b4_d4_ev)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = (PAL_DITHER(0, 0)) | 
			  (PAL_DITHER(2, SRC_STEP) << 4);
		src += SRC_STEP*2;
	}

	if (width == 1) {
		*dest = PAL_DITHER(0, 0);
	}
}

static BLITFUNC(_ggi_trueemu_blit_b4_d4_od)
{
	uint8 *dest = dest_raw;

	for (; width > 1; width -= 2) {
		*dest++ = (PAL_DITHER(3, 0)) | 
			  (PAL_DITHER(1, SRC_STEP) << 4);
		src += SRC_STEP*2;
	}

	if (width == 1) {
		*dest = PAL_DITHER(3, 0);
	}
}
