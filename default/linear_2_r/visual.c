/* $Id: visual.c,v 1.3 2007/01/23 15:43:46 pekberg Exp $
******************************************************************************

   Linear 2 bit graphics (high-pair-right)

   Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]
   Copyright (C) 2007 Peter Rosin    [peda@lysator.liu.se]

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

#include "lin2rlib.h"


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	/* Frame handling
	 */

	vis->opdraw->setreadframe	= _ggi_default_setreadframe;
	vis->opdraw->setwriteframe	= _ggi_default_setwriteframe;
	
	/* Generic drawing
	 */

	 if (vis->needidleaccel) {
		vis->opdraw->putpixel_nc	= GGI_lin2r_putpixel_nca;
		vis->opdraw->putpixel		= GGI_lin2r_putpixela;
		vis->opdraw->drawpixel_nc	= GGI_lin2r_drawpixel_nca;
		vis->opdraw->drawpixel		= GGI_lin2r_drawpixela;
		vis->opdraw->getpixel		= GGI_lin2r_getpixela;
	} else {
		vis->opdraw->putpixel_nc	= GGI_lin2r_putpixel_nc;
		vis->opdraw->putpixel		= GGI_lin2r_putpixel;
		vis->opdraw->drawpixel_nc	= GGI_lin2r_drawpixel_nc;
		vis->opdraw->drawpixel		= GGI_lin2r_drawpixel;
		vis->opdraw->getpixel		= GGI_lin2r_getpixel;
	}

	vis->opdraw->drawhline_nc	= GGI_lin2r_drawhline_nc;
	vis->opdraw->drawhline		= GGI_lin2r_drawhline;

	vis->opdraw->drawvline_nc	= GGI_lin2r_drawvline_nc;
	vis->opdraw->drawvline		= GGI_lin2r_drawvline;

	if (GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT) {
		vis->opdraw->puthline		= GGI_lin2r_packed_puthline;
		vis->opdraw->gethline		= GGI_lin2r_packed_gethline;

		vis->opdraw->putvline		= GGI_lin2r_packed_putvline;
		vis->opdraw->getvline		= GGI_lin2r_packed_getvline;
	}

	*dlret = GGI_DL_OPCOLOR | GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_linear_2_r(int func, void **funcptr);

int GGIdl_linear_2_r(int func, void **funcptr)
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
