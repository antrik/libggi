/* $Id: crossblit.c,v 1.3 2004/09/13 10:47:47 cegger Exp $
******************************************************************************
   ATI Radeon crossblit acceleration

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

static inline void blit3dctx(ggi_visual *vis,  radeon_context_t *ctx,
			     int sb32, int w32, int h2, pp_txformat_t txformat)
{
        struct {
		cce_type0_header_t h1;
                pp_txformat_t txformat;
                cce_type0_header_t h2;
                pp_tex_size_t tex_size;
                pp_txpitch_t txpitch;
        } pkt;


        if (ctx->ctx_loaded != RADEON_CROSSBLIT_CTX) {
		ctx->crossblit_ctx.txformat = txformat;
                ctx->crossblit_ctx.tex_size.usize = w32;
                ctx->crossblit_ctx.tex_size.vsize = h2;
                ctx->crossblit_ctx.txpitch.txpitch = (sb32/32) - 1;
                RADEON_RESTORE_CTX(vis, RADEON_CROSSBLIT_CTX);
        } else { 
                memset(&pkt, 0, sizeof(pkt));

		pkt.h1.base_index = PP_TXFORMAT_1 >> 2;
                pkt.h1.count = 0;
		pkt.txformat = txformat;
                pkt.h2.base_index = PP_TEX_SIZE_1 >> 2;
                pkt.h2.count = 1;
                pkt.tex_size.usize = w32;
                pkt.tex_size.vsize = h2;
                pkt.txpitch.txpitch = (sb32/32) - 1;
                
                RADEON_RESTORE_CTX(vis, RADEON_BASE_CTX);
                RADEON_WRITEPACKET(vis, pkt);
                ctx->ctx_loaded = RADEON_CROSSBLIT_CTX;
        }
}

static inline int blit3d(ggi_visual *vis, radeon_context_t *ctx, 
			 int x, int y, int w, int h, void *buf, 
			 int wb, int sb, int sb32, int h2)
{
	int y2;

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
		uint32 txoffset;
	} pkt2;
	struct {
		cce_type0_header_t h;
	        pp_tex_size_t tex_size;
	} pkt3;

	y2 = 0;

	pkt3.tex_size.vsize = h2; 	/* This was made so by blit3dctx */

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
			/* idleaccel */ RADEON_FLUSH(vis);
			ctx->swatch_inuse = 0;
			h2 = KGI_PRIV(vis)->swatch_size / sb32;
		}
		if ((y2 + h2) > h) h2 = h - y2;

		if (pkt2.txoffset != 
		    (uint32)(KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse)) {
			pkt2.txoffset = 
			  (uint32)KGI_PRIV(vis)->swatch_gp + ctx->swatch_inuse;
			RADEON_WRITEPACKET(vis, pkt2);
		}
		if (h2 != pkt3.tex_size.vsize) {
			memset(&pkt3, 0, sizeof(pkt3));

			pkt3.h.base_index = PP_TEX_SIZE_1 >> 2;
			pkt3.h.count = 0;
			pkt3.tex_size.usize = sb32 / GT_ByPP(LIBGGI_GT(vis));
			pkt3.tex_size.vsize = h2;
			
			RADEON_WRITEPACKET(vis, pkt3);
		}
			
			
		pkt.v1y = pkt.v2y = y + y2;
		pkt.v3y = y + y2 + h2;
		
		pkt.v3t = h2;
			
		y2 += h2;
		while (h2--) {
			memcpy(KGI_PRIV(vis)->swatch + ctx->swatch_inuse,
			       (char *)buf, wb);
			ctx->swatch_inuse += sb32;
			(char *)buf += sb;
		}
	
		RADEON_WRITEPACKET(vis, pkt);

		h2 = KGI_PRIV(vis)->swatch_size - ctx->swatch_inuse;
		h2 /= sb32;

	}

	if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);	

	/* remove me */ RADEON_FLUSH(vis);

	return 0;
}


