/* $Id: gtext.c,v 1.6 2005/07/30 11:39:59 cegger Exp $
******************************************************************************

   ATI Radeon text acceleration

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
#include <ggi/internal/font/8x8>
#include <string.h>
#include "radeon_accel.h"


int GGI_kgi_radeon_putc_3d(ggi_visual *vis, int x, int y, char c)
{
        struct {
		cce_type0_header_t h1;
		uint32_t txcblend;
                cce_type3_header_t h2;
                cce_se_se_vtx_fmt_t vfmt;
                cce_se_se_vf_cntl_t vctl;
        } packet;
	struct {
                /* TODO: this wrongly assumes float is 32 bit everywhere. */
                float v1x, v1y, v1s, v1t; /* v1z; */ 
                float v2x, v2y, v2s, v2t; /* v2z; */ 
                float v3x, v3y, v3s, v3t; /* v3z; */ 
	} rect;


        RADEON_RESTORE_CTX(vis, RADEON_TEXT_CTX);
        
        memset(&packet, 0, sizeof(packet));

	packet.h1.base_index = PP_TXCBLEND_1 >> 2;
	packet.h1.count = 0;

	packet.txcblend = 0x000c3088; /* Mask diffuse (fg) color by font. */

        packet.h2.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
        packet.h2.count     = 13;
        packet.h2.type      = 3;

        packet.vfmt.st0 = 1;

        packet.vctl.num_vertices = 3;
        packet.vctl.en_maos = 1;
        packet.vctl.fmt_mode = 1;
        packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
        packet.vctl.prim_type = 8;   /* Rectangle list */

	RADEON_WRITEPACKET(vis, packet);

	rect.v1t = rect.v2t = 0;
	rect.v3t = 8;
	rect.v1y = rect.v2y = y;
	rect.v3y = y + 8;
	rect.v1s = rect.v3s = (float)((unsigned char)(c)) * 8;
	rect.v2s = rect.v1s + 8;
	rect.v1x = rect.v3x = x;
	rect.v2x = x + 8;
	RADEON_WRITEPACKET(vis, rect);

        if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);

	return 0;
}

int GGI_kgi_radeon_puts_3d(ggi_visual *vis, int x, int y, const char *string)
{
	int i, len, nx;
        struct {
		cce_type0_header_t h1;
		uint32_t txcblend;
                cce_type3_header_t h2;
                cce_se_se_vtx_fmt_t vfmt;
                cce_se_se_vf_cntl_t vctl;
        } packet;
	struct {
                /* TODO: this wrongly assumes float is 32 bit everywhere. */
                float v1x, v1y, v1s, v1t; /* v1z; */ 
                float v2x, v2y, v2s, v2t; /* v2z; */ 
                float v3x, v3y, v3s, v3t; /* v3z; */ 
	} rect;


        RADEON_RESTORE_CTX(vis, RADEON_TEXT_CTX);
        
        memset(&packet, 0, sizeof(packet));

	len = strlen(string);

	packet.h1.base_index = PP_TXCBLEND_1 >> 2;
	packet.h1.count = 0;

	packet.txcblend = 0x000c3088; /* Mask diffuse (fg) color by font. */

        packet.h2.it_opcode = CCE_IT_OPCODE_3D_DRAW_IMMD;
        packet.h2.count     = 1 + len * 12;
        packet.h2.type      = 3;

        packet.vfmt.st0 = 1;

        packet.vctl.num_vertices = 3 * len;
        packet.vctl.en_maos = 1;
        packet.vctl.fmt_mode = 1;
        packet.vctl.prim_walk = 3;   /* Vertex data follows in packet. */
        packet.vctl.prim_type = 8;   /* Rectangle list */

	RADEON_WRITEPACKET(vis, packet);

	rect.v1t = rect.v2t = 0;
	rect.v3t = 8;
	rect.v1y = rect.v2y = y;
	rect.v3y = y + 8;

	nx = x;
	for (i = 0; i < len; i++) {
		rect.v1s = rect.v3s = (float)((unsigned char)(string[i])) * 8;
		rect.v2s = rect.v1s + 8;
		rect.v1x = rect.v3x = nx;
		rect.v2x = nx + 8;
		nx += 8;

		RADEON_WRITEPACKET(vis, rect);
	}

        if (!(LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC)) RADEON_FLUSH(vis);

	return 0;
}

