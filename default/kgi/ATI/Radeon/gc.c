/* $Id: gc.c,v 1.4 2003/02/07 01:35:09 skids Exp $
******************************************************************************

   ATI Radeon gc acceleration

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

void GGI_kgi_radeon_gcchanged_3d(ggi_visual *vis, int mask) {
	if (mask & GGI_GCCHANGED_FG) {
		struct {
			cce_type0_header_t h;
			uint32 val;
		} packet;

		memset(&packet, 0, sizeof(packet));
		packet.h.base_index = RE_SOLID_COLOR >> 2;
		packet.h.count = 0;

		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
			packet.val = LIBGGI_GC_FGCOLOR(vis);
			packet.val <<= 16;
		} else if (GT_DEPTH(LIBGGI_GT(vis)) == 24) {
			packet.val = LIBGGI_GC_FGCOLOR(vis);
		} else {
			ggi_color col;

			ggiUnmapPixel(vis, LIBGGI_GC_FGCOLOR(vis), &col);
			col.a >>= 8;
			col.r >>= 8;
			col.g >>= 8;
			col.b >>= 8;
			packet.val = (uint32)col.a << 24 | 
			  (uint32)col.r << 16 | 
			  (uint32)col.g << 8 | 
			  (uint32)col.b;
		}
		RADEON_WRITEPACKET(vis, packet);
	}
	if (mask & GGI_GCCHANGED_BG) {

		struct {
	  		cce_type0_header_t h;
			uint32 val;
		} packet;


		memset(&packet, 0, sizeof(packet));
		packet.h.base_index = PP_TFACTOR_1 >> 2;
		packet.h.count = 0;

		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
			packet.val = LIBGGI_GC_BGCOLOR(vis);
			packet.val <<= 16;
		} else if (GT_DEPTH(LIBGGI_GT(vis)) == 24) {
			packet.val = LIBGGI_GC_BGCOLOR(vis);
		} else {
			ggi_color col;
			ggiUnmapPixel(vis, LIBGGI_GC_BGCOLOR(vis), &col);
			col.a >>= 8;
			col.r >>= 8;
			col.g >>= 8;
			col.b >>= 8;
			packet.val = (uint32)col.a << 24 | 
			  (uint32)col.r << 16 | 
			  (uint32)col.g << 8 | 
			  (uint32)col.b;
		}
		RADEON_WRITEPACKET(vis, packet);
	}
	if (mask & GGI_GCCHANGED_CLIP) {
		struct {
	  		cce_type0_header_t h;
			uint32 val;
		} packet;

		/* TODO: mask out bad values. */
		memset(&packet, 0, sizeof(packet));
		packet.h.base_index = RE_TOP_LEFT >> 2;
		packet.h.count = 0;
		packet.val = LIBGGI_GC(vis)->cliptl.x | 
		  (LIBGGI_GC(vis)->cliptl.y << 16);
		RADEON_CONTEXT(vis)->base_ctx.re_top_left = packet.val;
		RADEON_WRITEPACKET(vis, packet);

		memset(&packet, 0, sizeof(packet));
		packet.h.base_index = RE_WIDTH_HEIGHT >> 2;
		packet.h.count = 0;
		packet.val = (LIBGGI_GC(vis)->clipbr.x - 1) | 
		  ((LIBGGI_GC(vis)->clipbr.y - 1) << 16);
		RADEON_CONTEXT(vis)->base_ctx.re_width_height = packet.val;
		RADEON_WRITEPACKET(vis, packet);
	}
}

void GGI_kgi_radeon_gcchanged_2d(ggi_visual *vis, int mask) {
	/* Nothing to do because the scissors don't work in a desirable 
	 * manner when using the GUI2D engine through CCE -- clip must be
	 * sent with each 2D packet.  Solid colors are also sent in packet.
	 * We keep this place holder in case we find a need for it.
	 */
}

#if 0
void GGI_kgi_radeon_clut_ilut_sync(vis) {
        struct {
                cce_type0_header_t h1;
                uint32 clutidx;
		cce_type0_header_t h2;
		clutdata[256];
        } pkt;	
	radeon_context_t *ctx;
	unsigned int len, i;

	if (GT_SCHEME(LIBGGI_GT(vis)) != GT_PALLETE) return;
	if (KGI_PRIV(vis)->ilut_touched == 0) return;
	
	ctx = RADEON_CONTEXT(vis);

	len = 1 << GT_DEPTH(LIBGGI_GT(vis));

	pkt.h1.base_inde

	for (i = 0; i < len; i++) {
		pkt.clutdata[i] = 
		  ((vis->pallete[i].a >> 8) << 24) |
		  ((vis->pallete[i].r >> 8) << 16) |
		  ((vis->pallete[i].g >> 8) << 8) |
		  (vis->pallete[i].b >> 8);
	}

	KGI_PRIV(vis)->ilut_touched = 0; 
}
#endif
