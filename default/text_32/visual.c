/* $Id: visual.c,v 1.9 2008/01/20 19:26:38 pekberg Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck	[becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan	[jmcc@ggi-project.org]
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

#include "t32lib.h"


static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	/* Color handling
	 */
	vis->opcolor->mapcolor		= GGI_t32_mapcolor;
	vis->opcolor->unmappixel	= GGI_t32_unmappixel;

	/* Frame handling
	 */

	vis->opdraw->setreadframe	= _ggi_default_setreadframe;
	vis->opdraw->setwriteframe	= _ggi_default_setwriteframe;
	
	/* Generic drawing is minimal
	 */
	if (vis->needidleaccel) {
		vis->opdraw->drawpixel_nc	= GGI_t32_drawpixel_nca;
		vis->opdraw->drawpixel		= GGI_t32_drawpixela;
		vis->opdraw->putpixel_nc	= GGI_t32_putpixel_nca;
		vis->opdraw->putpixel		= GGI_t32_putpixela;
		vis->opdraw->getpixel_nc	= GGI_t32_getpixel_nca;
	} else {
		vis->opdraw->drawpixel_nc	= GGI_t32_drawpixel_nc;
		vis->opdraw->drawpixel		= GGI_t32_drawpixel;
		vis->opdraw->putpixel_nc	= GGI_t32_putpixel_nc;
		vis->opdraw->putpixel		= GGI_t32_putpixel;
		vis->opdraw->getpixel_nc	= GGI_t32_getpixel_nc;
	}

	vis->opdraw->putc		= GGI_t32_putc;
	vis->opdraw->getcharsize	= GGI_t32_getcharsize;
	
	*dlret = GGI_DL_OPDRAW | GGI_DL_OPCOLOR;
	return 0;
}


EXPORTFUNC
int GGIdl_text_32(int func, void **funcptr);

int GGIdl_text_32(int func, void **funcptr)
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
