/* $Id: visual.c,v 1.5 2004/02/23 14:24:41 pekberg Exp $
******************************************************************************

   LibGGI - fbdev mga2164w acceleration

   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "m2164w.h"
#include <string.h>
#include <errno.h>

/* The default LibGGI font */
#include <ggi/internal/font/8x8>


static int m2164w_acquire(ggi_resource *res, uint32 actype)
{
	ggi_visual *vis;

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}

	res->count++;
	res->curactype |= actype;
	if (res->count > 1) return 0;

	vis = res->priv;
	LIBGGIIdleAccel(vis);

	return 0;
}

static int m2164w_release(ggi_resource *res)
{
	if (res->count < 1) return GGI_ENOTALLOC;

	res->count--;
	if (res->count == 0) {
		res->curactype = 0;
	}

	return 0;
}

static int
m2164w_idleaccel(ggi_visual *vis)
{
	GGIDPRINT_DRAW("m2164w_idleaccel(%p) called \n", vis);

	mga_waitidle(FBDEV_PRIV(vis)->mmioaddr);
	
	vis->accelactive = 0;

	return 0;
}


static int do_cleanup(ggi_visual *vis)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct m2164w_priv *priv = NULL;
	int i;

	GGIDPRINT_MISC("mga-2164w: Starting cleanup\n");

	if (fbdevpriv != NULL) {
		priv = M2164W_PRIV(vis);
	}

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

	/* Restore OPMODE and terminate any pending DMA operations
	   Manual says we should write to byte 0 to terminate DMA sequence,
	   but it doesn't say whether a 8 bit access is required, or if any
	   access will do. We play it safe here... */
	mga_out8(fbdevpriv->mmioaddr, priv->origopmode & 0xff, OPMODE);
	mga_out16(fbdevpriv->mmioaddr, priv->origopmode, OPMODE);
	mga_waitidle(fbdevpriv->mmioaddr);

	munmap((void*)fbdevpriv->mmioaddr, fbdevpriv->orig_fix.mmio_len);
	GGIDPRINT_MISC("mga-2164w: Unmapped MMIO\n");

	/* Free DB resource structures */
	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i]->resource) {
			free(LIBGGI_APPBUFS(vis)[i]->resource);
			LIBGGI_APPBUFS(vis)[i]->resource = NULL;
		}
	}

	free(priv);
	M2164W_PRIV(vis) = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	return 0;
}
	

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct m2164w_priv *priv;
	unsigned long usedmemend;
	size_t fontlen;
	int pixbytes;
	int fd = LIBGGI_FD(vis);
	int i;

	if (GT_SIZE(LIBGGI_GT(vis)) % 8 != 0 ||
	    GT_SIZE(LIBGGI_GT(vis)) > 32 ||
	    GT_SIZE(LIBGGI_GT(vis)) < 8) {
		/* Unsupported mode */
		return GGI_ENOFUNC;
	}
	pixbytes = GT_SIZE(LIBGGI_GT(vis)) / 8;

	priv = malloc(sizeof(struct m2164w_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}

	fbdevpriv->mmioaddr = mmap(NULL, fbdevpriv->orig_fix.mmio_len,
				   PROT_READ | PROT_WRITE, MAP_SHARED,
				   fd, (signed)fbdevpriv->orig_fix.smem_len);
	if (fbdevpriv->mmioaddr == MAP_FAILED) {
		/* Can't mmap() MMIO region - bail out */
		GGIDPRINT_LIBS("mga-2164w: Unable to map MMIO region: %s\n"
			       "          fd: %d, len: %ld, offset: %ld\n",
			       strerror(errno), fd,
			       fbdevpriv->orig_fix.mmio_len,
			       fbdevpriv->orig_fix.smem_len);
		fbdevpriv->mmioaddr = NULL;
		free(priv);
		return GGI_ENODEVICE;
	}

	GGIDPRINT_MISC("mga-2164w: Mapped MMIO region at %p\n",
		       fbdevpriv->mmioaddr);

	/* Set up DirectBuffers */
	for (i=0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf = LIBGGI_APPBUFS(vis)[i];
		ggi_resource *res;
		
		res = malloc(sizeof(ggi_resource));
		if (res == NULL) {
			do_cleanup(vis);
			return GGI_ENOMEM;
		}
		buf->resource = res;
		buf->resource->acquire = m2164w_acquire;
		buf->resource->release = m2164w_release;
		buf->resource->self = buf;
		buf->resource->priv = vis;
		buf->resource->count = 0;
		buf->resource->curactype = 0;
	}

	priv->drawboxcmd
		= BOP_COPY | SHFTZERO | SGNZERO | ARZERO | SOLID | OP_TRAP;
	if (pixbytes != 3) {
		switch (fbdevpriv->orig_fix.accel) {	
		case FB_ACCEL_MATROX_MGA2064W:
		case FB_ACCEL_MATROX_MGA1064SG:
		case FB_ACCEL_MATROX_MGA2164W:
		case FB_ACCEL_MATROX_MGA2164W_AGP:
			/* Use block mode */
			priv->drawboxcmd |= ATYPE_BLK;
			break;
		default:
			/* For now - assume SDRAM for other cards */
			break;
		}
	}
	priv->dwgctl = 0;
	priv->oldfgcol = LIBGGI_GC(vis)->fg_color - 1;
	priv->oldbgcol = LIBGGI_GC(vis)->bg_color - 1;
	priv->oldtl.x = -1;
	priv->oldtl.y = -1;
	priv->oldbr.x = -1;
	priv->oldbr.y = -1;
	priv->oldyadd = -1;
	priv->curopmode = priv->origopmode = mga_in16(fbdevpriv->mmioaddr, OPMODE);
	/* Use the 7k Pseudo-DMA window */
	priv->dmaaddr = (void*)fbdevpriv->mmioaddr;
	priv->dma_len = 0x1c00;

	vis->needidleaccel = 1;
	fbdevpriv->idleaccel = m2164w_idleaccel;

	/* Accelerate fonts if possible */
	priv->font = (uint8 *)(font);
	usedmemend = LIBGGI_MODE(vis)->frames *
		fbdevpriv->fix.line_length * LIBGGI_MODE(vis)->virt.y;
	fontlen = 256*8;
	priv->fontoffset = fbdevpriv->orig_fix.smem_len - fontlen;
	priv->fontoffset &= ~127; /* Align */
	GGIDPRINT_MISC("mga-2164w: usedmemend: %ld, fontoffset: %ld\n",
		       usedmemend, priv->fontoffset);
	if (usedmemend <= priv->fontoffset) {
		memcpy((uint8*)fbdevpriv->fb_ptr + priv->fontoffset,
		       font, fontlen);
		priv->fontoffset *= 8; /* In bits */
		priv->charadd = FWIDTH*FHEIGHT;
		vis->opdraw->putc = GGI_m2164w_fastputc;
		vis->opdraw->puts = GGI_m2164w_fastputs;
		GGIDPRINT_MISC("mga-2164w: Using fast chars\n");
	} else {
		priv->fontoffset = 0;
		vis->opdraw->putc = GGI_m2164w_putc;
		vis->opdraw->puts = GGI_m2164w_puts;
		GGIDPRINT_MISC("mga-2164w: Using slow chars\n");
	}

	/* Save previous function pointers */
	priv->crossblit = vis->opdraw->crossblit;

	/* Initialize function pointers */
	vis->opdraw->getcharsize= GGI_m2164w_getcharsize;
	vis->opdraw->drawhline = GGI_m2164w_drawhline;
	vis->opdraw->drawvline = GGI_m2164w_drawvline;
	vis->opdraw->drawline = GGI_m2164w_drawline;
	vis->opdraw->drawbox = GGI_m2164w_drawbox;
	vis->opdraw->copybox = GGI_m2164w_copybox;
	vis->opdraw->fillscreen = GGI_m2164w_fillscreen;
	/* The crossblit in linear-* is faster on truecolor modes! */
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE ||
	    GT_SCHEME(LIBGGI_GT(vis)) == GT_STATIC_PALETTE) {
		vis->opdraw->crossblit = GGI_m2164w_crossblit;
	}

	M2164W_PRIV(vis) = priv;

	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_m2164w(int func, void **funcptr);

int GGIdl_m2164w(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
