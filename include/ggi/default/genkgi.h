/* $Id: genkgi.h,v 1.1 2001/05/12 23:03:19 cegger Exp $
******************************************************************************

   LibGGI - kgicon specific overrides for fbcon
   API header

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]

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

#ifndef _GENKGI_H
#define _GENKGI_H

/* FIXME: We should really use helperlibs for this */
#undef GENKGI_USE_PPBUF

#include <unistd.h>
#include <sys/mman.h>

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>
#include <kgi/kgi.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

struct genkgi_priv {
	ggi_gc *mapped_gc;
	unsigned int gc_size;
	ggifunc_drawline   *drawline;
	ggifunc_drawbox    *drawbox;
	ggifunc_copybox    *copybox;
	ggifunc_fillscreen *fillscreen;
	int fd_gc;
	int close_gc;
	int fd_kgicommand;
	uint8 *mapped_kgicommand;
	uint8 *kgicommand_ptr;
	unsigned int kgicommand_buffersize;
	unsigned int pagesize;
};

#define GENKGI_PRIV(vis) ((struct genkgi_priv*)FBDEV_PRIV(vis)->accelpriv)

ggifunc_drawline	GGI_genkgi_drawline;
ggifunc_drawbox		GGI_genkgi_drawbox;
ggifunc_copybox		GGI_genkgi_copybox;
ggifunc_fillscreen	GGI_genkgi_fillscreen;
ggifunc_flush		GGI_genkgi_flush;

#define GENKGI_PP_SETUP(val1, val2) \
val1 *temp; \
if \
((kgiu32)(GENKGI_PRIV(vis)->kgicommand_ptr + 4 + 4 + sizeof(val1)) > \
((kgiu32)(GENKGI_PRIV(vis)->kgicommand_ptr + GENKGI_PRIV(vis)->pagesize) & 0xfffff000)) \
GGI_genkgi_flush(vis, 0, 0, 0, 0, 0); \
temp = (val1 *)(GENKGI_PRIV(vis)->kgicommand_ptr + 4); \
*((kgiu32 *)GENKGI_PRIV(vis)->kgicommand_ptr) = (kgiu32)val2

#define GENKGI_PP_FINISH(val1) \
GENKGI_PRIV(vis)->kgicommand_ptr += sizeof(val1) + 4; \
*((kgiu32 *)GENKGI_PRIV(vis)->kgicommand_ptr) = (kgiu32)0; \
if (!(LIBGGI_FLAGS(vis) * GGIFLAG_ASYNC)) \
GGI_genkgi_flush(vis, 0, 0, 0, 0, 0); \
return GGI_OK


#endif /* _GENKGI_H */