static inline int blit3d_pack(ggi_visual *src, ggi_visual *dst, 
			      radeon_context_t *ctx, 
			      int x, int y, int w, int h, void *buf, 
			      int sb, int sb32, int h2)
{
	int y2;
	ggi_color *tmp;

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
		uint32 txoffset;
	} pkt2;
	struct {
		cce_type0_header_t h;
	        pp_tex_size_t tex_size;
	} pkt3;

	tmp = malloc(w * sizeof(ggi_color));
	if (tmp == NULL) return -1;

	y2 = 0;

	pkt3.tex_size.vsize = h2; 	/* This was made so by blit3dctx */

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
			/* idleaccel */ RADEON_FLUSH(dst);
			ctx->swatch_inuse = 0;
			h2 = KGI_PRIV(dst)->swatch_size / sb32;
		}
		if ((y2 + h2) > h) h2 = h - y2;

		if (pkt2.txoffset != 
		    (uint32)(KGI_PRIV(dst)->swatch_gp + ctx->swatch_inuse)) {
			pkt2.txoffset = 
			  (uint32)KGI_PRIV(dst)->swatch_gp + ctx->swatch_inuse;
			RADEON_WRITEPACKET(dst, pkt2);
		}
		if (h2 != pkt3.tex_size.vsize) {
			memset(&pkt3, 0, sizeof(pkt3));

			pkt3.h.base_index = PP_TEX_SIZE_1 >> 2;
			pkt3.h.count = 0;
			pkt3.tex_size.usize = sb32 / GT_ByPP(LIBGGI_GT(dst));
			pkt3.tex_size.vsize = h2;
			
			RADEON_WRITEPACKET(dst, pkt3);
		}
			
			
		pkt.v1y = pkt.v2y = y + y2;
		pkt.v3y = y + y2 + h2;
		
		pkt.v3t = h2;
			
		y2 += h2;
		while (h2--) {
			ggiUnpackPixels(src, buf, tmp, w);
			ggiPackColors(dst, 
				      KGI_PRIV(dst)->swatch +
				      ctx->swatch_inuse,
				      tmp, w);
			ctx->swatch_inuse += sb32;
			(char *)buf += sb;
		}
	
		RADEON_WRITEPACKET(dst, pkt);

		h2 = KGI_PRIV(dst)->swatch_size - ctx->swatch_inuse;
		h2 /= sb32;

	}

	if (!(LIBGGI_FLAGS(dst) & GGIFLAG_ASYNC)) RADEON_FLUSH(dst);	

	/* remove me */ RADEON_FLUSH(dst);
	free(tmp);

	return 0;
}


static inline int blit3d_get(ggi_visual *src, ggi_visual *dst, 
			     radeon_context_t *ctx, 
			     int x, int y, int w, int h,
			     int wg, int sx, int sy, int sb32, int h2)
{
	int y2;
	ggi_color *tmp;
	ggi_pixel *tmp2;

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
		uint32 txoffset;
	} pkt2;
	struct {
		cce_type0_header_t h;
	        pp_tex_size_t tex_size;
	} pkt3;

	tmp = malloc(w * sizeof(ggi_color));
	if (tmp == NULL) return -1;
	tmp2 = malloc(wg);
	if (tmp2 == NULL) { free(tmp); return -1; }

	y2 = 0;

	pkt3.tex_size.vsize = h2; 	/* This was made so by blit3dctx */

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
			/* idleaccel */ RADEON_FLUSH(dst);
			ctx->swatch_inuse = 0;
			h2 = KGI_PRIV(dst)->swatch_size / sb32;
		}
		if ((y2 + h2) > h) h2 = h - y2;

		if (pkt2.txoffset != 
		    (uint32)(KGI_PRIV(dst)->swatch_gp + ctx->swatch_inuse)) {
			pkt2.txoffset = 
			  (uint32)KGI_PRIV(dst)->swatch_gp + ctx->swatch_inuse;
			RADEON_WRITEPACKET(dst, pkt2);
		}
		if (h2 != pkt3.tex_size.vsize) {
			memset(&pkt3, 0, sizeof(pkt3));

			pkt3.h.base_index = PP_TEX_SIZE_1 >> 2;
			pkt3.h.count = 0;
			pkt3.tex_size.usize = sb32 / GT_ByPP(LIBGGI_GT(dst));
			pkt3.tex_size.vsize = h2;
			
			RADEON_WRITEPACKET(dst, pkt3);
		}
			
			
		pkt.v1y = pkt.v2y = y + y2;
		pkt.v3y = y + y2 + h2;
		
		pkt.v3t = h2;
			
		y2 += h2;

		while (h2--) {
			ggiGetHLine(src, sx, sy, w, tmp2);
			ggiUnpackPixels(src, tmp2, tmp, w);
			ggiPackColors(dst, 
				      KGI_PRIV(dst)->swatch +
				      ctx->swatch_inuse,
				      tmp, w);
			ctx->swatch_inuse += sb32;
			sy++;
		}
	
		RADEON_WRITEPACKET(dst, pkt);

		h2 = KGI_PRIV(dst)->swatch_size - ctx->swatch_inuse;
		h2 /= sb32;

	}

	if (!(LIBGGI_FLAGS(dst) & GGIFLAG_ASYNC)) RADEON_FLUSH(dst);	

	/* remove me */ RADEON_FLUSH(dst);

	free(tmp);
	return 0;
}


