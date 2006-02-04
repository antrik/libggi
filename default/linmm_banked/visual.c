/* $Id: visual.c,v 1.9 2006/02/04 22:11:46 soyt Exp $
******************************************************************************

   Banked Access Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]
   Copyright (C) 1997 Brian S. Julin   [bri@calyx.com]

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
#include <asm/page.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <kgi/kgi_commands.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

#define BANKFB __localafb
#define BANKFB_LEN __localafb_len

/* 
 * We are guaranteed to have been checked already...
 */

#define BANK_PAGE_SIZE (32 * 1024) /* Later get from new ioctl */
#define MMAP_PAGE_SIZE ((PAGE_SIZE > BANK_PAGE_SIZE) ? PAGE_SIZE : BANK_PAGE_SIZE)

uint8_t *__localrfb=NULL;  /* Read only bank */
uint8_t *__localwfb=NULL;  /* Write-only bank */
uint8_t *__localafb=NULL;  /* Bank that anticipate ascending access */
uint8_t *__localdfb=NULL;  /* Bank that anticipates descending access */
static int __localafb_len;
static int __localdfb_len;
static int __localrfb_len;
static int __localwfb_len;

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	if (LIBGGI_MODE(vis)) {
		__localafb_len =
			__localdfb_len =
			__localrfb_len =
			__localwfb_len =
			(((LIBGGI_VIRTX(vis) * LIBGGI_VIRTY(vis))
			  & ~(MMAP_PAGE_SIZE - 1)) + MMAP_PAGE_SIZE);

		DPRINT("afb is this big: %d\n",__localafb_len);
		DPRINT("dfb is this big: %d\n",__localdfb_len);
		DPRINT("rfb is this big: %d\n",__localrfb_len);
		DPRINT("wfb is this big: %d\n",__localwfb_len);
	} else {
		ggiPanic("linmm_banked: NULL mode provided...\n");
		return GGI_ENOFUNC;
	}


	__localafb=(uint8_t *)mmap(NULL,
				 __localafb_len,
				 PROT_READ | PROT_WRITE,
				 MAP_SHARED,
				 LIBGGI_FD(vis),
				 MMAP_TYPE_MMIO |
				 MMAP_PER_REGION_TYPE| 
				 MMAP_PRIVATE | 4);
	if ((long)(__localafb) == MAP_FAILED) {
		__localafb = NULL;
	}

	__localdfb=(uint8_t *)mmap(NULL,
				 __localdfb_len,
				 PROT_READ | PROT_WRITE,
				 MAP_SHARED,
				 LIBGGI_FD(vis),
				 MMAP_TYPE_MMIO |
				 MMAP_PER_REGION_TYPE| 
				 MMAP_PRIVATE | 8);
	if ((long)(__localdfb) == MAP_FAILED) {
		__localdfb = (uint8_t *)NULL;
	}

	__localrfb=(uint8_t *)mmap(NULL,
				 __localrfb_len,
				 PROT_READ,
				 MAP_SHARED,
				 LIBGGI_FD(vis),
				 MMAP_TYPE_MMIO |
				 MMAP_PER_REGION_TYPE| 
				 MMAP_PRIVATE | 16);
	if ((long)(__localrfb) == MAP_FAILED) {
		__localrfb = (uint8_t *)NULL;
	}

	__localwfb=(uint8_t *)mmap(NULL,
				 __localwfb_len,
				 PROT_WRITE,
				 MAP_SHARED,
				 LIBGGI_FD(vis),
				 MMAP_TYPE_MMIO |
				 MMAP_PER_REGION_TYPE| 
				 MMAP_PRIVATE | 32);
	if ((long)(__localwfb) == MAP_FAILED) {
		__localwfb = (uint8_t *)NULL;
	}

	/* Now attempt make-do with whatever the graphics hardware provided. */
	if (!__localrfb) {
		/* Must provide Bank1 */
		if (!__localafb) {
			printf("No banks\n");
			return GGI_ENODEVICE;
		}
		/* Use Ascending Bank in leiu of real RO bank, if it exists
		   that is. */
		__localrfb = __localafb;
	}

	if (!__localwfb) {
		/* Must provide Bank1 */
		if (!__localafb) {
			printf("No banks\n");
			return GGI_ENODEVICE;
		}
		/* Use Ascending Bank in leiu of real WO bank, if it exists
		   that is. */
		__localwfb = __localafb;
	}

	if (!__localdfb) { 
		/* Must provide Ascending Bank */
		if (!__localafb) {
			printf("No banks\n");
			return GGI_ENODEVICE;
		}
		__localdfb = __localafb;  /* This won't be used often if
					     at all. */
	}

	DPRINT("afb=%p, dfb=%p, rfb=%p, wfb=%p\n",
		  __localafb, __localdfb, __localrfb, __localwfb);

	/* Linear framebuffer setup */
	if (LIBGGI_CURWRITE(vis) != NULL) {
		munmap(LIBGGI_CURWRITE(vis), LIBGGI_FB_SIZE(LIBGGI_MODE(vis)));
	}
	LIBGGI_CURREAD(vis) = LIBGGI_CURWRITE(vis) = BANKFB;
	/* LIBGGI_FB_LINEAR_SIZE(vis) = BANKFB_LEN; */

	/* Generic drawing
	 */

	/* vis->opdraw->fillscreen=GGIfillscreen; */
        /* vis->opdraw->putc=GGIputc; */
        /* vis->opdraw->getcharsize=GGIgetcharsize; */

	vis->opdraw->drawpixel_nc	= GGIdrawpixel_nc;
	vis->opdraw->drawpixel		= GGIdrawpixel;
	vis->opdraw->putpixel_nc	= GGIputpixel_nc;
	vis->opdraw->putpixel		= GGIputpixel;
	vis->opdraw->getpixel		= GGIgetpixel;
	
	vis->opdraw->drawhline_nc = GGIdrawhline_nc;
	vis->opdraw->drawhline	= GGIdrawhline;
	vis->opdraw->puthline	= GGIputhline;
	vis->opdraw->gethline	= GGIgethline;

	vis->opdraw->drawvline_nc = GGIdrawvline_nc;
	vis->opdraw->drawvline	= GGIdrawvline;
	vis->opdraw->putvline	= GGIputvline;
	vis->opdraw->getvline	= GGIgetvline;
	/*
	vis->opdraw->drawbox	= GGIdrawbox; 
	vis->opdraw->putbox	= GGIputbox;
	vis->opdraw->getbox	= GGIgetbox;
	vis->opdraw->drawline	= GGIdrawline;
	*/

	*dlret = GGI_DL_OPCOLOR | GGI_DL_OPDRAW;
	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	if (__localafb != NULL) {
		munmap((char *)__localafb,__localafb_len);
	}
	if (__localdfb != NULL && __localdfb != __localafb) {
		munmap((char *)__localdfb,__localdfb_len);
	}
	if (__localrfb != NULL && __localrfb != __localafb) {
		munmap((char *)__localrfb,__localrfb_len);
	}
	if (__localwfb != NULL && __localwfb != __localafb
	    && __localwfb != __localdfb) {
		munmap((char *)__localwfb,__localwfb_len); 
	}
	
	return 0;
}
		

EXPORTFUNC
int GGIdl_linmm_banked(int func, void **funcptr);

int GGIdl_linmm_banked(int func, void **funcptr)
{
	ggifunc_open **openptr;
	ggifunc_close **closeptr;

	switch (func) {
	case GGIFUNC_open:
		openptr = (ggifunc_open **)funcptr;
		*openptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		closeptr = (ggifunc_close **)funcptr;
		*closeptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}
