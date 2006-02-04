/* $Id: visual.c,v 1.8 2006/02/04 22:11:46 soyt Exp $
******************************************************************************

   Generic drawing library

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "stublib.h"

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
	/* Generic drawing
	 */
	vis->opdraw->putc=GGI_stubs_putc;
	vis->opdraw->puts=GGI_stubs_puts;
	vis->opdraw->getcharsize=GGI_stubs_getcharsize;

	vis->opdraw->putpixel=GGI_stubs_putpixel;
	vis->opdraw->drawpixel=GGI_stubs_drawpixel;
	vis->opdraw->drawpixel_nc=GGI_stubs_drawpixel_nc;
	vis->opdraw->drawhline=GGI_stubs_drawhline;
	vis->opdraw->drawhline_nc=GGI_stubs_drawhline_nc;
	vis->opdraw->drawvline=GGI_stubs_drawvline;
	vis->opdraw->drawvline_nc=GGI_stubs_drawvline_nc;
	vis->opdraw->drawbox=GGI_stubs_drawbox;
	vis->opdraw->drawline=GGI_stubs_drawline;
	
	if (! (GT_SUBSCHEME(LIBGGI_GT(vis)) & GT_SUB_PACKED_GETPUT))

	switch (GT_ByPP(LIBGGI_GT(vis))) {

		case 1: vis->opdraw->puthline=_GGI_stubs_L1_puthline;
			vis->opdraw->putvline=_GGI_stubs_L1_putvline;
			vis->opdraw->gethline=_GGI_stubs_L1_gethline;
			vis->opdraw->getvline=_GGI_stubs_L1_getvline;
			break;

		case 2: vis->opdraw->puthline=_GGI_stubs_L2_puthline;
			vis->opdraw->putvline=_GGI_stubs_L2_putvline;
			vis->opdraw->gethline=_GGI_stubs_L2_gethline;
			vis->opdraw->getvline=_GGI_stubs_L2_getvline;
			break;

		case 3: vis->opdraw->puthline=_GGI_stubs_L3_puthline;
			vis->opdraw->putvline=_GGI_stubs_L3_putvline;
			vis->opdraw->gethline=_GGI_stubs_L3_gethline;
			vis->opdraw->getvline=_GGI_stubs_L3_getvline;
			break;

		case 4: vis->opdraw->puthline=_GGI_stubs_L4_puthline;
			vis->opdraw->putvline=_GGI_stubs_L4_putvline;
			vis->opdraw->gethline=_GGI_stubs_L4_gethline;
			vis->opdraw->getvline=_GGI_stubs_L4_getvline;
			break;
	}

	vis->opdraw->putbox=GGI_stubs_putbox;
	vis->opdraw->getbox=GGI_stubs_getbox;

	vis->opdraw->copybox=GGI_stubs_copybox;
	vis->opdraw->crossblit=GGI_stubs_crossblit;
	vis->opdraw->fillscreen=GGI_stubs_fillscreen;

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


EXPORTFUNC
int GGIdl_stubs(int func, void **funcptr);

int GGIdl_stubs(int func, void **funcptr)
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
