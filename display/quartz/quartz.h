/* $Id: quartz.h,v 1.2 2005/01/19 07:54:55 cegger Exp $
******************************************************************************

   Display-quartz: internal headers

   Copyright (C) 2004-2005 Christoph Egger

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

#ifndef _GGI_QUARTZ_H
#define _GGI_QUARTZ_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/quartz.h>


ggifunc_checkmode	GGI_quartz_checkmode;
ggifunc_getmode		GGI_quartz_getmode;
ggifunc_setmode		GGI_quartz_setmode;

ggifunc_getapi		GGI_quartz_getapi;
ggifunc_setflags	GGI_quartz_setflags;
ggifunc_setpalvec	GGI_quartz_setpalvec;
ggifunc_flush		GGI_quartz_flush;
ggifunc_gcchanged	GGI_quartz_gcchanged;

ggifunc_setgamma	GGI_quartz_setgamma;
ggifunc_getgamma	GGI_quartz_getgamma;
ggifunc_setgammamap	GGI_quartz_setgammamap;
ggifunc_getgammamap	GGI_quartz_getgammamap;

int _GGI_quartz_updateWindowContext(ggi_visual *vis);


#endif /* _GGI_QUARTZ_H */
