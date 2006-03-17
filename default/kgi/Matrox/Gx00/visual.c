/* $Id: visual.c,v 1.13 2006/03/17 21:55:42 cegger Exp $
******************************************************************************

   Matrox Gx00 acceleration sublib for kgi display target

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
#include "config.h"
#ifdef HAVE_STRING_H
#include <string.h>	/* memset */
#endif
#include <unistd.h>
#include <sys/mman.h>

#include <ggi/internal/ggi-dl.h>
#include "kgi/config.h"
#include "Gx00_accel.h"

int GGI_kgi_Gx00_idleaccel(struct ggi_visual *vis)
{
  int i;
  /* We flush all buffers one after the other */
  for (i = GX00_BUFFER_NUM; i > 0; i--)
    {
      GX00_FLUSH_START(vis);
    }
  /* when we come back to the initial buffer, we know it has been
   * executed by the accelerator engine
   */
  vis->accelactive = 0;
  return 0;
}

static int GGI_kgi_Gx00_flush(struct ggi_visual *vis, int x, int y,
				int w, int h, int tryflag)
{
  return GGI_kgi_Gx00_idleaccel(vis);
}

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_accel_t *accel;
	Gx00_context_t *ctx;

	/* NB: The accel engine is resource 2 (1 is the ILOAD aperture,
	** 0 the framebuffer) */
	if (!(accel = KGI_PRIV(vis)->map_accel(vis, 2, 0,
		GX00_BUFFER_SIZE_ORDER, GX00_BUFFER_NUM, 0)))
		return GGI_ENODEVICE;

	if (!(ctx = (Gx00_context_t*)malloc(sizeof(*ctx))))
		return GGI_ENOMEM;

	/* setup the accel_priv data structures */
	KGI_ACCEL_PRIV(vis) = ctx;
	memset(ctx, 0, sizeof(*ctx));
	ctx->accel = accel;
	ctx->hwgc_mask = 0; /* TODO: Should use -1 instead? */
	/* setup the DMA buffers */
	GX00_INIT(vis);

	/* Initializes the pitch */
	GX00_WRITE_REG(vis, LIBGGI_VIRTX(vis), PITCH);
	/* Initializes the MACCESS fields */
	{
	  uint32_t maccess = MACCESS_ZWIDTH_ZW16; /* NB: no fogging */
	  switch (GT_ByPP(LIBGGI_GT(vis))) {
	  case 1:
	    maccess |= MACCESS_PWIDTH_PW8;
	    break;
	  case 2:
	    maccess |= MACCESS_PWIDTH_PW16;
	    if (LIBGGI_GT(vis) == GT_15BIT)
	      maccess |= MACCESS_DIT555;
	    /* else: GT_16BIT */
	    break;
	  case 4:
	    maccess |= MACCESS_PWIDTH_PW32;
	    break;
	  case 3:
	    maccess |= MACCESS_PWIDTH_PW24;
	    break;
	  default:
	    ggPanic("Unknown depth size!");
	    break;
	  }
	  /* TODO: check? -- ortalo: maccess |= MACCESS_NODITHER; */
	  GX00_WRITE_REG(vis, maccess, MACCESS);
	}
	/* Initializes the destination origin */
	GX00_WRITE_REG(vis, 0x0, DSTORG);
	/* Initializes the old-style destination origin */
	GX00_WRITE_REG(vis, 0x0, YDSTORG);
	/* Initializes the plane mask */
	GX00_WRITE_REG(vis, 0xFFFFFFFF, PLNWT);
#if 0 /* Maybe not needed? TODO: Check in libggi if clipping is initialized */
	/* Initializes the clipping to max */
	GX00_WRITE_REG(vis, 0 |
		       (((LIBGGI_X(vis) - 1) << CXBNDRY_CXRIGHT_SHIFT)
			& CXBNDRY_CXRIGHT_MASK),
		       CXBNDRY);
	GX00_WRITE_REG(vis, 0, YTOP);
	GX00_WRITE_REG(vis, (LIBGGI_Y(vis) - 1) * LIBGGI_VIRTX(vis),
		       YBOT);
#endif

	vis->opdisplay->idleaccel = GGI_kgi_Gx00_idleaccel;
	vis->opdisplay->flush     = GGI_kgi_Gx00_flush;
	vis->opgc->gcchanged      = GGI_kgi_Gx00_gcchanged;
	vis->opdraw->drawhline_nc = GGI_kgi_Gx00_drawhline_nc;
	vis->opdraw->drawhline    = GGI_kgi_Gx00_drawhline;
	vis->opdraw->drawvline_nc = GGI_kgi_Gx00_drawvline_nc;
	vis->opdraw->drawvline    = GGI_kgi_Gx00_drawvline;
	vis->opdraw->drawbox      = GGI_kgi_Gx00_drawbox;
	vis->opdraw->fillscreen   = GGI_kgi_Gx00_fillscreen;
	vis->opdraw->drawline     = GGI_kgi_Gx00_drawline;
	vis->opdraw->copybox      = GGI_kgi_Gx00_copybox;
	/* bugs on the G400
	*/
	vis->opdraw->getcharsize  = GGI_kgi_Gx00_getcharsize;
	vis->opdraw->putc         = GGI_kgi_Gx00_putc;
	/* The generic puts uses putc, so...
	vis->opdraw->puts         = GGI_kgi_Gx00_puts;
	*/

	vis->needidleaccel = 1;
	vis->accelactive = 0;

	*dlret = GGI_DL_OPDRAW | GGI_DL_OPGC;

	return 0;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(KGI_ACCEL_PRIV(vis));
	KGI_ACCEL_PRIV(vis) = NULL;

	return 0;
}

EXPORTFUNC
int GGIdl_kgi_Gx00(int func, void **funcptr);

int GGIdl_kgi_Gx00(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit: /* TODO: Check if exit is also there */
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
