/* $Id: visual.c,v 1.1 2002/10/05 18:39:06 fspacek Exp $
******************************************************************************

   ATI Mach64 acceleration sublib for kgi display target

   Copyright (C) 2002 Paul Redmond

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

#include <unistd.h>
#include <sys/mman.h>

#include "mach64_accel.h"

static int GGI_kgi_mach64_flush(ggi_visual *vis, int x, int y,
				int w, int h, int tryflag)
{
	MACH64_FLUSH(vis);

	return 0;
}


static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_accel_t *accel;

	accel = KGI_PRIV(vis)->map_accel(vis, 1, 0, MACH64_BUFFER_SIZE_ORDER,
			                 MACH64_BUFFER_NUM, 0);
	
	if(!accel)
		return -1;

	KGI_ACCEL_PRIV(vis) = accel;

	vis->opdisplay->flush = GGI_kgi_mach64_flush;

	/*vis->opdraw->setreadframe  = _ggi_default_setreadframe;*/
	/*vis->opdraw->setwriteframe = _ggi_default_setwriteframe;*/
	
	/* Generic drawing
	 */
	/*vis->opdraw->putc      = _ggi_default_putc;*/

	/*vis->opdraw->drawpixel_nc = _ggi_default_drawpixel_nc;*/
	/*vis->opdraw->drawpixel	  = _ggi_default_drawpixel;*/
	/*vis->opdraw->putpixel_nc  = _ggi_default_putpixel_nc;*/
	/*vis->opdraw->putpixel	  = _ggi_default_putpixel;*/
	/*vis->opdraw->getpixel	  = _ggi_default_getpixel;*/

	vis->opdraw->drawhline_nc = GGI_kgi_mach64_drawhline;
	vis->opdraw->drawhline	  = GGI_kgi_mach64_drawhline;
	/*vis->opdraw->puthline	  = _ggi_default_puthline;*/
	/*vis->opdraw->gethline	  = _ggi_default_gethline;*/

	vis->opdraw->drawvline_nc = GGI_kgi_mach64_drawvline;
	vis->opdraw->drawvline	  = GGI_kgi_mach64_drawvline;
	/*vis->opdraw->putvline	  = _ggi_default_putvline;*/
	/*vis->opdraw->getvline	  = _ggi_default_getvline;*/

	vis->opdraw->drawline     = GGI_kgi_mach64_drawline;

	vis->opdraw->drawbox      = GGI_kgi_mach64_drawbox;
	/*vis->opdraw->putbox       = _ggi_default_putbox;*/
	vis->opdraw->copybox      = GGI_kgi_mach64_copybox;
	/*vis->opdraw->crossblit    = _ggi_default_crossblit;*/

	vis->opgc->gcchanged	  = GGI_kgi_mach64_gcchanged;

	*dlret = GGI_DL_OPDRAW | GGI_DL_OPGC;
	return 0;	
}


int GGIdl_mach64(int func, void **funcptr)
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
