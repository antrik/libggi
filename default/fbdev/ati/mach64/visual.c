/* $Id: visual.c,v 1.6 2004/02/23 14:24:41 pekberg Exp $
******************************************************************************

   LibGGI - fbdev ATi Mach64 and Rage Pro acceleration

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

#include "ati_mach64.h"
#include <string.h>
#include <errno.h>

/* The default LibGGI font */
#include <ggi/internal/font/8x8>

    /*
     *  Generic Mach64 routines
     */

static void ati_mach64_reset_engine(const struct ati_mach64_priv *priv)
{
    /* reset engine */
    aty_st_le32(GEN_TEST_CNTL,
                aty_ld_le32(GEN_TEST_CNTL, priv) & ~GUI_ENGINE_ENABLE, priv);
    /* enable engine */
    aty_st_le32(GEN_TEST_CNTL,
                aty_ld_le32(GEN_TEST_CNTL, priv) | GUI_ENGINE_ENABLE, priv);
    /* ensure engine is not locked up by clearing any FIFO or */
    /* HOST errors */
    aty_st_le32(BUS_CNTL, aty_ld_le32(BUS_CNTL, priv) | BUS_HOST_ERR_ACK |
                          BUS_FIFO_ERR_ACK, priv);
}

static void
ati_mach64_reset_GTC_3D_engine(const struct ati_mach64_priv *priv)
{
        aty_st_le32(SCALE_3D_CNTL, 0xc0, priv);
        usleep(GTC_3D_RESET_DELAY);
        aty_st_le32(SETUP_CNTL, 0x00, priv);
        usleep(GTC_3D_RESET_DELAY);
        aty_st_le32(SCALE_3D_CNTL, 0x00, priv);
        usleep(GTC_3D_RESET_DELAY);
}

