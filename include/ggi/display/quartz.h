/* $Id: quartz.h,v 1.1 2002/12/22 12:59:38 cegger Exp $
******************************************************************************

   Display-quartz: headers

   Copyright (C) 2002 Christoph Egger

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

#ifndef _GGI_DISPLAY_QUARTZ_H
#define _GGI_DISPLAY_QUARTZ_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/gii.h>

#include <ggi/input/cocoa.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Cocoa/Cocoa.h>

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


typedef struct {
	int		physzflags;
	ggi_coord	physz;

	NSApplication	*GGIApp;
	NSWindow	*window;	/* Cocoa window */

	size_t stride;
	uint8 *fb;

	/* 0 == main display (only support single display) */
	CGDirectDisplayID  display_id;

	/* current mode of the display */
	CFDictionaryRef    cur_mode;
	/* original mode of the display */
	CFDictionaryRef    save_mode;
	/* suggested mode from ggiCheckMode */
	CFDictionaryRef    suggest_mode;
	/* list of available fullscreen modes */
	CFArrayRef	   mode_list;

	/* palette of an 8-bit display */
	CGDirectPaletteRef palette;

	int		ncols;		/* Number of colors in the colormap */
	ggi_color	*gammamap;
	ggi_gammastate	gamma;

	gii_input	*inp;
} ggi_quartz_priv;


#define QUARTZ_PRIV(vis) ((ggi_quartz_priv *)LIBGGI_PRIVATE(vis))

#endif /* _GGI_DISPLAY_QUARTZ_H */
