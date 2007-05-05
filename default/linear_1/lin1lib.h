/* $Id: lin1lib.h,v 1.4 2007/05/05 18:19:09 pekberg Exp $
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

ggifunc_putc		GGI_lin1_putc;

ggifunc_mapcolor	GGI_lin1_mapcolor;
ggifunc_unmappixel	GGI_lin1_unmappixel;
ggifunc_packcolors	GGI_lin1_packcolors;
ggifunc_unpackpixels	GGI_lin1_unpackpixels;

ggifunc_drawpixel_nc	GGI_lin1_drawpixel_nc;
ggifunc_drawpixel	GGI_lin1_drawpixel;
ggifunc_putpixel_nc	GGI_lin1_putpixel_nc;
ggifunc_putpixel	GGI_lin1_putpixel;
ggifunc_getpixel	GGI_lin1_getpixel;

ggifunc_drawpixel_nc	GGI_lin1_drawpixel_nca;
ggifunc_drawpixel	GGI_lin1_drawpixela;
ggifunc_putpixel_nc	GGI_lin1_putpixel_nca;
ggifunc_putpixel	GGI_lin1_putpixela;
ggifunc_getpixel	GGI_lin1_getpixela;

ggifunc_drawhline_nc	GGI_lin1_drawhline_nc;
ggifunc_drawhline	GGI_lin1_drawhline;
ggifunc_puthline	GGI_lin1_packed_puthline;
ggifunc_gethline_nc	GGI_lin1_packed_gethline_nc;
ggifunc_gethline	GGI_lin1_packed_gethline;
ggifunc_puthline	GGI_lin1_unpacked_puthline;
ggifunc_gethline	GGI_lin1_unpacked_gethline;

ggifunc_drawvline_nc	GGI_lin1_drawvline_nc;
ggifunc_drawvline	GGI_lin1_drawvline;
ggifunc_putvline	GGI_lin1_packed_putvline;
ggifunc_getvline	GGI_lin1_packed_getvline;
ggifunc_putvline	GGI_lin1_unpacked_putvline;
ggifunc_getvline	GGI_lin1_unpacked_getvline;

ggifunc_copybox		GGI_lin1_copybox;
