/* $Id: visual.c,v 1.2 2002/09/08 21:37:47 soyt Exp $
******************************************************************************

   SVGAlib target vgagl helper: initialization

   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include "vgaglvis.h"

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	if (args == NULL || strncmp(args, "sVgALIb", 7) != 0) {
		return GGI_EARGINVAL;
	}

	gl_setcontextvga(vga_getcurrentmode());

	/* Generic drawing
	 */
	vis->opdraw->drawpixel_nc	= GGI_vgagl_drawpixel_nc;
	vis->opdraw->drawpixel		= GGI_vgagl_drawpixel;
	vis->opdraw->putpixel_nc	= GGI_vgagl_putpixel_nc;
	vis->opdraw->putpixel		= GGI_vgagl_putpixel;
	vis->opdraw->getpixel		= GGI_vgagl_getpixel;

	vis->opdraw->drawhline_nc	= GGI_vgagl_drawhline_nc;
	vis->opdraw->drawhline		= GGI_vgagl_drawhline;
	vis->opdraw->gethline		= GGI_vgagl_gethline;
	vis->opdraw->puthline		= GGI_vgagl_puthline;

	vis->opdraw->drawvline_nc	= GGI_vgagl_drawvline_nc;
	vis->opdraw->drawvline		= GGI_vgagl_drawvline;
	vis->opdraw->getvline		= GGI_vgagl_getvline;
	vis->opdraw->putvline		= GGI_vgagl_putvline;
	
	vis->opdraw->drawbox		= GGI_vgagl_drawbox;
	vis->opdraw->putbox		= GGI_vgagl_putbox;
	vis->opdraw->getbox		= GGI_vgagl_getbox;
	vis->opdraw->fillscreen		= GGI_vgagl_fillscreen;

	/* 
	vis->opdraw->putc		= GGI_vgagl_putc;
	vis->opdraw->getcharsize	= GGI_vgagl_getcharsize;
	vis->opdraw->drawline	= GGI_vgagl_drawline;
	vis->opdraw->crossblit	= GGI_vgagl_crossblit;
	*/

	*dlret = GGI_DL_OPDRAW;
	return 0;
}


int GGIdl_vgagl(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
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
