/* $Id: aa.h,v 1.2 2003/12/13 21:12:03 mooz Exp $
******************************************************************************

   Headers for AA target.

   Copyright (C) 1997 Andreas Beck		[becka@ggi-project.org]
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

#ifndef _GGI_DISPLAY_AA_H
#define _GGI_DISPLAY_AA_H

#include <aalib.h>
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/mansync.h>


ggifunc_flush			GGI_aa_flush;
ggifunc_getmode		GGI_aa_getmode;
ggifunc_setmode		GGI_aa_setmode;
ggifunc_checkmode	GGI_aa_checkmode;
ggifunc_getapi 		GGI_aa_getapi;
ggifunc_setflags	GGI_aa_setflags;

ggifunc_setPalette	GGI_aa_setPalette;
size_t GGI_aa_getPrivSize(ggi_visual_t);

giifunc_eventpoll	GII_aa_poll;
/*
giifunc_sendevent	GII_aa_sendevent;
*/

#define AA_PRIV(vis) ((ggi_aa_priv *)LIBGGI_PRIVATE(vis))

/* Multiply screen coordinates by this to get image coordinates */
#define AA_SCRMULT_X 2
#define AA_SCRMULT_Y 2

#define MANSYNC_init(vis)   AA_PRIV(vis)->opmansync->init(vis)
#define MANSYNC_deinit(vis) AA_PRIV(vis)->opmansync->deinit(vis)
#define MANSYNC_start(vis)  AA_PRIV(vis)->opmansync->start(vis)
#define MANSYNC_stop(vis)   AA_PRIV(vis)->opmansync->stop(vis)
#define MANSYNC_ignore(vis) AA_PRIV(vis)->opmansync->ignore(vis)
#define MANSYNC_cont(vis)   AA_PRIV(vis)->opmansync->cont(vis)

typedef struct { 
	aa_context *context;
	
	_ggi_opmansync *opmansync;
	void *aalock;

	int fastrender;

	int lx, ly, lb;
	unsigned int lastkey, lastkeyticks;
	int haverelease;
} ggi_aa_priv;

void _GGI_aa_freedbs(ggi_visual *vis);

#endif /* _GGI_DISPLAY_AA_H */
