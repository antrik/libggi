/* $Id: ati_mach64.h,v 1.7 2005/07/31 09:58:43 cegger Exp $
******************************************************************************

   LibGGI - ATI mach64 and rage pro acceleration for fbdev target

   Copyright (C) 2002 Daniel Mantione	[daniel.mantione@freepascal.org]

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

#ifndef _GGIFBDEV_ATI_MACH64_H
#define _GGIFBDEV_ATI_MACH64_H


#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>

#include "mach64.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

/* Size of the font */
#define FWIDTH	8
#define FHEIGHT	8


struct ati_mach64_priv {
	uint32_t regaddr[256];	/* This approach saves more code than it
				   costs memory */
	uint32_t regbase;
	uint32_t dp_src;
	uint32_t dst_cntl;
	unsigned long fontoffset;
	uint8_t *font;		/* Pointer to font in main RAM. */
	int charadd;
	uint8_t has_3d;		/* If true, reset 3d engine on init */
/*	uint32_t          dwgctl; */
	ggi_pixel oldfgcol;
	ggi_pixel oldbgcol;
	ggi_coord oldtl, oldbr;

#if 0
	int		oldyadd;
	uint16_t		curopmode;
	uint16_t		origopmode;
	uint32_t		drawboxcmd;
	uint32_t		mmio_len;
	volatile uint32_t *dmaaddr;
	uint32_t		dma_len;
	ggifunc_crossblit	*crossblit;
#endif
};

#define ATI_MACH64_PRIV(vis) ((struct ati_mach64_priv*)FBDEV_PRIV(vis)->accelpriv)
#define default_mix FRGD_MIX_S | BKGD_MIX_S

/* Cast values for insertion in registers */
#if 0
#define RS16(val)	( (uint16_t)((int16_t)(val)))
#define RS18(val)	(((uint32_t)((int32_t)(val)))&0x003ffff)
#define RS22(val)	(((uint32_t)((int32_t)(val)))&0x03fffff)
#define RS24(val)	(((uint32_t)((int32_t)(val)))&0x0ffffff)
#define RS27(val)	(((uint32_t)((int32_t)(val)))&0x7ffffff)
#endif

/* Update GC components if needed */
#if 0
static inline void
ati_gcupdate(volatile uint8_t *mmioaddr, struct mga_g400_priv *priv,
	     ggi_mode *mode, ggi_gc *gc, int virtx, int yadd)
{
	int newfg = (gc->fg_color != priv->oldfgcol);
	int newbg = (gc->bg_color != priv->oldbgcol);
	int newclip = (yadd != priv->oldyadd) ||
		(gc->cliptl.x != priv->oldtl.x) ||
		(gc->clipbr.x != priv->oldbr.x) ||
		(gc->cliptl.y != priv->oldtl.y) ||
		(gc->clipbr.y != priv->oldbr.y);

	if (! (newfg || newbg || newclip)) return;

	if (newfg) {
		mga_setcol(mmioaddr, mode, gc->fg_color, FCOL);
		priv->oldfgcol = gc->fg_color;
	}
	if (newbg) {
		mga_setcol(mmioaddr, mode, gc->bg_color, BCOL);
		priv->oldbgcol = gc->bg_color;
	}
	if (newclip) {
		mga_setclip(mmioaddr, gc, virtx, yadd);
		priv->oldyadd = yadd;
		priv->oldtl.x = gc->cliptl.x;
		priv->oldbr.x = gc->clipbr.x;
		priv->oldtl.y = gc->cliptl.y;
		priv->oldbr.y = gc->clipbr.y;
	}
}
#endif

ggifunc_getcharsize GGI_ati_mach64_getcharsize;
ggifunc_putc GGI_ati_mach64_putc;
ggifunc_puts GGI_ati_mach64_puts;
ggifunc_putc GGI_ati_mach64_fastputc;
ggifunc_puts GGI_ati_mach64_fastputs;

ggifunc_drawhline GGI_ati_mach64_drawhline;
ggifunc_drawvline GGI_ati_mach64_drawvline;
ggifunc_drawline GGI_ati_mach64_drawline;
ggifunc_drawbox GGI_ati_mach64_drawbox;
ggifunc_copybox GGI_ati_mach64_copybox;
ggifunc_fillscreen GGI_ati_mach64_fillscreen;
ggifunc_crossblit GGI_ati_mach64_crossblit;
ggifunc_gcchanged GGI_ati_mach64_gcchanged;


static inline uint32_t aty_ld_le32(int regindex,
				 const struct ati_mach64_priv *priv)
{
	/* Should be cleanly optimized by compiler */
	if (regindex >= 0x400) {
		regindex -= 0x400;
#warning FIXME: This is not 64bit safe
		return ((volatile uint32_t *) (priv->regbase))[regindex / 4];
	} else {
#warning FIXME: This is not 64bit safe
		return
		    *((volatile uint32_t *) (priv->regaddr[regindex / 4]));
	};
}

static inline void aty_st_le32(int regindex, uint32_t val,
			       const struct ati_mach64_priv *priv)
{
	/* Should be cleanly optimized by compiler */
	if (regindex >= 0x400) {
		regindex -= 0x400;
#warning FIXME: This is not 64bit safe
		((volatile uint32_t *) (priv->regbase))[regindex / 4] = val;
	} else {
#warning FIXME: This is not 64bit safe
		*((volatile uint32_t *) (priv->regaddr[regindex / 4])) = val;
	};
}

static inline void wait_for_fifo(uint16_t entries,
				 const struct ati_mach64_priv *priv)
{
	while ((aty_ld_le32(FIFO_STAT, priv) & 0xffff) >
	       ((uint32_t) (0x8000 >> entries)));
}

static inline void wait_for_idle(const struct ati_mach64_priv *priv)
{
	wait_for_fifo(16, priv);
	while ((aty_ld_le32(GUI_STAT, priv) & 1) != 0);
}

static inline void set_dp_src(struct ati_mach64_priv *priv, uint32_t value)
{
	if (priv->dp_src != value) {
		wait_for_fifo(1, priv);
		aty_st_le32(DP_SRC, value, priv);
		priv->dp_src = value;
	};
}

static inline void set_dst_cntl(struct ati_mach64_priv *priv, uint32_t value)
{
	if (priv->dst_cntl != value) {
		wait_for_fifo(1, priv);
		aty_st_le32(DST_CNTL, value, priv);
		priv->dst_cntl = value;
	};
}

static inline void draw_rect(struct ati_mach64_priv *priv,
			     int x, int y, int w, int h)
{
	set_dp_src(priv, FRGD_SRC_FRGD_CLR);
	set_dst_cntl(priv, /*DST_LAST_PEL| */
		     DST_Y_TOP_TO_BOTTOM | DST_X_LEFT_TO_RIGHT);
	wait_for_fifo(2, priv);
	aty_st_le32(DST_Y_X, (unsigned)(x << 16) | y, priv);
	aty_st_le32(DST_HEIGHT_WIDTH, (unsigned)(w << 16) | h, priv);
}

#endif	/* _GGIFBDEV_ATI_MACH64_H */