static void ati_mach64_init_engine(ggi_visual *vis)
{
    uint32 pitch_value;
    int bpp,depth;
    struct ati_mach64_priv *priv;

    priv=ATI_MACH64_PRIV(vis);
    bpp=GT_SIZE(LIBGGI_MODE(vis)->graphtype);
    depth=GT_DEPTH(LIBGGI_MODE(vis)->graphtype);

    /* On GTC (RagePro), we need to reset the 3D engine before */
    if (priv->has_3d)
        ati_mach64_reset_GTC_3D_engine(priv);

    /* Reset engine, enable, and clear any engine errors */
    ati_mach64_reset_engine(priv);
    /* Ensure that vga page pointers are set to zero - the upper */
    /* page pointers are set to 1 to handle overflows in the */
    /* lower page */
//    aty_st_le32(MEM_VGA_WP_SEL, 0x00010000, priv);
//    aty_st_le32(MEM_VGA_RP_SEL, 0x00010000, priv);

    /* ---- Setup standard engine context ---- */

    /* All GUI registers here are FIFOed - therefore, wait for */
    /* the appropriate number of empty FIFO entries */
    wait_for_fifo(16, priv);

    /* enable all registers to be loaded for context loads */
    aty_st_le32(CONTEXT_MASK, 0xFFFFFFFF, priv);

    /* determine modal privrmation from global mode structure */
    pitch_value = LIBGGI_VIRTX(vis);

    if (bpp == 24) {
        /* In 24 bpp, the engine is in 8 bpp - this requires that all */
        /* horizontal coordinates and widths must be adjusted */
        pitch_value = pitch_value * 3;
    } else if (bpp == 0) {
        pitch_value = pitch_value / 8;
    };

    /* set destination pitch to modal pitch, set offset to zero */
    aty_st_le32(DST_OFF_PITCH, (pitch_value / 8) << 22, priv);

    /* zero these registers (set them to a known state) */
    aty_st_le32(DST_Y_X, 0, priv);
    aty_st_le32(DST_HEIGHT, 0, priv);
    aty_st_le32(DST_BRES_ERR, 0, priv);
    aty_st_le32(DST_BRES_INC, 0, priv);
    aty_st_le32(DST_BRES_DEC, 0, priv);

    priv->dst_cntl=DST_LAST_PEL | DST_Y_TOP_TO_BOTTOM | DST_X_LEFT_TO_RIGHT;
    /* set destination drawing attributes */
    aty_st_le32(DST_CNTL, priv->dst_cntl, priv);

    /* set source pitch to modal pitch, set offset to zero */
    aty_st_le32(SRC_OFF_PITCH, (pitch_value / 8) << 22, priv);

    /* set these registers to a known state */
    aty_st_le32(SRC_Y_X, 0, priv);
    aty_st_le32(SRC_HEIGHT1_WIDTH1, 1, priv);
    aty_st_le32(SRC_Y_X_START, 0, priv);
    aty_st_le32(SRC_HEIGHT2_WIDTH2, 1, priv);

    /* set source pixel retrieving attributes */
    aty_st_le32(SRC_CNTL, SRC_LINE_X_LEFT_TO_RIGHT, priv);

    if (bpp == 0)
        /* set write mask to effect only the lower 16 bits, because
           the lower 16 bits are the characters and the higer 16 bits
           is the font */
        aty_st_le32(DP_WRITE_MASK, 0x0000FFFF, priv);
    else
        /* set write mask to effect all pixel bits */
        aty_st_le32(DP_WRITE_MASK, 0xFFFFFFFF, priv);

    /* Set mix register */
    aty_st_le32(DP_MIX, default_mix, priv);

    /* set primary source pixel channel to foreground color */
    /* register */
    priv->dp_src=FRGD_SRC_FRGD_CLR;
    aty_st_le32(DP_SRC, priv->dp_src, priv);

    /* set compare functionality to false (no-effect on */
    /* destination) */
    wait_for_fifo(3, priv);
    aty_st_le32(CLR_CMP_CLR, 0, priv);
    aty_st_le32(CLR_CMP_MASK, 0xFFFFFFFF, priv);
    aty_st_le32(CLR_CMP_CNTL, 0, priv);

    /* set pixel depth */
    wait_for_fifo(2, priv);
    GGIDPRINT_MISC("DEPTH: %x BPP: %x\n",depth,bpp);
    switch (depth) {
	case 0:
	    aty_st_le32(DP_PIX_WIDTH, HOST_32BPP|SRC_32BPP|DST_32BPP|BYTE_ORDER_MSB_TO_LSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x8080, priv);
	    break;
	case 4:
	    aty_st_le32(DP_PIX_WIDTH, HOST_1BPP|SRC_4BPP|DST_4BPP|BYTE_ORDER_LSB_TO_MSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x8888, priv);
	    break;
	case 8:
	    aty_st_le32(DP_PIX_WIDTH, HOST_1BPP|SRC_8BPP|DST_8BPP|BYTE_ORDER_MSB_TO_LSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x8080, priv);
	    break;
	case 16:
	    aty_st_le32(DP_PIX_WIDTH, HOST_1BPP|SRC_16BPP|DST_16BPP|BYTE_ORDER_MSB_TO_LSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x4210, priv);
	    break;
	case 24:
	    aty_st_le32(DP_PIX_WIDTH, HOST_1BPP|SRC_8BPP|DST_8BPP|BYTE_ORDER_MSB_TO_LSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x8080, priv);
	    break;
	case 32:
	    aty_st_le32(DP_PIX_WIDTH, HOST_1BPP|SRC_32BPP|DST_32BPP|BYTE_ORDER_MSB_TO_LSB, priv);
	    aty_st_le32(DP_CHAIN_MASK, 0x8080, priv);
	    break;
    };

    wait_for_fifo(4, priv);
    aty_st_le32(SCALE_3D_CNTL, 0, priv);
    aty_st_le32(Z_CNTL, 0, priv);
    aty_st_le32(CRTC_INT_CNTL, aty_ld_le32(CRTC_INT_CNTL, priv) & ~0x20U, priv);
    aty_st_le32(GUI_TRAJ_CNTL, 0x100023, priv);

    /* Set fifo size to 192. */
    wait_for_idle(priv);
    aty_st_le32(GUI_CNTL, 0, priv);

    /* insure engine is idle before leaving */
    wait_for_idle(priv);
}

static int ati_mach64_acquire(ggi_resource *res, uint32 actype)
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

static int ati_mach64_release(ggi_resource *res)
{
	if (res->count < 1) return GGI_ENOTALLOC;

	res->count--;
	if (res->count == 0) {
		res->curactype = 0;
	}

	return 0;
}

