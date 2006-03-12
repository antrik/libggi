/* $Id: visual.c,v 1.7 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   LibGGI - fbdev 3DLabs Permedia2 acceleration

   Copyright (C) 2005	Christoph Egger

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

#include "3dlabs_pm2.h"
#include <string.h>
#include <errno.h>

/* The default LibGGI font */
#include <ggi/internal/font/8x8>

static int _3dlabs_pm2_acquire(ggi_resource *res, uint32_t actype)
{
	struct ggi_visual *vis;

	DPRINT_DRAW("pm2_acquire(%p, %lu) entered\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		DPRINT_DRAW("pm2_acquire(): invalid args\n",
			res, actype);
		return GGI_EARGINVAL;
	}


	res->count++;
	res->curactype |= actype;
	if (res->count > 1) return 0;

	vis = res->priv;
	LIBGGIIdleAccel(vis);

	return 0;
}

static int _3dlabs_pm2_release(ggi_resource *res)
{
	DPRINT_DRAW("pm2_release(%p) entered\n", res);

	if (res->count < 1) return GGI_ENOTALLOC;

	res->count--;
	if (res->count == 0) {
		res->curactype = 0;
	}

	return 0;
}

static int _3dlabs_pm2_idleaccel(struct ggi_visual *vis)
{
	pm2_waitidle(FBDEV_PRIV(vis)->mmioaddr);
	
	vis->accelactive = 0;

	return 0;
}


static void pm2_initengine(volatile uint8_t *mmioaddr)
{
	/* Initialize the Accelerator Engine to defaults */

#define PM2_OUT(value, reg)			\
	do {					\
		pm2_waitidle(mmioaddr);		\
		pm2_out32(mmioaddr, (value), (reg));\
	} while(0)

	PM2_OUT(UNIT_DISABLE, PM2R_SCISSOR_MODE);
	PM2_OUT(UNIT_ENABLE, PM2R_FB_WRITE_MODE);
	PM2_OUT(0, PM2R_D_X_SUB);
#if 0
	PM2_OUT(GWIN_DisableLBUpdate, PM2R_WINDOW);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_DITHER_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_ALPHA_BLEND_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_COLOR_DDA_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_TEXTURE_COLOR_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_TEXTURE_ADDRESS_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_TEXTURE_READ_MODE);
#if 0
	PM2_OUT(, PM2R_LB_READ_MODE);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_TEXEL_LUT_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_YUV_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_DEPTH_MODE);
#if 0
	PM2_OUT(UNIT_DISABLE, PM2R_ROUTER_MODE);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_FOG_MODE);
#if 0
	PM2_OUT(UNIT_DISABLE, PM2R_ANTIALIAS_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_ALPHA_TEST_MODE);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_STENCIL_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_AREA_STIPPLE_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_LOGICAL_OP_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_DEPTH_MODE);
#if 0
	PM2_OUT(UNIT_DISABLE, PM2R_STATISTIC_MODE);
#endif
	PM2_OUT(0x400, PM2R_FILTER_MODE);
#if 0
	PM2_OUT(0xffffffff,   PM2R_FB_HARDWARE_WRITE_MASK);
	PM2_OUT(0xffffffff,   PM2R_FB_SOFTWARE_WRITE_MASK);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_RASTERIZER_MODE);
	PM2_OUT(UNIT_DISABLE, PM2R_FB_SOURCE_OFFSET);
	PM2_OUT(UNIT_DISABLE, PM2R_FB_PIXEL_OFFSET);
	PM2_OUT(UNIT_DISABLE, PM2R_LB_SOURCE_OFFSET);
	PM2_OUT(UNIT_DISABLE, PM2R_WINDOW_ORIGIN);
	PM2_OUT(UNIT_DISABLE, PM2R_FB_WINDOW_BASE);
#if 0
	PM2_OUT(UNIT_DISABLE, PM2R_FB_SOURCE_BASE);
#endif
	PM2_OUT(UNIT_DISABLE, PM2R_LB_WINDOW_BASE);

#if 0
	PM2_OUT(, PM2R_FB_READ_PIXEL);
	PM2_OUT(, PM2R_FB_TEXTURE_MAP_FORMAT);
#endif
	PM2_OUT(0, PM2R_RECTANGLE_SIZE);
	PM2_OUT(0, PM2R_RECTANGLE_ORIGIN);
	PM2_OUT(0, PM2R_D_X_DOM);
	PM2_OUT(1<<16, PM2R_D_Y);
	PM2_OUT(0, PM2R_START_X_DOM);
	PM2_OUT(0, PM2R_START_Y);
	PM2_OUT(0, PM2R_COUNT);

#undef PM2_OUT
}


static int do_cleanup(struct ggi_visual *vis)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct _3dlabs_pm2_priv *priv = NULL;
	int i;

	DPRINT_MISC("3dlabs_pm2: Starting cleanup\n");

	if (fbdevpriv != NULL) {
		priv = PM2_PRIV(vis);
	}

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

