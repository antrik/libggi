/* $Id: t32lib.h,v 1.3 2008/01/20 19:26:38 pekberg Exp $
******************************************************************************

   text-32 sublib function prototypes

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

ggifunc_mapcolor	GGI_t32_mapcolor;
ggifunc_unmappixel	GGI_t32_unmappixel;

ggifunc_getcharsize	GGI_t32_getcharsize;
ggifunc_putc		GGI_t32_putc;

ggifunc_drawpixel_nc	GGI_t32_drawpixel_nc;
ggifunc_drawpixel	GGI_t32_drawpixel;
ggifunc_putpixel_nc	GGI_t32_putpixel_nc;
ggifunc_putpixel	GGI_t32_putpixel;
ggifunc_getpixel_nc	GGI_t32_getpixel_nc;
ggifunc_getpixel	GGI_t32_getpixel;

ggifunc_drawpixel_nc	GGI_t32_drawpixel_nca;
ggifunc_drawpixel	GGI_t32_drawpixela;
ggifunc_putpixel_nc	GGI_t32_putpixel_nca;
ggifunc_putpixel	GGI_t32_putpixela;
ggifunc_getpixel_nc	GGI_t32_getpixel_nca;
ggifunc_getpixel	GGI_t32_getpixela;
