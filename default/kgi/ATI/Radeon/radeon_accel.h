/* $Id: radeon_accel.h,v 1.4 2002/11/30 14:53:51 fspacek Exp $
******************************************************************************

   ATI Radeon sublib function prototypes

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
#ifndef _RADEON_ACCEL_H
#define _RADEON_ACCEL_H

#include <ggi/internal/ggi-dl.h>

#include "kgi/config.h"
#include <ggi/display/kgi.h>
#include "radeon_cce.h"

typedef struct
{

	ggi_accel_t *accel;
	uint32 dst_type;
	
} radeon_context_t;

#define RADEON_CONTEXT(vis) ((radeon_context_t *)KGI_ACCEL_PRIV(vis))
#define RADEON_ACCEL(vis) (RADEON_CONTEXT(vis)->accel)

#define RADEON_BUFFER_SIZE_ORDER  1
#define RADEON_BUFFER_SIZE        (0x1000 << RADEON_BUFFER_SIZE_ORDER)
#define RADEON_BUFFER_MASK        (RADEON_BUFFER_SIZE - 1)
#define RADEON_BUFFER_SIZE32      (RADEON_BUFFER_SIZE >> 2)
#define RADEON_BUFFER_MASK32      (RADEON_BUFFER_SIZE32 - 1)

#define RADEON_BUFFER_NUM         16

#define RADEON_TOTAL_SIZE         (RADEON_BUFFER_SIZE * RADEON_BUFFER_NUM)
#define RADEON_TOTAL_SIZE32       (RADEON_BUFFER_SIZE32 * RADEON_BUFFER_NUM)

#define RADEON_FLUSH(vis) \
GGI_ACCEL_FLUSH_u32(RADEON_ACCEL(vis), RADEON_BUFFER_SIZE32, RADEON_TOTAL_SIZE32)

#define RADEON_CHECK(vis, n) \
GGI_ACCEL_CHECK_TOTAL_u32(RADEON_ACCEL(vis), n, RADEON_BUFFER_SIZE32, RADEON_TOTAL_SIZE32)

#define RADEON_WRITE(vis, val) \
GGI_ACCEL_WRITE_u32(RADEON_ACCEL(vis), val)

#define RADEON_WRITEPACKET(vis, data)\
	{\
		uint32 i = sizeof(data) / 4;\
		uint32 *ptr = (uint32*)&data;\
		RADEON_CHECK(vis, sizeof(data) / 4);\
		while (i--)\
			RADEON_WRITE(vis, *ptr++);\
	}
	
ggifunc_drawhline   GGI_kgi_radeon_drawhline;
ggifunc_drawvline   GGI_kgi_radeon_drawvline;
ggifunc_drawline    GGI_kgi_radeon_drawline;
ggifunc_drawbox	    GGI_kgi_radeon_drawbox;
ggifunc_copybox     GGI_kgi_radeon_copybox;
ggifunc_gcchanged   GGI_kgi_radeon_gcchanged;
ggifunc_putc        GGI_kgi_radeon_putc;
ggifunc_puts        GGI_kgi_radeon_puts;
ggifunc_getcharsize GGI_kgi_radeon_getcharsize;

#endif