int GGI_kgi_radeon_crossblit_3d (ggi_visual *src, int sx, int sy, int w, 
				 int h, ggi_visual *dst, int dx, int dy) 
{
	int wb, sb, sb32, h2, get;
	ggi_visual *trn;
	radeon_context_t *ctx;
	char *buf;
	pp_txformat_t txformat;

	LIBGGICLIP_COPYBOX(dst,sx,sy,w,h,dx,dy);

	ctx = RADEON_CONTEXT(dst);

	get = 0; trn = src; 
	
	sb = 0; buf = NULL; /* Silence GCC */

        if ((src->r_frame == NULL) ||
	    (src->r_frame->layout != blPixelLinearBuffer) ||
	    (GT_SIZE(LIBGGI_GT(src)) < 8) ||
	    (GT_SCHEME(LIBGGI_GT(src)) != GT_TRUECOLOR) ||
	    (GT_SCHEME(LIBGGI_GT(dst)) != GT_TRUECOLOR)) {
		trn = dst;
		get = 1;
		txformat = ctx->put_ctx.txformat; 
		goto skip;
	}

	memset(&txformat, 0, sizeof(txformat));
	txformat.non_power2 = 1;

        switch (src->r_frame->buffer.plb.pixelformat->stdformat) {

	case GGI_DB_STD_8a8r3g3b2:
		txformat.txformat = 2;
		break;

	case GGI_DB_STD_15a16p1r5g5b5rev:
		txformat.endian_swap = 1;
        case GGI_DB_STD_15a16p1r5g5b5:
		txformat.txformat = 3;
		break;

        case GGI_DB_STD_16a16r5g6b5rev:
		txformat.endian_swap = 1;
        case GGI_DB_STD_16a16r5g6b5:
		txformat.txformat = 4;
		break;

        case GGI_DB_STD_24a32b8g8r8p8:
		txformat.endian_swap = 2;
        case GGI_DB_STD_24a32p8r8g8b8:
		txformat.txformat = 6;
		break;

        case GGI_DB_STD_24a32p8b8g8r8:
		txformat.endian_swap = 2;
        case GGI_DB_STD_24a32r8g8b8p8:
		txformat.txformat = 7;
		break;

	case GGI_DB_STD_8a8i8:
        case GGI_DB_STD_15a16p1b5g5r5:
	case GGI_DB_STD_15a16p1b5g5r5rev:
        case GGI_DB_STD_16a16b5g6r5:
        case GGI_DB_STD_16a16b5g6r5rev:
        case GGI_DB_STD_24a24r8g8b8:
        case GGI_DB_STD_24a24b8g8r8:

        default:
		txformat = ctx->put_ctx.txformat;
		trn = dst;
        }

	sb = LIBGGI_FB_R_STRIDE(src);
	buf = LIBGGI_CURREAD(src);
	buf += sb * sy;
	buf += sx * GT_ByPP(LIBGGI_GT(src));
 skip:
	wb = w * GT_SIZE(LIBGGI_GT(trn));
	wb += 7;
	wb /= 8;
	sb32 = ((wb + 31) / 32) * 32;
        h2 = KGI_PRIV(dst)->swatch_size - ctx->swatch_inuse;
        h2 /= sb32;

	blit3dctx(dst,ctx,sb32,sb32/GT_ByPP(LIBGGI_GT(trn))/8,h2,txformat);

	if (get) {
	  return blit3d_get(src, dst, ctx, dx, dy, w, h, 
			    GT_ByPPP(w, LIBGGI_GT(src)),
			    sx, sy, sb32, h2);
	}
	if (trn == dst) {
	  return blit3d_pack(src, dst, ctx, dx, dy, w, h, 
			     buf, sb, sb32, h2);
	}
	return blit3d(dst, ctx, dx, dy, w, h, buf, wb, sb, sb32, h2);
}

