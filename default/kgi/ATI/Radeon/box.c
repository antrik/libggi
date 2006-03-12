/* $Id: box.c,v 1.9 2006/03/12 23:15:06 soyt Exp $
******************************************************************************
   ATI Radeon box acceleration

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

#include "radeon_accel.h"
#include <string.h>

int GGI_kgi_radeon_drawbox_2d(struct ggi_visual *vis, int x, int y, int w, int h)
{
	struct {
		cce_type3_header_t h;
		cce_gui_control_t gc;
		uint32_t bp;
		cce_paint_t paint;
	} packet;

	/* Clipping across the bus costs bandwidth, faster to do the math. */
	LIBGGICLIP_XYWH(vis, x, y, w, h);

	memset(&packet, 0, sizeof(packet));

	packet.h.it_opcode = CCE_IT_OPCODE_PAINT;
	packet.h.count     = (sizeof(packet) / 4) - 2;
	packet.h.type      = 0x3;

	packet.gc.brush_type = 13;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 3; /* 3 means same as dst_type */
	packet.gc.win31_rop  = ROP3_PATCOPY;

	packet.bp = LIBGGI_GC_FGCOLOR(vis);

	packet.paint.left   = x;
	packet.paint.top    = y;
	packet.paint.right  = x + w;
	packet.paint.bottom = y + h;
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);
	return 0;
}


int GGI_kgi_radeon_drawbox_3d(struct ggi_visual *vis, int x, int y, int w, int h)
{
	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
	  float v1x, v1y; /* v1z; */ 
	  float v2x, v2y; /* v2z; */ 
	  float v3x, v3y; /* v3z; */ 
	} packet;

	RADEON_RESTORE_CTX(vis, RADEON_SOLIDFILL_CTX);
	
	memset(&packet, 0, sizeof(packet));
	packet.h.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
	packet.h.count     = (sizeof(packet) / 4) - 2;
	packet.h.type      = 3;

	packet.vfmt.z = 0 /* 1 */;

	packet.vctl.num_vertices = 3;
	packet.vctl.en_maos = 1;
	packet.vctl.fmt_mode = 1;
	packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
	packet.vctl.prim_type = 8;   /* Rectangle list */

	packet.v1x = x;
	packet.v1y = y;
	packet.v2x = x + w;
	packet.v2y = y;
	packet.v3x = x;
	packet.v3y = y + h;

	/*	packet.v1z = packet.v2z = packet.v3z = 0; */
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);
	return 0;
}

int GGI_kgi_radeon_copybox_2d(struct ggi_visual *vis, int x, int y, int w, int h,
			   int nx, int ny)
{
	radeon_context_t *ctx;
	struct {
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_pitch_offset_t spo;
		cce_bitblt_t bb;
	} packet;

	/* Clipping across the bus costs bandwidth, faster to do the math. */
        LIBGGICLIP_COPYBOX(vis, x, y, w, h, nx, ny);

	ctx = KGI_ACCEL_PRIV(vis);

	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_BITBLT;
	packet.h.count     = (sizeof(packet) / 4) - 2;
	packet.h.type      = 0x3;
	
	packet.gc.src_pitch_off = 1;
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 3;
	packet.gc.win31_rop  = ROP3_SRCCOPY;

	packet.spo = ctx->src_pitch_offset;

	packet.bb.src_x1 = x;
	packet.bb.src_y1 = y;
	packet.bb.src_w1 = w;
	packet.bb.src_h1 = h;
	packet.bb.dst_x1 = nx;
	packet.bb.dst_y1 = ny;
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	
	return 0;
}

int GGI_kgi_radeon_copybox_3d(struct ggi_visual *vis, int x, int y, int w, int h,
			      int nx, int ny)
{

	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
		float v1x, v1y, v1s, v1t; /* v1z; */ 
		float v2x, v2y, v2s, v2t; /* v2z; */ 
		float v3x, v3y, v3s, v3t; /* v3z; */ 
	} packet;

	RADEON_RESTORE_CTX(vis, RADEON_COPYBOX_CTX);
	
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

	/* Order verts to prevent artifacts from overlapping copies. */
	if (nx > x) {
		packet.v2x = nx;
		packet.v2y = ny;
		packet.v2s = x;
		packet.v2t = y;
		packet.v1x = nx + w;
		packet.v1y = ny;
		packet.v1s = x + w;
		packet.v1t = y;
	} else {
		packet.v1x = nx;
		packet.v1y = ny;
		packet.v1s = x;
		packet.v1t = y;
		packet.v2x = nx + w;
		packet.v2y = ny;
		packet.v2s = x + w;
		packet.v2t = y;
	}

	packet.v3x = nx;
	packet.v3y = ny + h;
	packet.v3s = x;
	packet.v3t = y + h;
	
	RADEON_WRITEPACKET(vis, packet);

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	
	return 0;
}


