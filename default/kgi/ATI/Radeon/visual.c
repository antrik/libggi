/* $Id: visual.c,v 1.3 2002/11/03 04:23:07 redmondp Exp $
******************************************************************************

   ATI Radeon acceleration sublib for kgi display target

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

#include <unistd.h>
#include <sys/mman.h>

#include "radeon_accel.h"

static int GGI_kgi_radeon_flush(ggi_visual *vis, int x, int y,
				int w, int h, int tryflag)
{
	RADEON_FLUSH(vis);

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_accel_t *accel;
	radeon_context_t *ctx;

	if (!(accel = KGI_PRIV(vis)->map_accel(vis, 1, 0, 
		RADEON_BUFFER_SIZE_ORDER, RADEON_BUFFER_NUM, 0)))
		return -1;

	if (!(ctx = (radeon_context_t*)malloc(sizeof(*ctx))))
		return -1;
	
	/* setup the accel_priv */
	KGI_ACCEL_PRIV(vis) = ctx;
	memset(ctx, 0, sizeof(*ctx));
	ctx->accel = accel;
	switch (vis->mode->graphtype) {
	
		case GT_8BIT: ctx->dst_type = 2; break;
		case GT_15BIT:ctx->dst_type = 4; break;
		case GT_16BIT:ctx->dst_type = 3; break;
		/* what no 24bit ?? */
		case GT_32BIT:ctx->dst_type = 6; break;
		default:
			ctx->dst_type = 0;
	}

	vis->opdisplay->flush     = GGI_kgi_radeon_flush;
	vis->opdraw->drawhline_nc = GGI_kgi_radeon_drawhline;
	vis->opdraw->drawhline    = GGI_kgi_radeon_drawhline;
	vis->opdraw->drawvline_nc = GGI_kgi_radeon_drawvline;
	vis->opdraw->drawvline    = GGI_kgi_radeon_drawvline;
	vis->opdraw->drawline     = GGI_kgi_radeon_drawline;
	vis->opdraw->drawbox      = GGI_kgi_radeon_drawbox;
	vis->opdraw->copybox      = GGI_kgi_radeon_copybox;
	vis->opdraw->putc         = GGI_kgi_radeon_putc;
	vis->opdraw->puts         = GGI_kgi_radeon_puts;
	vis->opdraw->getcharsize  = GGI_kgi_radeon_getcharsize;
	vis->opgc->gcchanged      = GGI_kgi_radeon_gcchanged;
	
	*dlret = GGI_DL_OPDRAW | GGI_DL_OPGC;
	return 0;	
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(KGI_ACCEL_PRIV(vis));
	KGI_ACCEL_PRIV(vis) = NULL;
	
	return 0;
}

int GGIdl_radeon(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
