/* $Id: 3dlabs_pm2.h,v 1.2 2006/01/01 09:18:17 cegger Exp $
******************************************************************************

   LibGGI - 3Dlabs Permedia2 acceleration for fbdev target

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

#ifndef _GGIFBDEV_3DLABS_PM2_H
#define _GGIFBDEV_3DLABS_PM2_H


#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>

#include "permedia2.h"
#include <ggi/internal/ggi_debug.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

/* Size of the font */
#define FWIDTH	8
#define FHEIGHT	8


struct _3dlabs_pm2_priv {
	uint32_t	pprod;
	int		bppalign;	/* bits per pixel */
	int		bppshift;	/* bits per pixel */

	ggi_pixel	oldfgcol;
	ggi_pixel	oldbgcol;
	ggi_coord	oldtl, oldbr;
	int		oldyadd;
	uint16_t	curopmode;
	uint16_t	origopmode;
	unsigned long	fontoffset;
	int		charadd;
	int		fifosize;
	uint8_t	       *font;	/* Pointer to font in main RAM. */
	uint32_t	mmio_len;
#if 0
	volatile uint32_t *dmaaddr;
	uint32_t	dma_len;
#endif
	ggifunc_crossblit	*crossblit;
};

#define PM2_PRIV(vis) ((struct _3dlabs_pm2_priv*)FBDEV_PRIV(vis)->accelpriv)

/* We need the above struct here. */
#include "mmio.h"

/* Cast values for insertion in registers */
#define RS16(val)	( (uint16_t)((int16_t)(val)))
#define RS18(val)	(((uint32_t)((int32_t)(val)))&0x003ffff)
#define RS22(val)	(((uint32_t)((int32_t)(val)))&0x03fffff)
#define RS24(val)	(((uint32_t)((int32_t)(val)))&0x0ffffff)
#define RS27(val)	(((uint32_t)((int32_t)(val)))&0x7ffffff)


/* Update GC components if needed */
static inline void
pm2_gcupdate(volatile uint8_t *mmioaddr, struct _3dlabs_pm2_priv *priv,
	     ggi_mode *mode, ggi_gc *gc, int yadd)
{
	int newfg = (gc->fg_color != priv->oldfgcol);
	int newbg = (gc->bg_color != priv->oldbgcol);
	int newclip = (yadd != priv->oldyadd) ||
		(gc->cliptl.x != priv->oldtl.x) ||
		(gc->clipbr.x != priv->oldbr.x) ||
		(gc->cliptl.y != priv->oldtl.y) ||
		(gc->clipbr.y != priv->oldbr.y);

	/* restore default value */
	pm2_waitfifo(mmioaddr, 1);
	pm2_out32(mmioaddr, priv->pprod, PM2R_FB_READ_MODE);

	if (! (newfg || newbg || newclip)) return;

	if (newfg) {
		pm2_setcol(mmioaddr, mode, gc->fg_color,
			PM2R_CONSTANT_COLOR);
		priv->oldfgcol = gc->fg_color;
	}
	if (newbg) {
#if 0
		pm2_setcol(mmioaddr, mode, gc->bg_color, BCOL);
#endif
		priv->oldbgcol = gc->bg_color;
	}
	if (newclip) {
		pm2_setclip(mmioaddr, gc);
		priv->oldyadd = yadd;
		priv->oldtl.x = gc->cliptl.x;
		priv->oldbr.x = gc->clipbr.x;
		priv->oldtl.y = gc->cliptl.y;
		priv->oldbr.y = gc->clipbr.y;
	}
}

ggifunc_getcharsize	GGI_3dlabs_pm2_getcharsize;
ggifunc_putc		GGI_3dlabs_pm2_putc;
ggifunc_puts		GGI_3dlabs_pm2_puts;
#if 0
ggifunc_putc		GGI_3dlabs_pm2_fastputc;
ggifunc_puts		GGI_3dlabs_pm2_fastputs;
#endif

ggifunc_drawhline	GGI_3dlabs_pm2_drawhline;
ggifunc_drawvline	GGI_3dlabs_pm2_drawvline;
#if 0
ggifunc_drawline	GGI_3dlabs_pm2_drawline;
#endif
ggifunc_drawbox		GGI_3dlabs_pm2_drawbox;
ggifunc_copybox		GGI_3dlabs_pm2_copybox;
ggifunc_fillscreen	GGI_3dlabs_pm2_fillscreen;
#if 0
ggifunc_crossblit	GGI_3dlabs_pm2_crossblit;
#endif

ggifunc_putbox		GGI_3dlabs_pm2_putbox;
ggifunc_puthline	GGI_3dlabs_pm2_puthline;
ggifunc_putvline	GGI_3dlabs_pm2_putvline;

ggifunc_getbox		GGI_3dlabs_pm2_getbox;
ggifunc_gethline	GGI_3dlabs_pm2_gethline;
ggifunc_getvline	GGI_3dlabs_pm2_getvline;

#endif /* _GGIFBDEV_3DLABS_PM2_H */