int GGI_kgi_radeon_putbox_3d(struct ggi_visual *vis, int x, int y, int w, int h, 
			     const void *buf)
{
	int y2, h2, wb, w32;
	radeon_context_t *ctx;
	char *chbuf = (char *)buf;

	struct {
		cce_type3_header_t h;
		cce_se_se_vtx_fmt_t vfmt;
		cce_se_se_vf_cntl_t vctl;
		/* TODO: this wrongly assumes float is 32 bit everywhere. */
		float v1x, v1y, v1s, v1t; /* v1z; */ 
		float v2x, v2y, v2s, v2t; /* v2z; */ 
		float v3x, v3y, v3s, v3t; /* v3z; */ 
	} pkt;
	struct {
		cce_type0_header_t h;
		uint32_t txoffset;
	} pkt2;
	struct {
		cce_type0_header_t h;
	        pp_tex_size_t tex_size;
	        pp_txpitch_t txpitch;
	} pkt3;
	struct {
		cce_type0_header_t h;
	        pp_tex_size_t tex_size;
	} pkt4;

	ctx = RADEON_CONTEXT(vis);

	y2 = 0;
	wb = GT_ByPP(LIBGGI_GT(vis)) * w;
	w32 = ((wb + 31) / 32) * 32;
	h2 = KGI_PRIV(vis)->swatch_size - ctx->swatch_inuse;
	h2 /= w32;

	if (ctx->ctx_loaded != RADEON_PUT_CTX) {
		ctx->put_ctx.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		ctx->put_ctx.tex_size.vsize = h2;
		ctx->put_ctx.txpitch.txpitch = (w32/32) - 1;
		RADEON_RESTORE_CTX(vis, RADEON_PUT_CTX);
	} else {
		memset(&pkt3, 0, sizeof(pkt3));

		pkt3.h.base_index = PP_TEX_SIZE_1 >> 2;
		pkt3.h.count = 1;
		pkt3.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
		pkt3.tex_size.vsize = h2;
		pkt3.txpitch.txpitch = (w32/32) - 1;
		
		RADEON_RESTORE_CTX(vis, RADEON_BASE_CTX);
		RADEON_WRITEPACKET(vis, pkt3);
		ctx->ctx_loaded = RADEON_PUT_CTX;
	}
	pkt4.tex_size.vsize = h2;

	memset(&pkt, 0, sizeof(pkt));

	pkt.h.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
	pkt.h.count     = 13;
	pkt.h.type      = 3;

	pkt.vfmt.st0 = 1;
	pkt.vfmt.z = 0 /* 1 */;

	pkt.vctl.num_vertices = 3;
	pkt.vctl.en_maos = 1;
	pkt.vctl.fmt_mode = 1;
	pkt.vctl.prim_walk = 3;   /* Vertex data follows in pkt. */
	pkt.vctl.prim_type = 8;   /* Rectangle list */

	pkt.v1x = pkt.v3x = x;
	pkt.v2x = x + w;
	pkt.v1s = pkt.v3s = 0;
	pkt.v2s = w;
	pkt.v1t = pkt.v2t = 0;

	memset(&pkt2, 0, sizeof(pkt2));
	pkt2.h.base_index = PP_TXOFFSET_1 >> 2;

	while (y2 < h) {

		if (h2 < 1) {
			/* idleaccel */
			ctx->swatch_inuse = 0;
			h2 = KGI_PRIV(vis)->swatch_size / w32;
		}
		if ((y2 + h2) > h) h2 = h - y2;

		if (pkt2.txoffset != 
		    (uint32_t)(KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse)) {
			pkt2.txoffset = 
			  (uint32_t)KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse;
			RADEON_WRITEPACKET(vis, pkt2);
		}
		if (h2 != pkt4.tex_size.vsize) {
			memset(&pkt4, 0, sizeof(pkt4));

			pkt4.h.base_index = PP_TEX_SIZE_1 >> 2;
			pkt4.h.count = 0;
			pkt4.tex_size.usize = w32 / GT_ByPP(LIBGGI_GT(vis));
			pkt4.tex_size.vsize = h2;
			
			RADEON_WRITEPACKET(vis, pkt4);
		}
			
			
		pkt.v1y = pkt.v2y = y + y2;
		pkt.v3y = y + y2 + h2;
		
		pkt.v3t = h2;
			
		y2 += h2;
		while (h2--) {
			memcpy(KGI_PRIV(vis)->swatch + ctx->swatch_inuse,
			       chbuf, wb);
			ctx->swatch_inuse += w32;
			chbuf += wb;
		}
	
		RADEON_WRITEPACKET(vis, pkt);

		h2 = KGI_PRIV(vis)->swatch_size - ctx->swatch_inuse;
		h2 /= w32;

	}

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	
	return 0;
}
