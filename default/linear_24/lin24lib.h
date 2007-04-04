/* $Id: lin24lib.h,v 1.3 2007/04/04 13:58:21 ggibecka Exp $
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

ggifunc_drawpixel_nc	GGI_lin24_drawpixel_nc;
ggifunc_drawpixel	GGI_lin24_drawpixel;
ggifunc_putpixel_nc	GGI_lin24_putpixel_nc;
ggifunc_putpixel	GGI_lin24_putpixel;
ggifunc_getpixel	GGI_lin24_getpixel;

ggifunc_drawpixel_nc	GGI_lin24_drawpixel_nca;
ggifunc_drawpixel	GGI_lin24_drawpixela;
ggifunc_putpixel_nc	GGI_lin24_putpixel_nca;
ggifunc_putpixel	GGI_lin24_putpixela;
ggifunc_getpixel	GGI_lin24_getpixela;

ggifunc_drawhline_nc	GGI_lin24_drawhline_nc;
ggifunc_drawhline	GGI_lin24_drawhline;
ggifunc_puthline	GGI_lin24_puthline;
ggifunc_gethline_nc	GGI_lin24_gethline_nc;
ggifunc_gethline	GGI_lin24_gethline;

ggifunc_drawvline_nc	GGI_lin24_drawvline_nc;
ggifunc_drawvline	GGI_lin24_drawvline;
ggifunc_putvline	GGI_lin24_putvline;
ggifunc_getvline	GGI_lin24_getvline;

ggifunc_crossblit	GGI_lin24_crossblit;
ggifunc_copybox		GGI_lin24_copybox;
ggifunc_drawbox		GGI_lin24_drawbox;
ggifunc_putbox		GGI_lin24_putbox;
