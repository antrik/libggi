/* $Id: m2164w.h,v 1.4 2005/07/31 09:58:44 cegger Exp $
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

#ifndef _GGIFBDEV_MGA_2164W_H
#define _GGIFBDEV_MGA_2164W_H


#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>

#include "regs.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

/* Size of the font */
#define FWIDTH	8
#define FHEIGHT	8


struct m2164w_priv {
	uint32_t		dwgctl;
	ggi_pixel	oldfgcol;
	ggi_pixel	oldbgcol;
	ggi_coord	oldtl, oldbr;
	int		oldyadd;
	uint16_t		curopmode;
	uint16_t		origopmode;
	uint32_t		drawboxcmd;
	unsigned long	fontoffset;
	int		charadd;
	uint8_t	       *font;	/* Pointer to font in main RAM. */
	uint32_t		mmio_len;
	volatile uint32_t *dmaaddr;
	uint32_t		dma_len;
	ggifunc_crossblit	*crossblit;
};

#define M2164W_PRIV(vis) ((struct m2164w_priv*)FBDEV_PRIV(vis)->accelpriv)

/* We need the above struct here. */
#include "mmio.h"

/* Cast values for insertion in registers */
#define RS16(val)	( (uint16_t)((int16_t)(val)))
#define RS18(val)	(((uint32_t)((int32_t)(val)))&0x003ffff)
#define RS24(val)	(((uint32_t)((int32_t)(val)))&0x0ffffff)
#define RS27(val)	(((uint32_t)((int32_t)(val)))&0x7ffffff)


/* Update GC components if needed */
static inline void
mga_gcupdate(volatile uint8_t *mmioaddr, struct m2164w_priv *priv,
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
	
ggifunc_getcharsize	GGI_m2164w_getcharsize;
ggifunc_putc		GGI_m2164w_putc;
ggifunc_puts		GGI_m2164w_puts;
ggifunc_putc		GGI_m2164w_fastputc;
ggifunc_puts		GGI_m2164w_fastputs;

ggifunc_drawhline	GGI_m2164w_drawhline;
ggifunc_drawvline	GGI_m2164w_drawvline;
ggifunc_drawline	GGI_m2164w_drawline;
ggifunc_drawbox		GGI_m2164w_drawbox;
ggifunc_copybox		GGI_m2164w_copybox;
ggifunc_fillscreen	GGI_m2164w_fillscreen;
ggifunc_crossblit	GGI_m2164w_crossblit;

#endif /* _GGIFBDEV_MGA_2164W_H */