#if 0
	/* Fix up DSTORG register in case someone messed with it - fbdev
	 * really dislikes that being wrong.
	 */
	mga_waitfifo(fbdevpriv->mmioaddr, 2);
	mga_out32(fbdevpriv->mmioaddr, 0, DSTORG);
	mga_out32(fbdevpriv->mmioaddr, 0, SRCORG);

	/* Restore OPMODE and terminate any pending DMA operations
	   Manual says we should write to byte 0 to terminate DMA sequence,
	   but it doesn't say whether a 8 bit access is required, or if any
	   access will do. We play it safe here... */
	mga_out8(fbdevpriv->mmioaddr, priv->origopmode&0xff, OPMODE);
	mga_out16(fbdevpriv->mmioaddr, priv->origopmode, OPMODE);
#endif
	pm2_waitidle(fbdevpriv->mmioaddr);
	pm2_initengine(fbdevpriv->mmioaddr);
	pm2_waitidle(fbdevpriv->mmioaddr);

	munmap((void *)fbdevpriv->mmioaddr, fbdevpriv->orig_fix.mmio_len);
	DPRINT_MISC("3dlabs_pm2: Unmapped MMIO\n");

	/* Free DB resource structures */
	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i]->resource) {
			free(LIBGGI_APPBUFS(vis)[i]->resource);
			LIBGGI_APPBUFS(vis)[i]->resource = NULL;
		}
	}

	free(priv);
	FBDEV_PRIV(vis)->accelpriv = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	return 0;
}


#define PARTPROD(a,b,c)	(((a) << 6) | ((b) << 3) | (c))

static char bppand[4] = { 0x03, /* 8bpp */
			  0x01, /* 16bpp */
			  0x00, /* 24bpp */
			  0x00  /* 32bpp */
			};

static int partprodPermedia[] = {
	-1,
	PARTPROD(0,0,1), PARTPROD(0,1,1), PARTPROD(1,1,1), PARTPROD(1,1,2),
	PARTPROD(1,2,2), PARTPROD(2,2,2), PARTPROD(1,2,3), PARTPROD(2,2,3),
	PARTPROD(1,3,3), PARTPROD(2,3,3), PARTPROD(1,2,4), PARTPROD(3,3,3),
	PARTPROD(1,3,4), PARTPROD(2,3,4), 	       -1, PARTPROD(3,3,4),
	PARTPROD(1,4,4), PARTPROD(2,4,4), 	       -1, PARTPROD(3,4,4),
		     -1, PARTPROD(2,3,5), 	       -1, PARTPROD(4,4,4),
	PARTPROD(1,4,5), PARTPROD(2,4,5), PARTPROD(3,4,5), 		-1,
		     -1, 	      -1, 	       -1, PARTPROD(4,4,5),
	PARTPROD(1,5,5), PARTPROD(2,5,5), 	       -1, PARTPROD(3,5,5),
		     -1, 	      -1, 	       -1, PARTPROD(4,5,5),
		     -1, 	      -1, 	       -1, PARTPROD(3,4,6),
		     -1, 	      -1, 	       -1, PARTPROD(5,5,5),
	PARTPROD(1,5,6), PARTPROD(2,5,6), 	       -1, PARTPROD(3,5,6),
		     -1, 	      -1, 	       -1, PARTPROD(4,5,6),
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, PARTPROD(5,5,6),
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		     -1, 	      -1, 	       -1, 		-1,
		       0
};


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct _3dlabs_pm2_priv *priv;
	unsigned long usedmemend;
	size_t fontlen;
	int pixbytes;
	int fd = LIBGGI_FD(vis);
	int i;

	if (GT_SIZE(LIBGGI_GT(vis)) % 8 != 0 ||
	    GT_SIZE(LIBGGI_GT(vis)) > 32 ||
	    GT_SIZE(LIBGGI_GT(vis)) < 8)
	{
		/* Unsupported mode */
		return GGI_ENOFUNC;
	}
	if ((LIBGGI_VIRTX(vis) > 2048)
	   || (LIBGGI_VIRTY(vis) > 2048))
	{
		/* Unsupported mode */
		return GGI_ENOFUNC;
	}

	pixbytes = GT_ByPP(LIBGGI_GT(vis));

	priv = malloc(sizeof(struct _3dlabs_pm2_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}

	fbdevpriv->mmioaddr = mmap(NULL, fbdevpriv->orig_fix.mmio_len,
				   PROT_READ | PROT_WRITE, MAP_SHARED,
				   fd, (signed)fbdevpriv->orig_fix.smem_len);
	if (fbdevpriv->mmioaddr == MAP_FAILED) {
		/* Can't mmap() MMIO region - bail out */
		DPRINT_LIBS("_3dlabs_pm2: Unable to map MMIO region: %s\n"
			       "          fd: %d, len: %ld, offset: %ld\n",
			       strerror(errno), fd,
			       fbdevpriv->orig_fix.mmio_len,
			       fbdevpriv->orig_fix.smem_len);
		fbdevpriv->mmioaddr = NULL;
		free(priv);
		return GGI_ENODEVICE;
	}

	DPRINT_MISC("_3dlabs_pm2: Mapped MMIO region at %p\n",
		       fbdevpriv->mmioaddr);

	pm2_waitidle(fbdevpriv->mmioaddr);
	pm2_initengine(fbdevpriv->mmioaddr);
	pm2_waitidle(fbdevpriv->mmioaddr);


	priv->pprod = partprodPermedia[LIBGGI_VIRTX(vis) >> 5];
	priv->bppalign = bppand[(LIBGGI_VIRTX(vis) >> 3)-1];
	priv->fifosize = 256;	/* don't copy!! Only valid for Permedia 2/2V */

	switch (GT_DEPTH(LIBGGI_GT(vis))) {
	case 8:
		priv->bppshift = 2;
		break;
	case 16:
		if (LIBGGI_MODE(vis)->frames == 2) {
			priv->bppshift = 0;	
		} else {
			priv->bppshift = 1;
		}
		break;
	case 24:
		priv->bppshift = 2;
		break;
	case 32:
		priv->bppshift = 0;
		break;
	}

	DPRINT_MISC("_3dlabs_pm2: Setting up DirectBuffers\n");

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
		buf->resource->acquire = _3dlabs_pm2_acquire;
		buf->resource->release = _3dlabs_pm2_release;
		buf->resource->self = buf;
		buf->resource->priv = vis;
		buf->resource->count = 0;
		buf->resource->curactype = 0;
	}

	priv->oldfgcol = LIBGGI_GC(vis)->fg_color - 1;
	priv->oldbgcol = LIBGGI_GC(vis)->bg_color - 1;
	priv->oldtl.x = -1;
	priv->oldtl.y = -1;
	priv->oldbr.x = -1;
	priv->oldbr.y = -1;
	priv->oldyadd = -1;
