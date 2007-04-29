/* $Id: stublib.h,v 1.3 2007/04/29 07:02:41 cegger Exp $
******************************************************************************

   Generic drawing library

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>

ggifunc_puthline	_GGI_stubs_L1_puthline;
ggifunc_puthline	_GGI_stubs_L2_puthline;
ggifunc_puthline	_GGI_stubs_L3_puthline;
ggifunc_puthline	_GGI_stubs_L4_puthline;

ggifunc_putvline	_GGI_stubs_L1_putvline;
ggifunc_putvline	_GGI_stubs_L2_putvline;
ggifunc_putvline	_GGI_stubs_L3_putvline;
ggifunc_putvline	_GGI_stubs_L4_putvline;

ggifunc_gethline	_GGI_stubs_L1_gethline;
ggifunc_gethline	_GGI_stubs_L2_gethline;
ggifunc_gethline	_GGI_stubs_L3_gethline;
ggifunc_gethline	_GGI_stubs_L4_gethline;

ggifunc_gethline	_GGI_stubs_L1_gethline_nc;
ggifunc_gethline	_GGI_stubs_L2_gethline_nc;
ggifunc_gethline	_GGI_stubs_L3_gethline_nc;
ggifunc_gethline	_GGI_stubs_L4_gethline_nc;

ggifunc_getvline	_GGI_stubs_L1_getvline;
ggifunc_getvline	_GGI_stubs_L2_getvline;
ggifunc_getvline	_GGI_stubs_L3_getvline;
ggifunc_getvline	_GGI_stubs_L4_getvline;

ggifunc_putc		GGI_stubs_putc;
ggifunc_puts		GGI_stubs_puts;
ggifunc_getcharsize	GGI_stubs_getcharsize;

ggifunc_putpixel	GGI_stubs_putpixel;
ggifunc_drawpixel	GGI_stubs_drawpixel;
ggifunc_drawpixel	GGI_stubs_drawpixel_nc;
ggifunc_drawhline	GGI_stubs_drawhline;
ggifunc_drawhline	GGI_stubs_drawhline_nc;
ggifunc_drawvline	GGI_stubs_drawvline;
ggifunc_drawvline	GGI_stubs_drawvline_nc;
ggifunc_drawbox		GGI_stubs_drawbox;
ggifunc_drawline	GGI_stubs_drawline;

ggifunc_putbox		GGI_stubs_putbox;
ggifunc_getbox		GGI_stubs_getbox;

ggifunc_copybox		GGI_stubs_copybox;
ggifunc_crossblit	GGI_stubs_crossblit;
ggifunc_fillscreen	GGI_stubs_fillscreen;
