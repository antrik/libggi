/* $Id: mode.m,v 1.8 2004/11/27 16:42:25 soyt Exp $
******************************************************************************

   Display quartz : mode management

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ggi/display/quartz.h>
#include <ggi/internal/ggi_debug.h>

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

int GGI_quartz_getapi(ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	*arguments = '\0';

	switch(num) {
	case 0: strcpy(apiname, "display-quartz");
		return 0;
	case 1: strcpy(apiname, "generic-stubs");
		return 0;
	case 2: strcpy(apiname, "generic-color");
		return 0;
	case 3:
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%d",
				GT_SIZE(LIBGGI_GT(vis)));
		} else {
			sprintf(apiname, "generic-linear-%d%s",
				GT_SIZE(LIBGGI_GT(vis)),
				(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT)
				? "-r" : "");
		}	/* if */
		return 0;
	}	/* switch */

	return GGI_ENOMATCH;
}	/* GGI_quartz_getapi */



static int _GGInumberForKey( CFDictionaryRef desc, CFStringRef key )
{
	CFNumberRef value;
	int num = 0;

	if ( (value = CFDictionaryGetValue(desc, key)) == NULL ) {
		return 0;
	}	/* if */
	CFNumberGetValue(value, kCFNumberIntType, &num);
	return num;
}	/* _GGInumberForKey */


static void _GGIfreedbs(ggi_visual *vis)
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}	/* for */
}	/* _GGIfreedbs */


static void _GGIallocdbs(ggi_visual *vis)
{
	int i;
	uint8 *fb_ptr;
	ggi_mode *tm;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	tm = LIBGGI_MODE(vis);

	priv->fb = (uint8 *)CGDisplayBaseAddress(priv->display_id);
	priv->stride = CGDisplayBytesPerRow(priv->display_id);

#if 0
	priv->window = NSMakeRect(0, 0, tm->visible.x, tm->visible.y);
#endif

	fb_ptr = priv->fb;
	for (i = 0; i < tm->frames; i++) {

		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->write = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = priv->stride;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = 
			LIBGGI_PIXFMT(vis);

		fb_ptr += LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride*tm->virt.y;
	}	/* for */

	return;
}	/* _GGIallocdbs */



static int _ggi_load_mode_libs(ggi_visual *vis)
{
	int err, i;
	char	name[GGI_MAX_APILEN];
	char	args[GGI_MAX_APILEN];

	DPRINT("display-quartz: _ggi_load_mode_libs: called\n");

	_ggiZapMode(vis, 0);
	for(i=1; 0 == GGI_quartz_getapi(vis, i, name, args); i++) {
		err = _ggiOpenDL(vis, name, args, NULL);
		if (err) {
			fprintf(stderr,"display-quartz: Can't open the "
				"%s (%s) library.\n", name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading %s (%s)\n",
				name, args);
		}	/* if */
	}	/* for */
	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}	/* _ggi_load_mode_libs */


int GGI_quartz_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	int err = 0;
	ggi_quartz_priv *priv;
	CFDictionaryRef bestmode;
	boolean_t exactMatch;

	priv = QUARTZ_PRIV(vis);

	/* Take care of automatic graphtype selection */
	_GGIhandle_ggiauto(mode,
			_GGInumberForKey(priv->cur_mode, kCGDisplayWidth),
			_GGInumberForKey(priv->cur_mode, kCGDisplayHeight));

	if (mode->graphtype == GT_AUTO) {
		switch (_GGInumberForKey(priv->cur_mode, kCGDisplayBitsPerPixel)) {
		case 1: mode->graphtype = GT_1BIT; break;
		case 2: mode->graphtype = GT_2BIT; break;
		case 4: mode->graphtype = GT_4BIT; break;
		case 8: mode->graphtype = GT_8BIT; break;
		case 15: mode->graphtype = GT_15BIT; break;
		case 16: mode->graphtype = GT_16BIT; break;
		case 24: mode->graphtype = GT_24BIT; break;
		case 32: mode->graphtype = GT_32BIT; break;
		}	/* switch */
	}	/* if */

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	if (mode->frames == GGI_AUTO) mode->frames = 1;

	if (mode->frames != 1) {
		err = -1;
		mode->frames = 1;
	}	/* if */

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO))
	{
		err = -1;
	}	/* if */
	mode->dpp.x = mode->dpp.y = 1;

#if 1
	fprintf(stderr, "Check for mode: ");
	ggiFPrintMode(stderr, mode);
	fprintf(stderr, "\n");
