/* $Id: crossblit.c,v 1.5 2005/07/30 11:39:57 cegger Exp $
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

#include "mga_g400.h"


static inline void
dbblit_32bpp(ggi_visual *src, int sx, int sy, int w, int h, 
	     ggi_visual *dst, int dx, int dy, uint32_t srcfmt)
{
	struct mga_g400_priv *priv = MGA_G400_PRIV(dst);
	volatile uint8_t *mmioaddr = FBDEV_PRIV(dst)->mmioaddr;
	int yadd = dst->w_frame_num * LIBGGI_VIRTY(dst);
	volatile uint32_t *dstptr;
	uint32_t dwgctl, bltmod = BLTMOD_BU32RGB;
	uint16_t opmode;
	uint8_t *srcptr;
	uint32_t *srcptr32;
	int srcinc;
	int maxpix;

	dstptr = priv->dmaaddr;
	srcinc = LIBGGI_FB_R_STRIDE(src);
	srcptr = (uint8_t*) LIBGGI_CURWRITE(src) + sy*srcinc + sx*4;
	srcinc -= w*4;
	maxpix = priv->dma_len/4;

	dy += yadd;

	switch (srcfmt) {
#if 0 /* This case is the default. */
	case GGI_DB_STD_24a32p8r8g8b8:
		bltmod = BLTMOD_BU32RGB;
		break;
#endif
	case GGI_DB_STD_24a32p8b8g8r8:
		bltmod = BLTMOD_BU32BGR;
		break;
	}

	dwgctl = bltmod | BOP_COPY | SHFTZERO | OP_ILOAD | SGNZERO;
#ifdef GGI_BIG_ENDIAN
	opmode = OPMODE_DMA_BLIT_WRITE | OPMODE_DMA_BE_32BPP;
#else
	opmode = OPMODE_DMA_BLIT_WRITE | OPMODE_DMA_LE;
#endif

	mga_gcupdate(mmioaddr, priv, LIBGGI_MODE(dst), LIBGGI_GC(dst),
		     LIBGGI_VIRTX(dst), yadd);

	if (priv->curopmode != opmode) {
		priv->curopmode = opmode;
		mga_waitidle(mmioaddr);
		mga_out16(mmioaddr, opmode, OPMODE);
	}
	if (priv->dwgctl != dwgctl) {
		mga_waitfifo(mmioaddr, 6);
		mga_setdwgctl(mmioaddr, priv, dwgctl);
	} else {
		mga_waitfifo(mmioaddr, 5);
	}
	mga_out32(mmioaddr, RS18(w-1), AR0);
	mga_out32(mmioaddr, 0, AR3);
	mga_out32(mmioaddr, 0, AR5);
	mga_out32(mmioaddr, (unsigned)(RS16(dx + w - 1) << 16) | RS16(dx), 
		  FXBNDRY);
	mga_out32(mmioaddr, (unsigned)(RS16(dy) << 16) | RS16(h),
		  YDSTLEN | EXECUTE);

	dst->accelactive = 1;

	if (w > maxpix) {
		while (h--) {
			int tmpw = w;

			while (tmpw) {
				int tmpw2 = (tmpw > maxpix ? maxpix : tmpw);

				tmpw -= tmpw2;
				while (tmpw2--) {
					srcptr32 = (uint32_t *)srcptr;
					*(dstptr++) = *(srcptr32++);
					srcptr = (uint8_t *)srcptr32;
				}
				dstptr = priv->dmaaddr;
			}
			srcptr += srcinc;
		}
	} else {
		while (h--) {
			int tmpw = w;

			while (tmpw--) {
				srcptr32 = (uint32_t *)srcptr;
				*(dstptr++) = *(srcptr32++);
				srcptr = (uint8_t *)srcptr32;
			}
			srcptr += srcinc;
			dstptr = priv->dmaaddr;
		}
	}
}


int GGI_mga_g400_crossblit(ggi_visual *src, int sx, int sy, int w, int h, 
			 ggi_visual *dst, int dx, int dy)
{
	/* Software clip so we don't read/write unused data. */
	LIBGGICLIP_COPYBOX(dst, sx, sy, w, h, dx, dy);

	if (src->r_frame && src->r_frame->layout == dst->w_frame->layout) {
		uint32_t srcformat
			= src->r_frame->buffer.plb.pixelformat->stdformat;

		PREPARE_FB(src);

		switch (srcformat) {
			case GGI_DB_STD_24a32p8r8g8b8:
			case GGI_DB_STD_24a32p8b8g8r8:
				dbblit_32bpp(src, sx, sy, w, h, dst, dx, dy, srcformat);
				return 0;
		}
	}

	return MGA_G400_PRIV(dst)->crossblit(src, sx, sy, w, h, dst, dx, dy);
}
