/* $Id: line.c,v 1.5 2004/12/01 23:08:01 cegger Exp $
******************************************************************************

   ATI Radeon line acceleration

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

*******************************************************************************/

#include "radeon_accel.h"
#include <string.h>

int GGI_kgi_radeon_drawhline(ggi_visual *vis, int x, int y, int w)
{
	vis->opdraw->drawbox(vis, x, y, w, 1);
	return 0;
}

int GGI_kgi_radeon_drawvline(ggi_visual *vis, int x, int y, int h)
{
	vis->opdraw->drawbox(vis, x, y, 1, h);
	return 0;
}

int GGI_kgi_radeon_drawline_2d(ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_scissor_t tl;
		cce_scissor_t br;
		uint32 bp;
		cce_polyline_t pl;
	} packet;

	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_POLYLINE;
	packet.h.count     = sizeof(packet) / 4 - 2;
	packet.h.type      = 0x3;

	packet.gc.dst_clipping = 1;
	packet.gc.brush_type = 14;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 3;
	packet.gc.win31_rop  = ROP3_PATCOPY;

	packet.gc.dst_type = RADEON_CONTEXT(vis)->dst_type;

	packet.tl.x = LIBGGI_GC(vis)->cliptl.x;
	packet.tl.y = LIBGGI_GC(vis)->cliptl.y;
	packet.br.x = LIBGGI_GC(vis)->clipbr.x;
	packet.br.y = LIBGGI_GC(vis)->clipbr.y;

	packet.bp = LIBGGI_GC_FGCOLOR(vis);

	packet.pl.x0 = x1;
	packet.pl.y0 = y1;
	packet.pl.x1 = x2;
	packet.pl.y1 = y2;

	RADEON_WRITEPACKET(vis, packet);
        if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);

	return 0;
}

int GGI_kgi_radeon_drawline_3d(ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
	  float v1x, v1y; /* v1z; */ 
	  float v2x, v2y; /* v2z; */ 
	} packet;

	RADEON_RESTORE_CTX(vis, RADEON_SOLIDFILL_CTX);
	
	memset(&packet, 0, sizeof(packet));
	packet.h.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
	packet.h.count     = (sizeof(packet) / 4) - 2;
	packet.h.type      = 3;

	packet.vfmt.z = 0 /* 1 */;

	packet.vctl.num_vertices = 2;
	packet.vctl.en_maos = 1;
	packet.vctl.fmt_mode = 1;
	packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
	packet.vctl.prim_type = 2;   /* Line list */

	packet.v1x = x1;
	packet.v1y = y1;
	packet.v2x = x2;
	packet.v2y = y2;

	/*	packet.v1z = packet.v2z = packet.v3z = 0; */
	
	RADEON_WRITEPACKET(vis, packet);
        if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);

	return 0;
}

int GGI_kgi_radeon_puthline_3d(ggi_visual *vis, int x, int y, int w, const void *buf)
{
	int wb, w32;
	radeon_context_t *ctx;

	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
		float v1x, v1y, v1s, v1t; /* v1z; */ 
		float v2x, v2y, v2s, v2t; /* v2z; */ 
		float v3x, v3y, v3s, v3t; /* v3z; */ 
	} packet;
	struct {
		cce_type0_header_t h;
		uint32 txoffset;
	} offsetpkt;


	ctx = RADEON_CONTEXT(vis);

	wb = GT_ByPP(LIBGGI_GT(vis)) * w;
	w32 = ((wb + 31) / 32) * 32;

	if (ctx->ctx_loaded != RADEON_PUT_CTX) {
		ctx->put_ctx.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		ctx->put_ctx.tex_size.vsize = 1;
		ctx->put_ctx.txpitch.txpitch = (w32/32) - 1;
		RADEON_RESTORE_CTX(vis, RADEON_PUT_CTX);
	} else {
		struct {
			cce_type0_header_t h;
			pp_tex_size_t tex_size;
			pp_txpitch_t txpitch;
		} packet2;

		memset(&packet2, 0, sizeof(packet2));

		packet2.h.base_index = PP_TEX_SIZE_1 >> 2;
		packet2.h.count = 1;
		packet2.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		packet2.tex_size.vsize = 1;
		packet2.txpitch.txpitch = (w32/32) - 1;
		
		RADEON_RESTORE_CTX(vis, RADEON_BASE_CTX);
		RADEON_WRITEPACKET(vis, packet2);
		ctx->ctx_loaded = RADEON_PUT_CTX;
	}

	memset(&packet, 0, sizeof(packet));

	packet.h.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
	packet.h.count     = 13;
	packet.h.type      = 3;

	packet.vfmt.st0 = 1;
	packet.vfmt.z = 0 /* 1 */;

	packet.vctl.num_vertices = 3;
	packet.vctl.en_maos = 1;
	packet.vctl.fmt_mode = 1;
	packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
	packet.vctl.prim_type = 8;   /* Rectangle list */

	packet.v1x = packet.v3x = x;
	packet.v2x = x + w;
	packet.v1y = packet.v2y = y;
	packet.v3y = y + 1;
	packet.v1s = packet.v3s = 0;
	packet.v2s = w;
	packet.v1t = packet.v2t = 0;
	packet.v3t = 1;

	memset(&offsetpkt, 0, sizeof(offsetpkt));
	offsetpkt.h.base_index = PP_TXOFFSET_1 >> 2;

	if ((KGI_PRIV(vis)->swatch_size - ctx->swatch_inuse) < w32) {
		/* idleaccel */
	        ctx->swatch_inuse = 0;
	}

	offsetpkt.txoffset = 
	  (uint32)KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse;

	RADEON_WRITEPACKET(vis, offsetpkt);

	memcpy(KGI_PRIV(vis)->swatch + ctx->swatch_inuse,
	       (char *)buf, wb);
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	
	return 0;
}