#endif

	bestmode = CGDisplayBestModeForParameters(priv->display_id, GT_DEPTH(mode->graphtype),
						mode->visible.x, mode->visible.y,
						&exactMatch);
	if (bestmode == NULL) {
		/* invalid display */
		return GGI_ENODEVICE;
	}	/* if */

	priv->suggest_mode = bestmode;

	/* do some checks */
	if (!exactMatch) {
		mode->visible.x = _GGInumberForKey(bestmode, kCGDisplayWidth);
		mode->visible.y = _GGInumberForKey(bestmode, kCGDisplayHeight);

		switch (_GGInumberForKey(bestmode, kCGDisplayBitsPerPixel)) {
		case 1: mode->graphtype = GT_1BIT; break;
		case 2: mode->graphtype = GT_2BIT; break;
		case 4: mode->graphtype = GT_4BIT; break;
		case 8: mode->graphtype = GT_8BIT; break;
		case 15: mode->graphtype = GT_15BIT; break;
		case 16: mode->graphtype = GT_16BIT; break;
		case 24: mode->graphtype = GT_24BIT; break;
		case 32: mode->graphtype = GT_32BIT; break;
		}	/* switch */
		if ( CGDisplayCanSetPalette(priv->display_id) ) {
			fprintf(stderr, "checkmode: palette mode\n");
			GT_SETSCHEME(mode->graphtype, GT_PALETTE);
		} else {
			fprintf(stderr, "checkmode: truecolor mode\n");
			GT_SETSCHEME(mode->graphtype, GT_TRUECOLOR);
		}	/* if */
	}	/* if */

	/* Quartz has no virtual resolutions. */
	if (mode->virt.x != mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}	/* if */

	if (mode->virt.y != mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}	/* if */

#if 1
	fprintf(stderr, "Got mode: ");
	ggiFPrintMode(stderr, mode);
	fprintf(stderr, "\n");
#endif

	if (err) return err;
	err = _ggi_physz_figure_size(mode, priv->physzflags,
				&(priv->physz),
				0, 0, mode->visible.x, mode->visible.y);

	return err;
}	/* GGI_quartz_checkmode */


int GGI_quartz_setmode(ggi_visual *vis, ggi_mode *mode)
{
	int err;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	DPRINT("display-quartz: GGIsetmode: called\n");

	APP_ASSERT(vis != NULL, "GGI_memory_setmode: Visual == NULL");

	if ((err = ggiCheckMode(vis, mode)) != 0) return err;

	/* some elements of the mode setup rely on this. */
	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	err = CGDisplaySwitchToMode(priv->display_id, priv->suggest_mode);
	if ( err != CGDisplayNoErr ) {
		/* FIXME: I don't know, whether err is positive or negative */
		return (err >= 0) ? -err : err;
	}	/* if */

	_GGIfreedbs(vis);

	priv->cur_mode = CGDisplayCurrentMode(priv->display_id);

	switch (_GGInumberForKey(priv->cur_mode, kCGDisplayBitsPerPixel)) {
	case 1: mode->graphtype = GT_1BIT; break;
	case 2: mode->graphtype = GT_2BIT; break;
	case 4: mode->graphtype = GT_4BIT; break;
	case 8: mode->graphtype = GT_8BIT; break;
	case 15: mode->graphtype = GT_15BIT; break;
	case 16: mode->graphtype = GT_16BIT; break;
	case 24: mode->graphtype = GT_24BIT; break;
	case 32: mode->graphtype = GT_32BIT; break;
	}	/* switch */
	if ( CGDisplayCanSetPalette(priv->display_id) ) {
		GT_SETSCHEME(mode->graphtype, GT_PALETTE);
	} else {
		GT_SETSCHEME(mode->graphtype, GT_TRUECOLOR);
	}	/* if */

	priv->ncols = 1 << GT_DEPTH(mode->graphtype);
	if (GT_SCHEME(mode->graphtype) == GT_PALETTE) {
		fprintf(stderr, "setmode: palette mode\n");
		vis->palette = _ggi_malloc(sizeof(ggi_color) * priv->ncols);
		vis->opcolor->setpalvec = GGI_quartz_setpalvec;
	}	/* if */

	/* initialize gamma structure */
	vis->gamma->start = 0;
	vis->gamma->len = priv->ncols;

	/* Pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);

	_GGIallocdbs(vis);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	err = _ggi_load_mode_libs(vis);
	DPRINT("display-quartz: GGIsetmode: domode=%d\n",err);
	if (err) return err;

	ggiIndicateChange(vis, GGI_CHG_APILIST);
	DPRINT("display-quartz: GGIsetmode: change indicated\n",err);

	return 0;
}	/* GGI_quartz_setmode */


int GGI_quartz_getmode(ggi_visual *vis, ggi_mode *mode)
{
	ggi_mode mymode;
	DPRINT("display-quartz: GGIgetmode(%p,%p)\n", vis, mode);

	memcpy(&mymode, LIBGGI_MODE(vis), sizeof(ggi_mode));
	memcpy(mode, &mymode, sizeof(ggi_mode));

	return 0;
}	/* GGI_quartz_getmode */


int GGI_quartz_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
	return 0;
}	/* GGI_quartz_setflags */


int GGI_quartz_flush(ggi_visual *vis,
		int x, int y, int w, int h, int tryflag)
{
#warning Implement flush()
	return 0;
}	/* GGI_quartz_flush */