static int ati_mach64_idleaccel(ggi_visual *vis)
{
	GGIDPRINT_DRAW("ati_mach64_idleaccel(%p) called \n", vis);

//	mga_waitidle(FBDEV_PRIV(vis)->mmioaddr);
	wait_for_idle(ATI_MACH64_PRIV(vis));
	
	vis->accelactive = 0;

	return 0;
}


static int do_cleanup(ggi_visual *vis)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct ati_mach64_priv *priv = NULL;

	GGIDPRINT_MISC("ati_mach64: Starting cleanup\n");

	if (fbdevpriv != NULL) {
		priv = ATI_MACH64_PRIV(vis);
	}

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;

	/* Fix up DSTORG register in case someone messed with it - fbdev
	 * really dislikes that being wrong.
	 */
/*	mga_waitfifo(fbdevpriv->mmioaddr, 2);
	mga_out32(fbdevpriv->mmioaddr, 0, DSTORG);
	mga_out32(fbdevpriv->mmioaddr, 0, SRCORG);*/

	/* Restore OPMODE and terminate any pending DMA operations
	   Manual says we should write to byte 0 to terminate DMA sequence,
	   but it doesn't say whether a 8 bit access is required, or if any
	   access will do. We play it safe here... */
/*	mga_out8(fbdevpriv->mmioaddr, priv->origopmode&0xff, OPMODE);
	mga_out16(fbdevpriv->mmioaddr, priv->origopmode, OPMODE);
	mga_waitidle(fbdevpriv->mmioaddr);*/

	munmap((void*)fbdevpriv->mmioaddr, fbdevpriv->orig_fix.mmio_len);
	GGIDPRINT_MISC("ati_mach64: Unmapped MMIO\n");

	/* Free DB resource structures */
/*	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i]->resource) {
			free(LIBGGI_APPBUFS(vis)[i]->resource);
			LIBGGI_APPBUFS(vis)[i]->resource = NULL;
		}
	}*/

	free(priv);
	ATI_MACH64_PRIV(vis) = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	return 0;
}
	

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct ati_mach64_priv *priv;
	unsigned long usedmemend;
	size_t fontlen;
	int pixbytes;
	int fd = LIBGGI_FD(vis);
	int i;
	uint32 addr;

	if (GT_SIZE(LIBGGI_GT(vis)) % 8 != 0 ||
	    GT_SIZE(LIBGGI_GT(vis)) > 32 ||
	    GT_SIZE(LIBGGI_GT(vis)) < 8) {
		/* Unsupported mode */
		return GGI_ENOFUNC;
	}
	pixbytes = GT_SIZE(LIBGGI_GT(vis)) / 8;

	priv = malloc(sizeof(struct ati_mach64_priv));
	if (priv == NULL) {
		return GGI_ENOMEM;
	}
	ATI_MACH64_PRIV(vis) = priv;


	fbdevpriv->mmioaddr = mmap(NULL, fbdevpriv->orig_fix.mmio_len,
				   PROT_READ | PROT_WRITE, MAP_SHARED,
				   fd, (signed)fbdevpriv->orig_fix.smem_len);
	if (fbdevpriv->mmioaddr == MAP_FAILED) {
		/* Can't mmap() MMIO region - bail out */
		GGIDPRINT_LIBS("ati-mach64: Unable to map MMIO region: %s\n"
			       "            fd: %d, len: %ld, offset: %ld\n",
			       strerror(errno), fd,
			       fbdevpriv->orig_fix.mmio_len,
			       fbdevpriv->orig_fix.smem_len);
		fbdevpriv->mmioaddr = NULL;
		free(priv);
		return GGI_ENODEVICE;
	}

	GGIDPRINT_MISC("ati-mach64: Mapped MMIO region at %p\n",
		       fbdevpriv->mmioaddr);
#warning Not 64bit safe: Cast from pointer to integer of different size
	priv->regbase = (uint32)fbdevpriv->mmioaddr;
	addr = priv->regbase + 0x400;
	for (i=0;i<256;i++) {
	    priv->regaddr[i]=addr;
	    addr+=sizeof(uint32);
	};

	priv->has_3d = (fbdevpriv->orig_fix.accel == FB_ACCEL_ATI_MACH64GT);

	/* Initialize the 2D engine */
	GGIDPRINT_MISC("ati-mach64: Initializing 2D engine.\n");
	ati_mach64_init_engine(vis);
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
		buf->resource->acquire = ati_mach64_acquire;
		buf->resource->release = ati_mach64_release;
		buf->resource->self = buf;
		buf->resource->priv = vis;
		buf->resource->count = 0;
		buf->resource->curactype = 0;
	}