int GGI_kgi_radeon_putvline_3d(ggi_visual *vis, int x, int y, int h, const void *buf)
{
	int wb, w32;
	radeon_context_t *ctx;

	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
		float v1x, v1y, v1s, v1t; /* v1z; */ 
		float v2x, v2y, v2s, v2t; /* v2z; */ 
		float v3x, v3y, v3s, v3t; /* v3z; */ 
	} packet;
	struct {
		cce_type0_header_t h;
		uint32 txoffset;
	} offsetpkt;


	ctx = RADEON_CONTEXT(vis);

	wb = GT_ByPP(LIBGGI_GT(vis)) * h;
	w32 = ((wb + 31) / 32) * 32;

	if (ctx->ctx_loaded != RADEON_PUT_CTX) {
		ctx->put_ctx.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		ctx->put_ctx.tex_size.vsize = 1;
		ctx->put_ctx.txpitch.txpitch = (w32/32) - 1;
		RADEON_RESTORE_CTX(vis, RADEON_PUT_CTX);
	} else {
		struct {
			cce_type0_header_t h;
			pp_tex_size_t tex_size;
			pp_txpitch_t txpitch;
		} packet2;

		memset(&packet2, 0, sizeof(packet2));

		packet2.h.base_index = PP_TEX_SIZE_1 >> 2;
		packet2.h.count = 1;
		packet2.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		packet2.tex_size.vsize = 1;
		packet2.txpitch.txpitch = (w32/32) - 1;
		
		RADEON_RESTORE_CTX(vis, RADEON_BASE_CTX);
		RADEON_WRITEPACKET(vis, packet2);
		ctx->ctx_loaded = RADEON_PUT_CTX;
	}

	memset(&packet, 0, sizeof(packet));

	packet.h.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
	packet.h.count     = 13;
	packet.h.type      = 3;

	packet.vfmt.st0 = 1;
	packet.vfmt.z = 0 /* 1 */;

	packet.vctl.num_vertices = 3;
	packet.vctl.en_maos = 1;
	packet.vctl.fmt_mode = 1;
	packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
	packet.vctl.prim_type = 8;   /* Rectangle list */

	packet.v1x = packet.v3x = x;
	packet.v2x = x + 1;
	packet.v1y = packet.v2y = y;
	packet.v3y = y + h;
	packet.v1s = packet.v2s = 0;
        packet.v3s = h;
	packet.v1t = packet.v3t = 1;
	packet.v2t = 0;

	memset(&offsetpkt, 0, sizeof(offsetpkt));
	offsetpkt.h.base_index = PP_TXOFFSET_1 >> 2;

	if ((KGI_PRIV(vis)->swatch_size - ctx->swatch_inuse) < w32) {
		/* idleaccel */
	        ctx->swatch_inuse = 0;
	}

	offsetpkt.txoffset = 
	  (uint32)KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse;

	RADEON_WRITEPACKET(vis, offsetpkt);

	memcpy(KGI_PRIV(vis)->swatch + ctx->swatch_inuse,
	       (char *)buf, wb);
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	
	return 0;
}
