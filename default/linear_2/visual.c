/* $Id: visual.c,v 1.12 2008/01/20 19:26:24 pekberg Exp $
******************************************************************************

  Linear 2 bit graphics (high-pair-left)

  Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]

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

#include "lin2lib.h"


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	/* Color mapping 
	 */
	if (GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT) {
		vis->opcolor->packcolors	= GGI_lin2_packcolors;
		vis->opcolor->unpackpixels	= GGI_lin2_unpackpixels;
	}

	/* Frame handling
	 */

	vis->opdraw->setreadframe	= _ggi_default_setreadframe;
	vis->opdraw->setwriteframe	= _ggi_default_setwriteframe;
	
	/* Drawing ops
	 */
	if (vis->needidleaccel) {
		vis->opdraw->putpixel_nc	= GGI_lin2_putpixel_nca;
		vis->opdraw->getpixel_nc	= GGI_lin2_getpixel_nca;
	} else {
		vis->opdraw->putpixel_nc	= GGI_lin2_putpixel_nc;
		vis->opdraw->getpixel_nc	= GGI_lin2_getpixel_nc;
	}

	vis->opdraw->drawhline		= GGI_lin2_drawhline;
	vis->opdraw->drawhline_nc	= GGI_lin2_drawhline_nc;

	vis->opdraw->drawvline		= GGI_lin2_drawvline;
	vis->opdraw->drawvline_nc	= GGI_lin2_drawvline_nc;

	if (GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT) {
		vis->opdraw->puthline		= GGI_lin2_packed_puthline;
		vis->opdraw->gethline		= GGI_lin2_packed_gethline;

		vis->opdraw->putvline		= GGI_lin2_packed_putvline;
		vis->opdraw->getvline		= GGI_lin2_packed_getvline;
	}

	*dlret = GGI_DL_OPCOLOR | GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_linear_2(int func, void **funcptr);

int GGIdl_linear_2(int func, void **funcptr)
{
	ggifunc_open **openptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
	case GGIFUNC_close:
		*funcptr = NULL;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
