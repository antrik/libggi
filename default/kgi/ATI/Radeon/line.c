/* $Id: line.c,v 1.2 2002/10/31 03:20:17 redmondp Exp $
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

int GGI_kgi_radeon_drawhline(ggi_visual *vis, int x, int y, int w)
{
/*	GGI_kgi_radeon_drawline(vis, x, y, x + w - 1, y);
*/	GGI_kgi_radeon_drawbox(vis, x, y, w, 1);

	return 0;
}

int GGI_kgi_radeon_drawvline(ggi_visual *vis, int x, int y, int h)
{
/*	GGI_kgi_radeon_drawline(vis, x, y, x, y + h - 1);
*/	GGI_kgi_radeon_drawbox(vis, x, y, 1, h);

	return 0;
}

int GGI_kgi_radeon_drawline(ggi_visual *vis, int x1, int y1, int x2, int y2)
{
	struct {
	
		cce_type3_header_t h;
		cce_gui_control_t gc;
		uint32 bp;
		cce_polyline_t pl;
		
	} packet;
	
	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_POLYLINE;
	packet.h.count     = sizeof(packet) / 4 - 2;
	packet.h.type      = 0x3;

	packet.gc.brush_type = 14;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 3;
	packet.gc.win31_rop  = ROP3_PATCOPY;

	packet.gc.dst_type = RADEON_CONTEXT(vis)->dst_type;

	packet.bp = LIBGGI_GC_FGCOLOR(vis);

	packet.pl.x0 = x1;
	packet.pl.y0 = y1;
	packet.pl.x1 = x2;
	packet.pl.y1 = y2;

	RADEON_WRITEPACKET(vis, packet);

	return 0;
}