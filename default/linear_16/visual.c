/* $Id: visual.c,v 1.12 2007/02/25 18:13:11 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]

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
#include "lin16lib.h"

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	gg_swartype swar;
	/* Frame handling
	 */

	vis->opdraw->setreadframe  = _ggi_default_setreadframe;
	vis->opdraw->setwriteframe = _ggi_default_setwriteframe;
	
	/* Generic drawing
	 */
	if (vis->needidleaccel) {
		vis->opdraw->drawpixel_nc	= GGI_lin16_drawpixel_nca;
		vis->opdraw->drawpixel		= GGI_lin16_drawpixela;
		vis->opdraw->putpixel_nc	= GGI_lin16_putpixel_nca;
		vis->opdraw->putpixel		= GGI_lin16_putpixela;
		vis->opdraw->getpixel		= GGI_lin16_getpixela;
	} else {
		vis->opdraw->drawpixel_nc	= GGI_lin16_drawpixel_nc;
		vis->opdraw->drawpixel		= GGI_lin16_drawpixel;
		vis->opdraw->putpixel_nc	= GGI_lin16_putpixel_nc;
		vis->opdraw->putpixel		= GGI_lin16_putpixel;
		vis->opdraw->getpixel		= GGI_lin16_getpixel;
	}

	vis->opdraw->drawhline		= GGI_lin16_drawhline;
	vis->opdraw->drawhline_nc	= GGI_lin16_drawhline_nc;
	vis->opdraw->puthline		= GGI_lin16_puthline;
	vis->opdraw->gethline		= GGI_lin16_gethline;

	vis->opdraw->drawvline		= GGI_lin16_drawvline;
	vis->opdraw->drawvline_nc	= GGI_lin16_drawvline_nc;
	vis->opdraw->putvline		= GGI_lin16_putvline;
	vis->opdraw->getvline		= GGI_lin16_getvline;

	vis->opdraw->drawline		= GGI_lin16_drawline;

	vis->opdraw->drawbox		= GGI_lin16_drawbox;
	vis->opdraw->putbox		= GGI_lin16_putbox;
	vis->opdraw->copybox		= GGI_lin16_copybox;

	swar = _ggiGetSwarType();

	vis->opdraw->crossblit		= NULL;

#ifdef DO_SWAR_NONE
	if (swar & GG_SWAR_NONE) 
		vis->opdraw->crossblit	= GGI_lin16_crossblit;
#endif
#ifdef DO_SWAR_64BITC
	if (swar & GG_SWAR_64BITC)
		vis->opdraw->crossblit	= GGI_lin16_crossblit_64bitc;
#endif
#ifdef DO_SWAR_MMX
	if (swar & GG_SWAR_MMX) 
		vis->opdraw->crossblit	= GGI_lin16_crossblit_mmx;
#endif

	if (vis->opdraw->crossblit == NULL) {
		fprintf(stderr, "linear_16: No acceptible SWAR.  Aborting.\n");
		return GGI_ENOFUNC;
	}

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_linear_16(int func, void **funcptr);

int GGIdl_linear_16(int func, void **funcptr)
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
