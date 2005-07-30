/* $Id: visual.c,v 1.15 2005/07/30 11:39:59 cegger Exp $
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
#include <string.h>

#include "radeon_accel.h"

static int GGI_kgi_radeon_flush(ggi_visual *vis, int x, int y,
				int w, int h, int tryflag)
{
	RADEON_FLUSH(vis);
	return 0;
}

static int rwframes_changed_2d(ggi_visual *vis) {
	radeon_context_t *ctx;
	struct {
		cce_type0_header_t h1;
		cce_pitch_offset_t w_offset;
	} packet;

	ctx = KGI_ACCEL_PRIV(vis);
	if (vis->w_frame) ctx->gui2d_ctx.default_pitch_offset.offset = 
	  ((kgi_u8_t *)vis->w_frame->write - KGI_PRIV(vis)->fb) / 1024;
	if (vis->w_frame) ctx->src_pitch_offset.offset = 
	  ((kgi_u8_t *)(vis->r_frame->write) - KGI_PRIV(vis)->fb) / 1024;
	memset(&packet, 0, sizeof(packet));
	packet.h1.base_index = DEFAULT_PITCH_OFFSET >> 2;
	packet.h1.count = 0;
	packet.w_offset = ctx->gui2d_ctx.default_pitch_offset;

	RADEON_WRITEPACKET(vis, packet);

	return 0;
}

static int rwframes_changed_3d(ggi_visual *vis) {

	radeon_context_t *ctx;
	struct {
		cce_type0_header_t h1;
		uint32_t txoffset;
		cce_type0_header_t h2;
		uint32_t coloroffset;
	} packet;

	ctx = KGI_ACCEL_PRIV(vis);

	if (vis->r_frame) ctx->copybox_ctx.txoffset = 
	   (kgi_u8_t *)(vis->r_frame->read) - KGI_PRIV(vis)->fb;
	if (vis->w_frame) ctx->base_ctx.rb3d_coloroffset = 
	   (kgi_u8_t *)(vis->w_frame->write) - KGI_PRIV(vis)->fb;

	memset(&packet, 0, sizeof(packet));
	packet.h1.base_index = PP_TXOFFSET_1 >> 2;
	packet.h1.count = 0;
	packet.txoffset = ctx->copybox_ctx.txoffset;
	packet.h2.base_index = RB3D_COLOROFFSET >> 2;
	packet.h2.count = 0;
	packet.coloroffset = ctx->base_ctx.rb3d_coloroffset;
	RADEON_WRITEPACKET(vis, packet);

	return 0;
}

static int origin_changed(ggi_visual *vis) {
        ggi_directbuffer *db;
	ggi_graphtype gt;
	struct {
		cce_type0_header_t h1;
		uint32_t crtc_offset;
	} packet;

        db = _ggi_db_find_frame(vis, vis->d_frame_num);

        if (db == NULL) return GGI_ENOTFOUND;

	gt = LIBGGI_GT(vis);

	memset(&packet, 0, sizeof(packet));
	packet.h1.base_index = CRTC_OFFSET >> 2;
	packet.h1.count = 0;	
	packet.crtc_offset = 
	  (kgi_u8_t *)(db->read) - KGI_PRIV(vis)->fb;
	packet.crtc_offset +=
	  (vis->origin_y * LIBGGI_VIRTX(vis) + vis->origin_x) * GT_ByPP(gt);
	switch (GT_SIZE(gt)) {
	case 32:
		packet.crtc_offset &= 0xffffffc0;
		break;
	case 16:
		packet.crtc_offset &= 0xffffffe0;
		break;
	default:
		packet.crtc_offset &= 0xfffffff0;
		break;
	}

	RADEON_WRITEPACKET(vis, packet);
	RADEON_FLUSH(vis);

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	ggi_accel_t *accel;
	radeon_context_t *ctx;
	int use3d;

	if (!(accel = KGI_PRIV(vis)->map_accel(vis, 1, 0, 
		RADEON_BUFFER_SIZE_ORDER, RADEON_BUFFER_NUM, 0)))
		return GGI_ENODEVICE;

	if (!(ctx = (radeon_context_t*)malloc(sizeof(*ctx))))
		return GGI_ENOMEM;
	
	/* setup the accel_priv */
	KGI_ACCEL_PRIV(vis) = ctx;
	memset(ctx, 0, sizeof(*ctx));
	ctx->accel = accel;

	/* Set up the 2D graphics context. */
	ctx->src_pitch_offset.offset = 0;
	ctx->src_pitch_offset.pitch = 
	  (LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_GT(vis)) / 64 / 8); 
	switch (LIBGGI_GT(vis)) {
	
		case GT_8BIT: ctx->dst_type = 2; break;
		case GT_15BIT:ctx->dst_type = 4; break;
		case GT_16BIT:ctx->dst_type = 3; break;
		/* what no 24bit ?? */
		case GT_32BIT:ctx->dst_type = 6; break;
		default:
		  /* ?? Abort load ?? */
			ctx->dst_type = 0;
	}
	ctx->gui2d_ctx.h1.base_index = DEFAULT_PITCH_OFFSET >> 2;
	ctx->gui2d_ctx.h1.count = 0;
	ctx->gui2d_ctx.default_pitch_offset.offset = 0;
	ctx->gui2d_ctx.default_pitch_offset.pitch = 
	  (LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_GT(vis)) / 64 / 8); 

	ctx->gui2d_ctx.h2.base_index = DEFAULT_SC_BOT_RIGHT >> 2;
	ctx->gui2d_ctx.h2.count = 0;
	ctx->gui2d_ctx.default_sc_bot_right.x = 8191;
	ctx->gui2d_ctx.default_sc_bot_right.y = 8191;

	/* Set up the 3D graphics contexts */
	ctx->base_ctx.h1.base_index = RB3D_CNTL >> 2;
	ctx->base_ctx.h1.count = 5;

	use3d = KGI_PRIV(vis)->use3d;
	switch (LIBGGI_GT(vis)) {
		case GT_8BIT:
			ctx->base_ctx.rb3d_cntl = 9 << 10;  
			if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TRUECOLOR)
				ctx->base_ctx.rb3d_cntl = 7 << 10;
			break;
		case GT_15BIT: ctx->base_ctx.rb3d_cntl = 3 << 10; break;
		case GT_16BIT: ctx->base_ctx.rb3d_cntl = 4 << 10; break;
		case GT_32BIT: ctx->base_ctx.rb3d_cntl = 6 << 10; break;
		default: use3d = 0; break;
	}
	ctx->base_ctx.rb3d_coloroffset = 0;
	ctx->base_ctx.re_width_height = LIBGGI_VIRTX(vis) | 
	  (LIBGGI_VIRTY(vis) << 16);
	ctx->base_ctx.rb3d_colorpitch = LIBGGI_VIRTX(vis);
	/* Use solid color register, disable a bunch of features. */
	ctx->base_ctx.se_cntl = 0x0800003e;
	/* Use non-parametric texture coordinates. */
	ctx->base_ctx.se_coord_fmt = 0x04000100;
	ctx->base_ctx.h2.base_index = SE_CNTL_STATUS >> 2;
	ctx->base_ctx.h2.count = 0;
	ctx->base_ctx.se_cntl_status = 0;
	ctx->base_ctx.h3.base_index = RE_TOP_LEFT >> 2;
	ctx->base_ctx.h3.count = 0;
	ctx->base_ctx.re_top_left = 0;
	ctx->base_ctx.h4.base_index = PP_TXABLEND_1 >> 2;
	ctx->base_ctx.h4.count = 0;
	ctx->base_ctx.txablend = 0;


	ctx->ctx_loaded = RADEON_BAD_CTX;

	ctx->solidfill_ctx.h1.base_index = PP_CNTL >> 2;
	ctx->solidfill_ctx.h1.count = 0;
	ctx->solidfill_ctx.pp_cntl = 2;

	ctx->copybox_ctx.h1.base_index = PP_CNTL >> 2;
	ctx->copybox_ctx.h1.count = 0;
	ctx->copybox_ctx.pp_cntl = 0x00002022; /* Enable texture 0 */
	ctx->copybox_ctx.h2.base_index = PP_TXFORMAT_1 >> 2;
	ctx->copybox_ctx.h2.count = 2;
	switch (LIBGGI_GT(vis)) {
        case GT_8BIT:
		ctx->copybox_ctx.txformat.txformat = 0;  
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TRUECOLOR)
			ctx->copybox_ctx.txformat.txformat = 2;
		break;
        case GT_15BIT:
		ctx->copybox_ctx.txformat.txformat = 3; break;
	case GT_16BIT:
		ctx->copybox_ctx.txformat.txformat = 4; break;
        case GT_32BIT: 
		ctx->copybox_ctx.txformat.txformat = 6; break;
        default: use3d = 0; break;
        }
        ctx->copybox_ctx.txformat.non_power2 = 1;
	ctx->copybox_ctx.txoffset = 0;
	ctx->copybox_ctx.txcblend = 0x00803000;
	ctx->copybox_ctx.h3.base_index = PP_TEX_SIZE_1 >> 2;
	ctx->copybox_ctx.h3.count = 1;
	ctx->copybox_ctx.tex_size.usize = LIBGGI_VIRTX(vis);
	ctx->copybox_ctx.tex_size.vsize = LIBGGI_VIRTY(vis);
	ctx->copybox_ctx.txpitch.txpitch = 
	  (LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_GT(vis)) / 8 / 32) - 1; 

	ctx->put_ctx.h1.base_index = PP_CNTL >> 2;
	ctx->put_ctx.h1.count = 0;
	ctx->put_ctx.pp_cntl = 0x00002022; /* Enable texture 2 */
	ctx->put_ctx.h2.base_index = PP_TXFORMAT_1 >> 2;
	ctx->put_ctx.h2.count = 2;
	ctx->put_ctx.txformat = ctx->copybox_ctx.txformat;
	ctx->put_ctx.txoffset = 0;
	ctx->put_ctx.txcblend = 0x00803000;
	ctx->put_ctx.h3.base_index = PP_TEX_SIZE_1 >> 2;
	ctx->put_ctx.h3.count = 1;
	ctx->put_ctx.tex_size.usize = ctx->put_ctx.tex_size.vsize = 0;
	ctx->put_ctx.txpitch.txpitch = 0;

	ctx->text_ctx.h1.base_index = PP_CNTL >> 2;
	ctx->text_ctx.h1.count = 0;
	ctx->text_ctx.pp_cntl = 0x00002022; /* Enable texture 2 */
	ctx->text_ctx.h2.base_index = PP_TXFORMAT_1 >> 2;
	ctx->text_ctx.h2.count = 2;
        ctx->text_ctx.txformat.txformat = 0;
        ctx->text_ctx.txformat.non_power2 = 0;
	ctx->text_ctx.txformat.txwidth = 0xb;
	ctx->text_ctx.txformat.txheight = 0x3;
	ctx->text_ctx.txoffset = (kgi_u32_t)KGI_PRIV(vis)->font_gp;
	ctx->text_ctx.txcblend = 0x000c3080;

	ctx->crossblit_ctx.h1.base_index = PP_CNTL >> 2;
	ctx->crossblit_ctx.h1.count = 0;
	ctx->crossblit_ctx.pp_cntl = 0x00002022; /* Enable texture 2 */
	ctx->crossblit_ctx.h2.base_index = PP_TXFORMAT_1 >> 2;
	ctx->crossblit_ctx.h2.count = 2;
	ctx->crossblit_ctx.txformat = ctx->copybox_ctx.txformat;
	ctx->crossblit_ctx.txoffset = 0;
	ctx->crossblit_ctx.txcblend = 0x00803000;
	ctx->crossblit_ctx.h3.base_index = PP_TEX_SIZE_1 >> 2;
	ctx->crossblit_ctx.h3.count = 1;
	ctx->crossblit_ctx.tex_size.usize = 
	  ctx->crossblit_ctx.tex_size.vsize = 0;
	ctx->crossblit_ctx.txpitch.txpitch = 0;

	ctx->swatch_inuse = 0;

	if (use3d) RADEON_RESTORE_CTX(vis, RADEON_BASE_CTX);
	else RADEON_RESTORE_CTX(vis, RADEON_GUI2D_CTX);

	/* Load drawing operations */
	vis->opdisplay->flush     = GGI_kgi_radeon_flush;
	vis->opdraw->drawhline_nc = GGI_kgi_radeon_drawhline;
	vis->opdraw->drawhline    = GGI_kgi_radeon_drawhline;
	vis->opdraw->drawvline_nc = GGI_kgi_radeon_drawvline;
	vis->opdraw->drawvline    = GGI_kgi_radeon_drawvline;
	vis->opdraw->getcharsize  = GGI_kgi_radeon_getcharsize;

	/* If the mode is compatible, use the 3D engine */ 
	if (use3d) {	
		DPRINT("Using 3D engine.\n");
		vis->opdraw->drawline     = GGI_kgi_radeon_drawline_3d;
		vis->opdraw->drawbox      = GGI_kgi_radeon_drawbox_3d;
		vis->opdraw->copybox      = GGI_kgi_radeon_copybox_3d;
		vis->opgc->gcchanged      = GGI_kgi_radeon_gcchanged_3d;

		/* If we have swatch space, load accelerated put functions.
		 * The base KGI code will send -1 or a large enough swatch.
		 */
		if (KGI_PRIV(vis)->swatch_size >= 0) {
			DPRINT("Accelerating Put* functions.\n");
			vis->opdraw->putc        = GGI_kgi_radeon_putc_3d;
			vis->opdraw->puts        = GGI_kgi_radeon_puts_3d;
			vis->opdraw->putbox      = GGI_kgi_radeon_putbox_3d;
			vis->opdraw->puthline    = GGI_kgi_radeon_puthline_3d;
			vis->opdraw->putvline    = GGI_kgi_radeon_putvline_3d;
			vis->opdraw->crossblit   = GGI_kgi_radeon_crossblit_3d;
		}
		KGI_PRIV(vis)->rwframes_changed = rwframes_changed_3d;
	} else {
		DPRINT("Using 2D GUI engine.\n");
		vis->opdraw->drawline     = GGI_kgi_radeon_drawline_2d;
		vis->opdraw->drawbox      = GGI_kgi_radeon_drawbox_2d;
		vis->opdraw->copybox      = GGI_kgi_radeon_copybox_2d;
		vis->opgc->gcchanged      = GGI_kgi_radeon_gcchanged_2d;
		vis->opdraw->putc         = GGI_kgi_radeon_putc_2d;
		vis->opdraw->puts         = GGI_kgi_radeon_puts_2d;
		KGI_PRIV(vis)->rwframes_changed = rwframes_changed_2d;
	}

	KGI_PRIV(vis)->origin_changed = origin_changed;
	
	*dlret = GGI_DL_OPDRAW | GGI_DL_OPGC;
	return 0;	
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	free(KGI_ACCEL_PRIV(vis));
	KGI_ACCEL_PRIV(vis) = NULL;
	KGI_PRIV(vis)->origin_changed = NULL;
	KGI_PRIV(vis)->rwframes_changed = NULL;
	
	return 0;
}

EXPORTFUNC
int GGIdl_radeon(int func, void **funcptr);

int GGIdl_radeon(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
