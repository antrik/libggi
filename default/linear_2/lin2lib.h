/* $Id: lin2lib.h,v 1.2 2002/09/08 21:37:43 soyt Exp $
******************************************************************************

   linear-2 sublib function prototypes

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

ggifunc_drawpixel_nc	GGI_lin2_drawpixel_nc;
ggifunc_drawpixel	GGI_lin2_drawpixel;
ggifunc_putpixel_nc	GGI_lin2_putpixel_nc;
ggifunc_putpixel	GGI_lin2_putpixel;
ggifunc_getpixel	GGI_lin2_getpixel;

ggifunc_drawpixel_nc	GGI_lin2_drawpixel_nca;
ggifunc_drawpixel	GGI_lin2_drawpixela;
ggifunc_putpixel_nc	GGI_lin2_putpixel_nca;
ggifunc_putpixel	GGI_lin2_putpixela;
ggifunc_getpixel	GGI_lin2_getpixela;
