/* $Id: visual.c,v 1.9 2008/01/20 19:26:25 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]

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

#include "lin24lib.h"


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	/* Frame handling
	 */

	vis->opdraw->setreadframe  = _ggi_default_setreadframe;
	vis->opdraw->setwriteframe = _ggi_default_setwriteframe;
	
	/* Generic drawing
	 */
	if (vis->needidleaccel) {
		vis->opdraw->drawpixel_nc	= GGI_lin24_drawpixel_nca;
		vis->opdraw->drawpixel		= GGI_lin24_drawpixela;
		vis->opdraw->putpixel_nc	= GGI_lin24_putpixel_nca;
		vis->opdraw->putpixel		= GGI_lin24_putpixela;
		vis->opdraw->getpixel_nc	= GGI_lin24_getpixel_nca;
	} else {
		vis->opdraw->drawpixel_nc	= GGI_lin24_drawpixel_nc;
		vis->opdraw->drawpixel		= GGI_lin24_drawpixel;
		vis->opdraw->putpixel_nc	= GGI_lin24_putpixel_nc;
		vis->opdraw->putpixel		= GGI_lin24_putpixel;
		vis->opdraw->getpixel_nc	= GGI_lin24_getpixel_nc;
	}

	vis->opdraw->drawhline_nc	= GGI_lin24_drawhline_nc;
	vis->opdraw->drawhline	= GGI_lin24_drawhline;
	vis->opdraw->puthline	= GGI_lin24_puthline;
	vis->opdraw->gethline	= GGI_lin24_gethline;

	vis->opdraw->drawvline_nc	= GGI_lin24_drawvline_nc;
	vis->opdraw->drawvline	= GGI_lin24_drawvline;
	vis->opdraw->putvline	= GGI_lin24_putvline;
	vis->opdraw->getvline	= GGI_lin24_getvline;

	vis->opdraw->drawbox		= GGI_lin24_drawbox;
	vis->opdraw->putbox		= GGI_lin24_putbox;
	vis->opdraw->copybox		= GGI_lin24_copybox;
	vis->opdraw->crossblit	= GGI_lin24_crossblit;

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_linear_24(int func, void **funcptr);

int GGIdl_linear_24(int func, void **funcptr)
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
