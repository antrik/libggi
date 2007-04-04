/* $Id: lin4lib.h,v 1.4 2007/04/04 17:30:50 ggibecka Exp $
******************************************************************************

   linear-4 sublib function prototypes

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

ggifunc_mapcolor	GGI_lin4_mapcolor;
ggifunc_unmappixel	GGI_lin4_unmappixel;
ggifunc_packcolors	GGI_lin4_packcolors;
ggifunc_unpackpixels	GGI_lin4_unpackpixels;

ggifunc_drawpixel_nc	GGI_lin4_drawpixel_nc;
ggifunc_drawpixel	GGI_lin4_drawpixel;
ggifunc_putpixel_nc	GGI_lin4_putpixel_nc;
ggifunc_putpixel	GGI_lin4_putpixel;
ggifunc_getpixel	GGI_lin4_getpixel;

ggifunc_drawpixel_nc	GGI_lin4_drawpixel_nca;
ggifunc_drawpixel	GGI_lin4_drawpixela;
ggifunc_putpixel_nc	GGI_lin4_putpixel_nca;
ggifunc_putpixel	GGI_lin4_putpixela;
ggifunc_getpixel	GGI_lin4_getpixela;

ggifunc_drawhline_nc	GGI_lin4_drawhline_nc;
ggifunc_drawhline	GGI_lin4_drawhline;
ggifunc_puthline	GGI_lin4_packed_puthline;
ggifunc_gethline_nc	GGI_lin4_packed_gethline_nc;
ggifunc_gethline	GGI_lin4_packed_gethline;
ggifunc_puthline	GGI_lin4_unpacked_puthline;
ggifunc_gethline	GGI_lin4_unpacked_gethline;

ggifunc_drawvline_nc	GGI_lin4_drawvline_nc;
ggifunc_drawvline	GGI_lin4_drawvline;
ggifunc_putvline	GGI_lin4_packed_putvline;
ggifunc_getvline	GGI_lin4_packed_getvline;
ggifunc_putvline	GGI_lin4_unpacked_putvline;
ggifunc_getvline	GGI_lin4_unpacked_getvline;

ggifunc_copybox		GGI_lin4_copybox;
