/* $Id: visual.c,v 1.4 2004/11/06 22:48:20 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck   [becka@ggi-project.org]
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <kgi/kgi_commands.h>

#include "ioctllib.h"

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif


struct ggi_visual_opdraw fallback_opdraw;

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
#if 0
	ggi_gc *mygc;
	
	/* map the HW GC ! */
	mygc=(ggi_gc *)mmap(NULL,/*PAGE_SIZE*/4096,PROT_READ|PROT_WRITE,
			MAP_SHARED,LIBGGI_FD(visual),MMAP_TYPE_GC);

 	if (mygc == MAP_FAILED) mygc = NULL;
	else LIBGGI_GC(visual) = mygc;
	GGIDPRINT("Signature: %x\n",*(int *)LIBGGI_GC(visual));
#endif
	/* This is only an "override library". It will try to override
	 * functions which it thinks the Accelerator can do better.
	 */

	/* Color mapping 
	 */

	/**** 2D ops ****/

	/* Save away the old operations. We might need to use them. */
	fallback_opdraw=*visual->opdraw;

	/* Positioning is not supported...
	 */

	/* Sprites aren't supported...
	 */

	/* Generic drawing
	 */

	visual->opdraw->drawline  =GGI_ioctl_drawline;
	visual->opdraw->drawbox   =GGI_ioctl_drawbox;
	visual->opdraw->fillscreen=GGI_ioctl_fillscreen;
	visual->opdraw->copybox=GGIcopybox;

	*dlret = GGI_DL_OPDRAW;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	/* FIXME unmap GC. */
#if 0
	munmap((void*)LIBGGI_GC(visual), 4096);
	LIBGGI_GC(visual) = NULL;
#endif
                
	return 0;
}
		

EXPORTFUNC
int GGIdl_ioctl(int func, void **funcptr);

int GGIdl_ioctl(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
