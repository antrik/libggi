/* $Id: line.c,v 1.1 2002/12/23 13:17:35 ortalo Exp $
******************************************************************************

   Matrox Gx00 line acceleration

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

#include <ggi/internal/ggi-dl.h>

#include "Gx00.h"
#include "Gx00_accel.h"

int GGI_kgi_Gx00_drawhline(ggi_visual *vis, int x, int y, int w)
{
/*	GGI_kgi_Gx00_drawline(vis, x, y, x + w - 1, y);
*/	GGI_kgi_Gx00_drawbox(vis, x, y, w, 1);

	return 0;
}

int GGI_kgi_Gx00_drawvline(ggi_visual *vis, int x, int y, int h)
{
/*	GGI_kgi_Gx00_drawline(vis, x, y, x, y + h - 1);
*/	GGI_kgi_Gx00_drawbox(vis, x, y, 1, h);

	return 0;
}

int GGI_kgi_Gx00_drawhline_nc(ggi_visual *vis, int x, int y, int w)
{
	return GGI_kgi_Gx00_drawbox_nc(vis, x, y, w, 1);
}

int GGI_kgi_Gx00_drawvline_nc(ggi_visual *vis, int x, int y, int h)
{
	return GGI_kgi_Gx00_drawbox_nc(vis, x, y, 1, h);
}

int GGI_kgi_Gx00_drawline(ggi_visual *vis, int x1, int y1, int x2, int y2)
{
  GGI_kgi_Gx00_updatehwgc(vis);

  GX00_WRITE_DSTORG(vis, vis->w_frame_num * LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));
  GX00_WRITE_DWGCTL(vis,
		    DWGCTL_OPCOD_AUTOLINE_CLOSE
		    | DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
		    | DWGCTL_SOLID
		    | DWGCTL_SHFTZERO
		    | ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
		    | ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
		    | DWGCTL_BLTMOD_BFCOL
		    /* patterning disabled */
		    /* translucency disabled */
		    /* clipping enabled */
		    );
  GX00_WRITE_REG(vis, (x1 & 0xFFFF) | ((y1 & 0xFFFF) << 16),
		 XYSTRT);
  GX00_WRITE_REG(vis, (x2 & 0xFFFF) | ((y2 & 0xFFFF) << 16),
		  XYEND | ACCEL_GO);

  return 0;
}
