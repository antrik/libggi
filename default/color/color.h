/* $Id: color.h,v 1.3 2002/09/08 21:37:42 soyt Exp $
******************************************************************************

   Generic color library defines

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
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

#ifndef _LIBGGI_DEFAULT_COLOR_H
#define _LIBGGI_DEFAULT_COLOR_H

#include "config.h"
#include <ggi/internal/ggi-dl.h>

ggifunc_mapcolor	GGI_color_PAL_mapcolor;
ggifunc_mapcolor	GGI_color_TRUE_mapcolor;
ggifunc_mapcolor	GGI_color_GREY_mapcolor;

ggifunc_unmappixel	GGI_color_PAL_unmappixel;
ggifunc_unmappixel	GGI_color_TRUE_unmappixel_gte8;
ggifunc_unmappixel	GGI_color_TRUE_unmappixel_gte4;
ggifunc_unmappixel	GGI_color_TRUE_unmappixel_gte2;
ggifunc_unmappixel	GGI_color_TRUE_unmappixel_gte1;
ggifunc_unmappixel	GGI_color_GREY_unmappixel;

ggifunc_packcolors	GGI_color_L1_packcolors;
ggifunc_packcolors	GGI_color_L2_packcolors;
ggifunc_packcolors	GGI_color_L3_packcolors;
ggifunc_packcolors	GGI_color_L4_packcolors;

ggifunc_unpackpixels	GGI_color_L1_unpackpixels;
ggifunc_unpackpixels	GGI_color_L2_unpackpixels;
ggifunc_unpackpixels	GGI_color_L3_unpackpixels;
ggifunc_unpackpixels	GGI_color_L4_unpackpixels;

ggifunc_getpalvec	GGI_color_getpalvec;
ggifunc_getgamma	GGI_color_getgamma;	
ggifunc_setgamma	GGI_color_setgamma;	

typedef struct {
	/* total = length(mask) + shift(mask)
	 */
	int red_map;
	int red_unmap;
	int red_mask;
	int red_nbits;
	int green_map;
	int green_unmap;
	int green_mask;
	int green_nbits;
	int blue_map;
	int blue_unmap;
	int blue_mask;
	int blue_nbits;
} color_truepriv;

typedef struct {
	int numcols;
	ggi_color prev_col;
	ggi_pixel prev_val;
} color_palpriv;

typedef struct {
	int shift;
} color_greypriv;


typedef union {
	color_truepriv truecol;
	color_palpriv  pal;
	color_greypriv grey;
} color_priv;

#define COLOR_TRUEPRIV(vis)	((color_truepriv*)((vis)->colorpriv))
#define COLOR_PALPRIV(vis)	((color_palpriv*)((vis)->colorpriv))
#define COLOR_GREYPRIV(vis)	((color_greypriv*)((vis)->colorpriv))

#endif /* _LIBGGI_DEFAULT_COLOR_H */
