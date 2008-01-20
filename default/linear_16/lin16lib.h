/* $Id: lin16lib.h,v 1.7 2008/01/20 19:26:23 pekberg Exp $
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

#include <ggi/internal/ggi-dl.h>

ggifunc_putc		GGI_lin16_putc;

ggifunc_drawpixel_nc	GGI_lin16_drawpixel_nc;
ggifunc_drawpixel	GGI_lin16_drawpixel;
ggifunc_putpixel_nc	GGI_lin16_putpixel_nc;
ggifunc_putpixel	GGI_lin16_putpixel;
ggifunc_getpixel_nc	GGI_lin16_getpixel_nc;
ggifunc_getpixel	GGI_lin16_getpixel;

ggifunc_drawpixel_nc	GGI_lin16_drawpixel_nca;
ggifunc_drawpixel	GGI_lin16_drawpixela;
ggifunc_putpixel_nc	GGI_lin16_putpixel_nca;
ggifunc_putpixel	GGI_lin16_putpixela;
ggifunc_getpixel_nc	GGI_lin16_getpixel_nca;
ggifunc_getpixel	GGI_lin16_getpixela;

ggifunc_drawhline_nc	GGI_lin16_drawhline_nc;
ggifunc_drawhline	GGI_lin16_drawhline;
ggifunc_puthline	GGI_lin16_puthline;
ggifunc_gethline_nc	GGI_lin16_gethline_nc;
ggifunc_gethline	GGI_lin16_gethline;

ggifunc_drawvline_nc	GGI_lin16_drawvline_nc;
ggifunc_drawvline	GGI_lin16_drawvline;
ggifunc_putvline	GGI_lin16_putvline;
ggifunc_getvline	GGI_lin16_getvline;

ggifunc_drawline	GGI_lin16_drawline;

ggifunc_copybox		GGI_lin16_copybox;
ggifunc_drawbox		GGI_lin16_drawbox;
ggifunc_putbox		GGI_lin16_putbox;

#ifdef DO_SWAR_MMX
# ifndef HAVE_LIN16_SWAR
# define HAVE_LIN16_SWAR
# endif
ggifunc_crossblit	GGI_lin16_crossblit_mmx;
#endif

#ifdef DO_SWAR_64BITC
# ifndef HAVE_LIN16_SWAR
# define HAVE_LIN16_SWAR
# endif
ggifunc_crossblit	GGI_lin16_crossblit_64bitc;
#endif

#ifndef HAVE_LIN16_SWAR
/* Define DO_SWAR_NONE locally for this sublib */
# ifndef DO_SWAR_NONE
# define DO_SWAR_NONE
# endif
#endif

#ifdef DO_SWAR_NONE
ggifunc_crossblit	GGI_lin16_crossblit;
#endif

