/* $Id: vgaglvis.h,v 1.2 2005/07/31 15:30:37 soyt Exp $
******************************************************************************

   SVGAlib target vgagl helper

   Copyright (C) 1998 Marcus Sundberg   [marcus@ggi-project.org]

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

#include <vga.h>
#include <vgagl.h>

/* Prototypes
 */

ggifunc_drawpixel_nc	GGI_vgagl_drawpixel_nc;
ggifunc_drawpixel	GGI_vgagl_drawpixel;
ggifunc_putpixel_nc	GGI_vgagl_putpixel_nc;
ggifunc_putpixel	GGI_vgagl_putpixel;
ggifunc_getpixel	GGI_vgagl_getpixel;
ggifunc_drawhline_nc	GGI_vgagl_drawhline_nc;
ggifunc_drawhline	GGI_vgagl_drawhline;
ggifunc_gethline	GGI_vgagl_gethline;
ggifunc_puthline	GGI_vgagl_puthline;
ggifunc_drawvline_nc	GGI_vgagl_drawvline_nc;
ggifunc_drawvline	GGI_vgagl_drawvline;
ggifunc_getvline	GGI_vgagl_getvline;
ggifunc_putvline	GGI_vgagl_putvline;
ggifunc_drawbox		GGI_vgagl_drawbox;
ggifunc_putbox		GGI_vgagl_putbox;
ggifunc_getbox		GGI_vgagl_getbox;
ggifunc_fillscreen	GGI_vgagl_fillscreen;