#if 0
	priv->drawboxcmd
		= BOP_COPY | SHFTZERO | SGNZERO | ARZERO | SOLID | OP_TRAP;
	if (pixbytes != 3) {
		switch (fbdevpriv->orig_fix.accel) {	
    		case  FB_ACCEL_ATI_MACH64GX:
    		case  FB_ACCEL_ATI_MACH64GX:
    		case  FB_ACCEL_ATI_MACH64GX:
    		case  FB_ACCEL_ATI_MACH64GX:
			* Use block mode *
			priv->drawboxcmd |= ATYPE_BLK;
			break;
		default:
			* For now - assume SDRAM for other cards *
			break;
		}
	}
#endif
	priv->oldfgcol = LIBGGI_GC(vis)->fg_color - 1;
	priv->oldbgcol = LIBGGI_GC(vis)->bg_color - 1;
	priv->oldtl.x = -1;
	priv->oldtl.y = -1;
	priv->oldbr.x = -1;
	priv->oldbr.y = -1;

#if 0
	priv->oldyadd = -1;
	priv->curopmode = priv->origopmode = mga_in16(fbdevpriv->mmioaddr, OPMODE);
	* Use the 7k Pseudo-DMA window *
	priv->dmaaddr = (void*)fbdevpriv->mmioaddr;
	priv->dma_len = 0x1c00;
#endif

	fbdevpriv->idleaccel = ati_mach64_idleaccel;

	/* Accelerate fonts if possible */
	priv->font = (uint8 *)(font);
	usedmemend = LIBGGI_MODE(vis)->frames *
		fbdevpriv->fix.line_length * LIBGGI_MODE(vis)->virt.y;
	fontlen = 256*8;
	priv->fontoffset = fbdevpriv->orig_fix.smem_len - fontlen;
	priv->fontoffset &= ~7; /* Align */
	GGIDPRINT_MISC("ati-mach64: usedmemend: %ld, fontoffset: %ld\n",
		       usedmemend, priv->fontoffset);
	if (usedmemend <= priv->fontoffset) {
		memcpy((uint8*)fbdevpriv->fb_ptr + priv->fontoffset,
		       font, fontlen);
		priv->charadd = FWIDTH*FHEIGHT;
		vis->opdraw->putc = GGI_ati_mach64_fastputc;
		vis->opdraw->puts = GGI_ati_mach64_fastputs;
		GGIDPRINT_MISC("ati-mach64: Using fast chars\n");
	} else {
		priv->fontoffset = 0;
		vis->opdraw->putc = GGI_ati_mach64_putc;
		vis->opdraw->puts = GGI_ati_mach64_puts;
		GGIDPRINT_MISC("ati-mach64: Using slow chars\n");
	}

	/* Save previous function pointers *
	priv->crossblit = vis->opdraw->crossblit;*/

	/* Initialize function pointers */
	vis->opdraw->getcharsize= GGI_ati_mach64_getcharsize;
	vis->opdraw->drawhline = GGI_ati_mach64_drawhline;
	vis->opdraw->drawvline = GGI_ati_mach64_drawvline;
	vis->opdraw->drawline = GGI_ati_mach64_drawline;
	vis->opdraw->drawbox = GGI_ati_mach64_drawbox;
	vis->opdraw->copybox = GGI_ati_mach64_copybox;
	vis->opdraw->fillscreen = GGI_ati_mach64_fillscreen;
	vis->opgc->gcchanged = GGI_ati_mach64_gcchanged;
	/* The crossblit in linear-* is faster on truecolor modes! */
	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE ||
	    GT_SCHEME(LIBGGI_GT(vis)) == GT_STATIC_PALETTE) {
		//vis->opdraw->crossblit = GGI_ati_mach64_crossblit;
	}

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
int GGIdl_mach64(int func, void **funcptr);

int GGIdl_mach64(int func, void **funcptr)
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
