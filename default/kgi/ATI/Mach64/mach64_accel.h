/* $Id: mach64_accel.h,v 1.2 2002/11/30 14:51:13 fspacek Exp $
******************************************************************************

   ATI Mach64 sublib function prototypes

   Copyright (C) 2002 Paul Redmond

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
#ifndef _MACH64LIB_H
#define _MACH64LIB_H

#include <ggi/internal/ggi-dl.h>

#include "kgi/config.h"
#include <ggi/display/kgi.h>
#include "MACH64.h"

#define MACH64_ACCEL(vis) ((ggi_accel_t *)KGI_ACCEL_PRIV(vis))

#define MACH64_BUFFER_SIZE_ORDER  1
#define MACH64_BUFFER_SIZE        (0x1000 << MACH64_BUFFER_SIZE_ORDER)
#define MACH64_BUFFER_MASK        (MACH64_BUFFER_SIZE - 1)
#define MACH64_BUFFER_SIZE32      (MACH64_BUFFER_SIZE >> 2)
#define MACH64_BUFFER_MASK32      (MACH64_BUFFER_SIZE32 - 1)

#define MACH64_BUFFER_NUM         16

#define MACH64_TOTAL_SIZE         (MACH64_BUFFER_SIZE * MACH64_BUFFER_NUM)
#define MACH64_TOTAL_SIZE32       (MACH64_BUFFER_SIZE32 * MACH64_BUFFER_NUM)

#define MACH64_FLUSH(vis) \
GGI_ACCEL_FLUSH_u32(MACH64_ACCEL(vis), MACH64_BUFFER_SIZE32, MACH64_TOTAL_SIZE32)

#define MACH64_CHECK(vis, n) \
GGI_ACCEL_CHECK_TOTAL_u32(MACH64_ACCEL(vis), n, MACH64_BUFFER_SIZE32, MACH64_TOTAL_SIZE32)

#define MACH64_WRITE(vis, val) \
GGI_ACCEL_WRITE_u32(MACH64_ACCEL(vis), val)

ggifunc_drawhline GGI_kgi_mach64_drawhline;
ggifunc_drawvline GGI_kgi_mach64_drawvline;
ggifunc_drawline  GGI_kgi_mach64_drawline;
ggifunc_drawbox	  GGI_kgi_mach64_drawbox;
ggifunc_copybox   GGI_kgi_mach64_copybox;
ggifunc_gcchanged GGI_kgi_mach64_gcchanged;

#endif