#if 0
	priv->curopmode = priv->origopmode = mga_in16(fbdevpriv->mmioaddr, OPMODE);
	/* Use the 7k Pseudo-DMA window */
	priv->dmaaddr = (void*)fbdevpriv->mmioaddr;
	priv->dma_len = 0x1c00;
#endif

	vis->needidleaccel = 1;
	fbdevpriv->idleaccel = _3dlabs_pm2_idleaccel;

	/* Accelerate fonts if possible */
	priv->font = (uint8_t *)(font);
	usedmemend = LIBGGI_MODE(vis)->frames *
		fbdevpriv->fix.line_length * LIBGGI_VIRTY(vis);
	fontlen = 256*8;
	priv->fontoffset = fbdevpriv->orig_fix.smem_len - fontlen;
	priv->fontoffset &= ~127; /* Align */
	DPRINT_MISC("3dlabs_pm2: usedmemend: %ld, fontoffset: %ld\n",
			usedmemend, priv->fontoffset);
#if 0
	if (usedmemend <= priv->fontoffset) {
		memcpy((uint8_t*)fbdevpriv->fb_ptr + priv->fontoffset,
			font, fontlen);
		priv->fontoffset *= 8; /* In bits */
		priv->charadd = FWIDTH*FHEIGHT;
		vis->opdraw->putc = GGI_3dlabs_pm2_fastputc;
		vis->opdraw->puts = GGI_3dlabs_pm2_fastputs;
		DPRINT_MISC("3dlabs_pm2: Using fast chars\n");
	} else {
#endif
		priv->fontoffset = 0;
		vis->opdraw->putc = GGI_3dlabs_pm2_putc;
		vis->opdraw->puts = GGI_3dlabs_pm2_puts;
		DPRINT_MISC("3dlabs_pm2: Using slow chars\n");
#if 0
	}
#endif

	/* Save previous function pointers */
	priv->crossblit = vis->opdraw->crossblit;

	/* Initialize function pointers */
	vis->opdraw->getcharsize= GGI_3dlabs_pm2_getcharsize;
	vis->opdraw->drawhline = GGI_3dlabs_pm2_drawhline;
	vis->opdraw->drawvline = GGI_3dlabs_pm2_drawvline;
#if 0
	vis->opdraw->drawline = GGI_3dlabs_pm2_drawline;
#endif
	vis->opdraw->drawbox = GGI_3dlabs_pm2_drawbox;
	vis->opdraw->copybox = GGI_3dlabs_pm2_copybox;
	vis->opdraw->fillscreen = GGI_3dlabs_pm2_fillscreen;

	/* ggiPutBox only works on 32bit modes so far */
	if (GT_SIZE(LIBGGI_GT(vis)) == 32) {
		vis->opdraw->putbox = GGI_3dlabs_pm2_putbox;
		vis->opdraw->puthline = GGI_3dlabs_pm2_puthline;
		vis->opdraw->putvline = GGI_3dlabs_pm2_putvline;
	}

#if 0
	vis->opdraw->getbox = GGI_3dlabs_pm2_getbox;
	vis->opdraw->gethline = GGI_3dlabs_pm2_gethline;
	vis->opdraw->getvline = GGI_3dlabs_pm2_getvline;
#endif


	FBDEV_PRIV(vis)->accelpriv = priv;

	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_fbdev_3dlabs_pm2(int func, void **funcptr);

int GGIdl_fbdev_3dlabs_pm2(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
