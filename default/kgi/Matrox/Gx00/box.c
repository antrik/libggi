/* $Id: box.c,v 1.2 2003/01/10 23:28:05 ortalo Exp $
******************************************************************************
   Matrox Gx00 box acceleration

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

#include <ggi/internal/ggi-dl.h>

#include "Gx00.h"
#include "Gx00_accel.h"

int GGI_kgi_Gx00_drawbox(ggi_visual *vis, int x, int y, int w, int h)
{
  /* TODO: Question what should happen if w or h are negative */
  if ((w <= 0)||(h <= 0))
    return 0;

  GGI_kgi_Gx00_updatehwgc(vis);

  GX00_WRITE_DSTORG(vis, vis->w_frame_num * LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));
  GX00_WRITE_DWGCTL(vis,
		    DWGCTL_OPCOD_TRAP | DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
		    | DWGCTL_SOLID | DWGCTL_ARZERO | DWGCTL_SGNZERO
		    | DWGCTL_SHFTZERO
		    | ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
		    | ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
		    | DWGCTL_BLTMOD_BFCOL
		    /* patterning disabled */
		    | DWGCTL_TRANSC
		    /* clipping enabled */
		    );
  GX00_WRITE_REG(vis, (x & 0xFFFF) | (((x + w) & 0xFFFF) << 16),
		 FXBNDRY);
  GX00_WRITE_REG(vis, (h & 0xFFFF) | ((y & 0xFFFF) << 16),
		 YDSTLEN | ACCEL_GO);
  return 0;
}

int GGI_kgi_Gx00_drawbox_nc(ggi_visual *vis, int x, int y, int w, int h)
{
  if ((w <= 0)||(h <= 0))
    return 0;

  GGI_kgi_Gx00_updatehwgc(vis);

  GX00_WRITE_DWGCTL(vis,
		    DWGCTL_OPCOD_TRAP | DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
		    | DWGCTL_SOLID | DWGCTL_ARZERO | DWGCTL_SGNZERO
		    | DWGCTL_SHFTZERO
		    | ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
		    | ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
		    | DWGCTL_BLTMOD_BFCOL
		    /* patterning disabled */
		    | DWGCTL_TRANSC
		    /* no clipping */
		    | DWGCTL_CLIPDIS);
  GX00_WRITE_REG(vis, (x & 0xFFFF) | (((x + w) & 0xFFFF) << 16),
		 FXBNDRY);
  GX00_WRITE_REG(vis, (h & 0xFFFF) | ((y & 0xFFFF) << 16),
		 YDSTLEN | ACCEL_GO);
  return 0;
}

int GGI_kgi_Gx00_fillscreen(ggi_visual *vis)
{
  return
    GGI_kgi_Gx00_drawbox_nc(vis, 0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
}

int GGI_kgi_Gx00_copybox(ggi_visual *vis, int x, int y, int w, int h,
			   int nx, int ny)
{
  sint32 ar5;
  uint32 begin, end;
  uint32 sgn = 0;

  if ((w <= 0)||(h <= 0))
    return 0;

  GGI_kgi_Gx00_updatehwgc(vis);

  /*
  fprintf(stderr, "copybox x=%i y=%i w=%i h=%i nx=%i ny=%i\n",
	  x,y,w,h,nx,ny);
  */
  /* TODO: Check the logic of copybox which was copied from fbdev/g400 */

  /* Should not be necessary if we use DSTORG and SRCORG
  ny += vis->w_frame_num * LIBGGI_VIRTY(vis);
  y += vis->r_frame_num * LIBGGI_VIRTY(vis);
  */
  if (ny > y) {
    sgn |= SGN_SDY;
    y += h - 1;
    ny += h - 1;
    ar5 = - LIBGGI_VIRTX(vis);
  } else {
    ar5 = LIBGGI_VIRTX(vis);
  }

  begin = end = y * LIBGGI_VIRTX(vis) + x;
  w--;
  if (nx > x) {
    sgn |= SGN_SCANLEFT;
    begin += w;
  } else {
    end += w;
  }

  /*
  fprintf(stderr, "copybox parms: begin=%u end=%u sgn=%u ar5=%i y=%i ny=%i\n",
	  begin, end, sgn, ar5, y, ny);
  */

  GX00_WRITE_DWGCTL(vis,
		    DWGCTL_OPCOD_BITBLT | DWGCTL_ATYPE_RSTR
		    | DWGCTL_ZMODE_NOZCMP
		    | (sgn ? 0 : DWGCTL_SGNZERO)
		    | DWGCTL_SHFTZERO
		    | ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
		    | ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
		    | DWGCTL_BLTMOD_BFCOL);
  /* TODO: G200 and G400 do not have the same precision for AR{0,3,5}... */
  GX00_WRITE_REG(vis, begin, AR3);
  GX00_WRITE_REG(vis, end, AR0);
  GX00_WRITE_REG(vis, ar5 & 0x3FFFFF, AR5); /* y increment */
  GX00_WRITE_REG(vis, (nx & 0xFFFF) | (((nx+w) & 0xFFFF) << 16),
		 FXBNDRY);
  GX00_WRITE_DSTORG(vis, vis->w_frame_num * LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));
  GX00_WRITE_SRCORG(vis, vis->r_frame_num * LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));
  if (sgn)
    GX00_WRITE_REG(vis, sgn, SGN);
  GX00_WRITE_REG(vis, (h & 0xFFFF) | ((ny & 0xFFFF) << 16),
		 YDSTLEN | ACCEL_GO);
  return 0;
}