typedef struct
{
	uint32_t dw1,dw2;
} glyph;


/* TODO: Use a BLT instead, because the smallchar_t doesn't support bgcolor */

int GGI_kgi_radeon_putc_2d(ggi_visual *vis, int x, int y, char c)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_scissor_t tl;
		cce_scissor_t br;
		cce_smalltext_t st;
		cce_smallchar_t sc;	
	} packet;
	glyph *glyphs = (glyph*)font;
	
	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_SMALL_TEXT;
	packet.h.count     = sizeof(packet) / 4 - 2;
	packet.h.type      = 0x3;
	
	packet.gc.dst_clipping = 1;
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 0;
	packet.gc.win31_rop  = ROP3_SRCCOPY;
	packet.gc.src_load   = 3;

        packet.tl.x = LIBGGI_GC(vis)->cliptl.x;
        packet.tl.y = LIBGGI_GC(vis)->cliptl.y;
        packet.br.x = LIBGGI_GC(vis)->clipbr.x;
        packet.br.y = LIBGGI_GC(vis)->clipbr.y;
	
	packet.st.frgd_color = LIBGGI_GC_FGCOLOR(vis);
	packet.st.bas_x = x;
	packet.st.bas_y = y;
	
	packet.sc.w       = 8;
	packet.sc.h       = 8;
	packet.sc.dy      = 8;
	packet.sc.raster1 = glyphs[(unsigned char)c].dw1;
	packet.sc.raster2 = glyphs[(unsigned char)c].dw2;
	
	RADEON_WRITEPACKET(vis, packet);
	
	return 0;
}


/* TODO: Use a BLT instead, because the smallchar_t doesn't support bgcolor */

int GGI_kgi_radeon_puts_2d(ggi_visual *vis, int x, int y, const char *string)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_scissor_t tl;
		cce_scissor_t br;
		cce_smalltext_t st;
	} packet;
	cce_smallchar_t sc;
	glyph *glyphs = (glyph*)font;

	if (!string) return 0;
	
	memset(&packet, 0, sizeof(packet));
	memset(&sc , 0, sizeof(sc));
	
	packet.h.it_opcode = CCE_IT_OPCODE_SMALL_TEXT;
	packet.h.count     = sizeof(packet) / 4 - 2;
	packet.h.type      = 0x3;
	
	packet.gc.dst_clipping = 1;
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 0;
	packet.gc.win31_rop  = ROP3_SRCCOPY;
	packet.gc.src_load   = 3;

        packet.tl.x = LIBGGI_GC(vis)->cliptl.x;
        packet.tl.y = LIBGGI_GC(vis)->cliptl.y;
        packet.br.x = LIBGGI_GC(vis)->clipbr.x;
        packet.br.y = LIBGGI_GC(vis)->clipbr.y;
	
	packet.st.frgd_color = LIBGGI_GC_FGCOLOR(vis);
	packet.st.bas_x = x;
	packet.st.bas_y = y;
		
	packet.h.count += strlen(string) * sizeof(sc) / 4;
	RADEON_WRITEPACKET(vis, packet);

	sc.w = 8;
	sc.h = 8;

	for (;*string; string++) {
		sc.raster1 = glyphs[(unsigned char)*string].dw1;
		sc.raster2 = glyphs[(unsigned char)*string].dw2;
		RADEON_WRITEPACKET(vis, sc);
		sc.dx = 8;
	}

	return 0;
}

int GGI_kgi_radeon_getcharsize(ggi_visual *vis, int *width, int *height)
{
	*width = *height = 8;
	
	return 0;
}
