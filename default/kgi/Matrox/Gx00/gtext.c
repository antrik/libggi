/* $Id: gtext.c,v 1.1 2002/12/23 13:17:35 ortalo Exp $
******************************************************************************

   Matrox Gx00 text acceleration

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
#include <ggi/internal/ggi-dl.h>
#include "Gx00.h"
#include "Gx00_accel.h"

#define GX00_FONTSIZE 8

int GGI_kgi_Gx00_putc(ggi_visual *vis, int x, int y, char c)
{
  uint32 *cg = (uint32*)(font + ((uint8)c<<3));

  GGI_kgi_Gx00_updatehwgc(vis);

  GX00_WRITE_DSTORG(vis, vis->w_frame_num * LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));

  /* We draw the character via a patterned draw box */
  GX00_WRITE_REG(vis, (*cg), PAT0);
  GX00_WRITE_REG(vis, (*(cg+1)), PAT1);

  GX00_WRITE_DWGCTL(vis,
		    DWGCTL_OPCOD_TRAP | DWGCTL_ATYPE_RSTR | DWGCTL_ZMODE_NOZCMP
		    | DWGCTL_ARZERO | DWGCTL_SGNZERO
		    | ((0xC << DWGCTL_BOP_SHIFT) & DWGCTL_BOP_MASK) /* ROP ! */
		    | ((0x0 << DWGCTL_TRANS_SHIFT) & DWGCTL_TRANS_MASK) /* trans.! */
		    | DWGCTL_BLTMOD_BMONOLEF
		    /* patterning disabled */
		    /* translucency disabled */
		    /* clipping enabled */
		    );
  /* Align the pattern */
  GX00_WRITE_REG(vis, ((-x) & 7) | (((-y) & 7) << 4), SHIFT);
  GX00_WRITE_REG(vis, (x & 0xFFFF) | (((x + GX00_FONTSIZE) & 0xFFFF) << 16),
		 FXBNDRY);
  GX00_WRITE_REG(vis, (GX00_FONTSIZE) | ((y & 0xFFFF) << 16),
		 YDSTLEN | ACCEL_GO);
	
  return 0;
}

int GGI_kgi_Gx00_puts(ggi_visual *vis, int x, int y, const char *string)
{
  /* TODO: Implement if it really seems useful */

  return 0;
}

int GGI_kgi_Gx00_getcharsize(ggi_visual *vis, int *width, int *height)
{
  *width = *height = GX00_FONTSIZE;
  
  return 0;
}
