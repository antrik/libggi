/* $Id: box.c,v 1.2 2002/10/31 03:20:17 redmondp Exp $
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

int GGI_kgi_radeon_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
	struct {
		cce_type3_header_t h;
		cce_gui_control_t gc;
		uint32 bp;
		cce_paint_t paint;
	} packet;
	
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

	return 0;
}

int GGI_kgi_radeon_copybox(ggi_visual *vis, int x, int y, int w, int h,
			   int nx, int ny)
{
	struct {
		cce_type3_header_t h;
		cce_gui_control_t gc;
		cce_bitblt_t bb;
	} packet;

	memset(&packet, 0, sizeof(packet));
	
	packet.h.it_opcode = CCE_IT_OPCODE_BITBLT;
	packet.h.count     = (sizeof(packet) / 4) - 2;
	packet.h.type      = 0x3;
	
	packet.gc.brush_type = 15;
	packet.gc.dst_type   = RADEON_CONTEXT(vis)->dst_type;
	packet.gc.src_type   = 3;
	packet.gc.win31_rop  = ROP3_SRCCOPY;

	packet.bb.src_x1 = x;
	packet.bb.src_y1 = y;
	packet.bb.src_w1 = w;
	packet.bb.src_h1 = h;
	packet.bb.dst_x1 = nx;
	packet.bb.dst_y1 = ny;
	
	RADEON_WRITEPACKET(vis, packet);
	
	return 0;
}