/* $Id: lin32lib.h,v 1.4 2008/01/20 19:26:27 pekberg Exp $
******************************************************************************

   Generic drawing library

   Copyright (C) 1999 Marcus Sundberg	[marcus@ggi-project.org]

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

ggifunc_putc		GGI_lin32_putc;

ggifunc_drawpixel_nc	GGI_lin32_drawpixel_nc;
ggifunc_drawpixel	GGI_lin32_drawpixel;
ggifunc_putpixel_nc	GGI_lin32_putpixel_nc;
ggifunc_putpixel	GGI_lin32_putpixel;
ggifunc_getpixel_nc	GGI_lin32_getpixel_nc;
ggifunc_getpixel	GGI_lin32_getpixel;

ggifunc_drawpixel_nc	GGI_lin32_drawpixel_nca;
ggifunc_drawpixel	GGI_lin32_drawpixela;
ggifunc_putpixel_nc	GGI_lin32_putpixel_nca;
ggifunc_putpixel	GGI_lin32_putpixela;
ggifunc_getpixel_nc	GGI_lin32_getpixel_nca;
ggifunc_getpixel	GGI_lin32_getpixela;

ggifunc_drawhline_nc	GGI_lin32_drawhline_nc;
ggifunc_drawhline	GGI_lin32_drawhline;
ggifunc_puthline	GGI_lin32_puthline;
ggifunc_gethline_nc	GGI_lin32_gethline_nc;
ggifunc_gethline	GGI_lin32_gethline;

ggifunc_drawvline_nc	GGI_lin32_drawvline_nc;
ggifunc_drawvline	GGI_lin32_drawvline;
ggifunc_putvline	GGI_lin32_putvline;
ggifunc_getvline	GGI_lin32_getvline;

ggifunc_drawline	GGI_lin32_drawline;

ggifunc_copybox		GGI_lin32_copybox;
ggifunc_crossblit	GGI_lin32_crossblit;
ggifunc_drawbox		GGI_lin32_drawbox;
ggifunc_putbox		GGI_lin32_putbox;
