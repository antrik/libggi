/* $Id: gtext.c,v 1.1 2002/11/03 04:25:29 redmondp Exp $
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
#include "radeon_accel.h"

typedef struct
{
	uint32 dw1,dw2;
} glyph;

int GGI_kgi_radeon_putc(ggi_visual *vis, int x, int y, char c)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_smalltext_t st;
		cce_smallchar_t sc;
	
	} packet;
	glyph *glyphs = (glyph*)font;
	
	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_SMALL_TEXT;
	packet.h.count     = sizeof(packet) / 4 - 2;
	packet.h.type      = 0x3;
	
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 0;
	packet.gc.win31_rop  = ROP3_SRCCOPY;
	packet.gc.src_load   = 3;
	
	packet.st.frgd_color = LIBGGI_GC_FGCOLOR(vis);
	packet.st.bas_x      = x;
	packet.st.bas_y      = y;
	
	packet.sc.w       = 8;
	packet.sc.h       = 8;
	packet.sc.dy      = 8;
	packet.sc.raster1 = glyphs[c].dw1;
	packet.sc.raster2 = glyphs[c].dw2;
	
	RADEON_WRITEPACKET(vis, packet);
	
	return 0;
}

int GGI_kgi_radeon_puts(ggi_visual *vis, int x, int y, const char *string)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
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
	
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 0;
	packet.gc.win31_rop  = ROP3_SRCCOPY;
	packet.gc.src_load   = 3;
	
	packet.st.frgd_color = LIBGGI_GC_FGCOLOR(vis);
	packet.st.bas_x = x;
	packet.st.bas_y = y;
		
	packet.h.count += strlen(string) * sizeof(sc) / 4;
	RADEON_WRITEPACKET(vis, packet);

	sc.w = 8;
	sc.h = 8;

	for (;*string; string++) {
		sc.raster1 = glyphs[*string].dw1;
		sc.raster2 = glyphs[*string].dw2;
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