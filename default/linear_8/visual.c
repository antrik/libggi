/* $Id: visual.c,v 1.4 2004/11/06 22:48:23 cegger Exp $
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

#include "lin8lib.h"

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	/* Frame handling
	 */

	vis->opdraw->setreadframe  = _ggi_default_setreadframe;
	vis->opdraw->setwriteframe = _ggi_default_setwriteframe;
	
	/* Generic drawing
	 */
        vis->opdraw->putc	= GGI_lin8_putc;

	if (vis->needidleaccel) {
		vis->opdraw->drawpixel_nc	= GGI_lin8_drawpixel_nca;
		vis->opdraw->drawpixel	= GGI_lin8_drawpixela;
		vis->opdraw->putpixel_nc	= GGI_lin8_putpixel_nca;
		vis->opdraw->putpixel	= GGI_lin8_putpixela;
		vis->opdraw->getpixel	= GGI_lin8_getpixela;
	} else {
		vis->opdraw->drawpixel_nc	= GGI_lin8_drawpixel_nc;
		vis->opdraw->drawpixel	= GGI_lin8_drawpixel;
		vis->opdraw->putpixel_nc	= GGI_lin8_putpixel_nc;
		vis->opdraw->putpixel	= GGI_lin8_putpixel;
		vis->opdraw->getpixel	= GGI_lin8_getpixel;
	}

	vis->opdraw->drawhline_nc	= GGI_lin8_drawhline_nc;
	vis->opdraw->drawhline	= GGI_lin8_drawhline;
	vis->opdraw->puthline	= GGI_lin8_puthline;
	vis->opdraw->gethline	= GGI_lin8_gethline;

	vis->opdraw->drawvline_nc	= GGI_lin8_drawvline_nc;
	vis->opdraw->drawvline	= GGI_lin8_drawvline;
	vis->opdraw->putvline	= GGI_lin8_putvline;
	vis->opdraw->getvline	= GGI_lin8_getvline;

	vis->opdraw->drawline	= GGI_lin8_drawline;

	vis->opdraw->drawbox	 	= GGI_lin8_drawbox;
	vis->opdraw->putbox	 	= GGI_lin8_putbox;
	vis->opdraw->copybox 	= GGI_lin8_copybox;
	vis->opdraw->crossblit	= GGI_lin8_crossblit;

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_linear_8(int func, void **funcptr);

int GGIdl_linear_8(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
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

#include <ggi/internal/ggidlinit.h>
