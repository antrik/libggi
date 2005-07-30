/* $Id: gc.c,v 1.2 2005/07/30 11:39:59 cegger Exp $
******************************************************************************

   Matrox Gx00 gc acceleration

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

void GGI_kgi_Gx00_updatehwgc(ggi_visual *vis)
{
  int mask = GX00_CONTEXT(vis)->hwgc_mask;

  if (mask & GGI_GCCHANGED_FG) {
    uint32_t fcol = LIBGGI_GC_FGCOLOR(vis);
    if (LIBGGI_GT(vis) == GT_15BIT) {
      /* In 5:5:5, bit15 originates from fcol<31> */
      fcol &= ~(1 << 31);
      fcol |= (fcol & (1 << 15)) << (31 - 15);
    }
    GX00_WRITE_REG(vis, fcol, FCOL);
  }
  if (mask & GGI_GCCHANGED_BG)
    GX00_WRITE_REG(vis, LIBGGI_GC_BGCOLOR(vis), BCOL);
  if (mask & GGI_GCCHANGED_CLIP)
    {
      GX00_WRITE_REG(vis,
		     ((LIBGGI_GC(vis)->cliptl.x << CXBNDRY_CXLEFT_SHIFT) 
		      & CXBNDRY_CXLEFT_MASK)
		     | (((LIBGGI_GC(vis)->clipbr.x - 1) << CXBNDRY_CXRIGHT_SHIFT) 
			& CXBNDRY_CXRIGHT_MASK),
		     CXBNDRY);
      /* NB: We assume YDSTORG to 0 (i.e. use of DSTORG) */
      GX00_WRITE_REG(vis, (LIBGGI_GC(vis)->cliptl.y * LIBGGI_VIRTX(vis))
		     & 0x00FFFFFF, YTOP);
      GX00_WRITE_REG(vis, ((LIBGGI_GC(vis)->clipbr.y - 1) * LIBGGI_VIRTX(vis))
		     & 0x00FFFFFF, YBOT);
    }
  /* TODO: Should we also update the plane_mask mask (PLNWT)? */

  GX00_CONTEXT(vis)->hwgc_mask = 0;
}

void GGI_kgi_Gx00_gcchanged(ggi_visual *vis, int mask)
{
  GX00_CONTEXT(vis)->hwgc_mask |= mask;
}

#if 0
/* Initial direct implementation: performance problems wrt DirectBuffer
 * accesses.
 */
void GGI_kgi_Gx00_gcchanged(ggi_visual *vis, int mask)
{
  if (mask & GGI_GCCHANGED_FG) {
    uint32_t fcol = LIBGGI_GC_FGCOLOR(vis);
    if (LIBGGI_GT(vis) == GT_15BIT) {
      /* In 5:5:5, bit15 originates from fcol<31> */
      fcol &= ~(1 << 31);
      fcol |= (fcol & (1 << 15)) << (31 - 15);
    }
    GX00_WRITE_REG(vis, fcol, FCOL);
  }
  if (mask & GGI_GCCHANGED_BG)
    GX00_WRITE_REG(vis, LIBGGI_GC_BGCOLOR(vis), BCOL);
  if (mask & GGI_GCCHANGED_CLIP)
    {
      GX00_WRITE_REG(vis,
		     ((LIBGGI_GC(vis)->cliptl.x << CXBNDRY_CXLEFT_SHIFT) 
		      & CXBNDRY_CXLEFT_MASK)
		     | (((LIBGGI_GC(vis)->clipbr.x - 1) << CXBNDRY_CXRIGHT_SHIFT) 
			& CXBNDRY_CXRIGHT_MASK),
		     CXBNDRY);
      /* NB: We assume YDSTORG to 0 (i.e. use of DSTORG) */
      GX00_WRITE_REG(vis, (LIBGGI_GC(vis)->cliptl.y * LIBGGI_VIRTX(vis))
		     & 0x00FFFFFF, YTOP);
      GX00_WRITE_REG(vis, ((LIBGGI_GC(vis)->clipbr.y - 1) * LIBGGI_VIRTX(vis))
		     & 0x00FFFFFF, YBOT);
    }
  /* TODO: Should we also update the plane_mask mask (PLNWT)? */
}
#endif
