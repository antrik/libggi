/* $Id: mode.c,v 1.47 2009/05/09 13:53:07 cegger Exp $
******************************************************************************

   Display quartz : mode management

   Copyright (C) 2002-2004 Christoph Egger

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
#include "quartz.h"
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

#include <ggi/input/quartz.h>



int GGI_quartz_getapi(struct ggi_visual *vis,int num,
			char *apiname, char *arguments)
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
			snprintf(apiname, GGI_MAX_APILEN, "generic-text-%d",
				GT_SIZE(LIBGGI_GT(vis)));
		} else {
			snprintf(apiname, GGI_MAX_APILEN, "generic-linear-%d%s",
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

#define _ggi_screenwidth(desc)  _GGInumberForKey((desc), kCGDisplayWidth)
#define _ggi_screenheight(desc) _GGInumberForKey((desc), kCGDisplayHeight)
#define _ggi_screen_gtdepth(desc) _GGInumberForKey((desc), kCGDisplayBitsPerPixel)
#define _ggi_screen_gtsize(desc) ((_ggi_screen_gtdepth(desc)+7)/8)

#if 0
static int _ggi_load_slaveops(struct ggi_visual *vis)
{
	ggi_quartz_priv *priv;
	priv = QUARTZ_PRIV(vis);

	DPRINT_MODE("load slave ops");
	LIB_ASSERT(priv->memvis != NULL, "there is no backbuffer\n");

	vis->opcolor->mapcolor = priv->memvis->opcolor->mapcolor;
	vis->opcolor->unmappixel = priv->memvis->opcolor->unmappixel;
	vis->opcolor->packcolors = priv->memvis->opcolor->packcolors;
	vis->opcolor->unpackpixels = priv->memvis->opcolor->unpackpixels;
	vis->opcolor->setpalvec = priv->memvis->opcolor->setpalvec;
	vis->opcolor->getpalvec = priv->memvis->opcolor->getpalvec;
	vis->opcolor->getgamma = priv->memvis->opcolor->getgamma;
	vis->opcolor->setgamma = priv->memvis->opcolor->setgamma;
	vis->opcolor->getgammamap = priv->memvis->opcolor->getgammamap;
	vis->opcolor->setgammamap = priv->memvis->opcolor->setgammamap;
	vis->opgc->gcchanged = priv->memvis->opgc->gcchanged;

	vis->opdraw->setdisplayframe = priv->memvis->opdraw->setdisplayframe;
	vis->opdraw->setreadframe = priv->memvis->opdraw->setreadframe;
	vis->opdraw->setwriteframe = priv->memvis->opdraw->setwriteframe;
	vis->opdraw->drawpixel = priv->memvis->opdraw->drawpixel;
	vis->opdraw->drawpixel_nc = priv->memvis->opdraw->drawpixel_nc;
	vis->opdraw->putpixel = priv->memvis->opdraw->putpixel;
	vis->opdraw->putpixel_nc = priv->memvis->opdraw->putpixel_nc;
	vis->opdraw->getpixel = priv->memvis->opdraw->getpixel;
	vis->opdraw->getpixel_nc = priv->memvis->opdraw->getpixel_nc;
	vis->opdraw->drawhline = priv->memvis->opdraw->drawhline;
	vis->opdraw->drawhline_nc = priv->memvis->opdraw->drawhline_nc;
	vis->opdraw->puthline = priv->memvis->opdraw->puthline;
	vis->opdraw->gethline = priv->memvis->opdraw->gethline;
	vis->opdraw->drawvline = priv->memvis->opdraw->drawvline;
	vis->opdraw->drawvline_nc = priv->memvis->opdraw->drawvline_nc;
	vis->opdraw->putvline = priv->memvis->opdraw->putvline;
	vis->opdraw->getvline = priv->memvis->opdraw->getvline;
	vis->opdraw->drawline = priv->memvis->opdraw->drawline;
	vis->opdraw->drawbox = priv->memvis->opdraw->drawbox;
	vis->opdraw->putbox = priv->memvis->opdraw->putbox;
	vis->opdraw->getbox = priv->memvis->opdraw->getbox;
	vis->opdraw->copybox = priv->memvis->opdraw->copybox;
	vis->opdraw->fillscreen = priv->memvis->opdraw->fillscreen;

	return 0;
}
#endif


static int
GGI_quartz_updateWindowContext(struct ggi_visual *vis, ggi_mode *mode)
{
	size_t fb_size;
	uint8_t *fb;
	size_t stride;
	CGRect tmpBounds;
	Rect contentRect;
	int titleborder;
	ggi_quartz_priv *priv = QUARTZ_PRIV(vis);

	LIB_ASSERT(priv->theWindow != NULL, "a mode has not been set!\n");

	GetWindowBounds( priv->theWindow, kWindowContentRgn, &contentRect);

	if (priv->fullscreen) {
		titleborder = GGI_FULLSCREEN_TITLEBORDER;
	} else {
		titleborder = GGI_WINDOW_TITLEBORDER;
	}

	fb_size = mode->visible.x * GT_ByPP(mode->graphtype)
			* mode->visible.y * mode->frames;
	if (fb_size == 0) {
		/* hmm... no mode set.
		 * Nothing to do then.
		 */
		return GGI_OK;
	}

	if ((priv->fb == NULL) || (priv->fb_size != fb_size)) {

		if (priv->fb == NULL) {
			fb = calloc(1, fb_size);
		} else {
			fb = realloc(priv->fb, fb_size);
		}
		if (fb == NULL) {
			return GGI_ENOMEM;
		}
		priv->fb = fb;
		priv->fb_size = fb_size;

		stride = mode->visible.x * GT_ByPP(mode->graphtype);

		LIB_ASSERT(priv->image == NULL,
			"priv->image is leaking\n");
		LIB_ASSERT(priv->dataProviderRef == NULL,
			"priv->dataProvider is leaking\n");

		priv->dataProviderRef = CGDataProviderCreateWithData(NULL,
				priv->fb, priv->fb_size, NULL);
		priv->image = CGImageCreate(mode->visible.x,
				mode->visible.y,
				8 /* bits per component */,
				GT_SIZE(mode->graphtype),
				stride,
				CGColorSpaceCreateDeviceRGB(),
				kCGImageAlphaNoneSkipFirst,
				priv->dataProviderRef,
				NULL, 0, kCGRenderingIntentDefault);
	}

	memcpy(LIBGGI_MODE(vis), &mode, sizeof(ggi_mode));
	memcpy(&priv->winRect, &contentRect, sizeof(Rect));

	/* Create drawing context */
	tmpBounds = CGRectMake(0, titleborder,
		priv->winRect.right, priv->winRect.bottom);
	CreateCGContextForPort(GetWindowPort(priv->theWindow),
		&priv->context);
	
	/* Refresh content */
	_ggiFlush(vis);

	return GGI_OK;
}	/* GGI_quartz_updateWindowContext */


static void _GGIfreedbs(struct ggi_visual *vis)
{
	int i;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}	/* for */

#if 0
	if (priv->memvis != NULL) {
		ggiClose(priv->memvis->instance.stem);
		priv->memvis = NULL;
	}
#endif
	if (priv->fb != NULL) {
		free(priv->fb);
		priv->fb = NULL;
	}

	if (priv->image != NULL) {
		CGImageRelease(priv->image);
		CGDataProviderRelease(priv->dataProviderRef);
		priv->dataProviderRef = NULL;
		priv->image = NULL;
	}
}	/* _GGIfreedbs */


static int _GGIallocdbs(struct ggi_visual *vis, int reload_api)
{
	char target[GGI_MAX_APILEN], args[GGI_MAX_APILEN];
	int i;
	uint8_t *fb_ptr;
	ggi_quartz_priv *priv = QUARTZ_PRIV(vis);
	size_t stride;
	ggi_mode tm;

	memcpy(&tm, LIBGGI_MODE(vis), sizeof(ggi_mode));
	stride = tm.visible.x * GT_ByPP(tm.graphtype);

	fb_ptr = priv->fb;
	for (i = 0; i < tm.frames; i++) {

		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->write = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = stride;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = 
			LIBGGI_PIXFMT(vis);

		fb_ptr += LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride * tm.virt.y;
	}	/* for */

	if (reload_api) {
		for (i = 1; 0 == GGI_quartz_getapi(vis, i, target, args); i++) {
			int err = _ggiOpenDL(vis, libggi->config,
						target, args, NULL);
			if (err) {
				fprintf(stderr,
					"display-quartz: Can't open the %s (%s) library.\n",
					target, args);
				return err;
			} else {
				DPRINT_LIBS("GGI_quartz_setmode: success in loading "
					"%s (%s)\n", target, args);
			}
		}
		ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);
	}

	return GGI_OK;
}	/* _GGIallocdbs */



static int
GGI_quartz_checkmode_fullscreen(struct ggi_visual *vis, ggi_mode *mode)
{
	int err = 0;
	ggi_quartz_priv *priv;
	CFDictionaryRef bestmode;
	boolean_t exactMatch;

	priv = QUARTZ_PRIV(vis);

	/* Take care of automatic graphtype selection */
	_GGIhandle_ggiauto(mode,
			_ggi_screenwidth(priv->cur_mode),
			_ggi_screenheight(priv->cur_mode));

	if (mode->graphtype == GT_AUTO) {
		switch (_ggi_screen_gtdepth(priv->cur_mode)) {
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

	if (mode->frames == GGI_AUTO)
		mode->frames = 1;

	if (mode->frames != 1) {
		DPRINT_MODE("GGI_quartz_checkmode_fullscreen: "
			"multi-frame support not implemented\n");
		err = GGI_ENOMATCH;
		mode->frames = 1;
	}	/* if */

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO))
	{
		DPRINT_MODE("GGI_quartz_checkmode_fullscreen: "
			"dpp != 1 or != GGI_AUTO\n");
		err = GGI_ENOMATCH;
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
		mode->visible.x = _ggi_screenwidth(bestmode);
		mode->visible.y = _ggi_screenheight(bestmode);

		switch (_ggi_screen_gtdepth(bestmode)) {
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
		DPRINT_MODE("GGI_quartz_checkmode_fullscreen: "
			"no virtual resolutions support. "
			"virt.x must match visible.x\n");
		mode->virt.x = mode->visible.x;
		err = GGI_ENOMATCH;
	}	/* if */

	if (mode->virt.y != mode->visible.y) {
		DPRINT_MODE("GGI_quartz_checkmode_fullscreen: "
			"no virtual resolutions support. "
			"virt.y must match visible.y\n");
		mode->virt.y = mode->visible.y;
		err = GGI_ENOMATCH;
	}	/* if */

#if 1
	fprintf(stderr, "Got mode: ");
	ggiFPrintMode(stderr, mode);
	fprintf(stderr, "\n");
#endif

	return err;
}	/* GGI_quartz_checkmode_fullscreen */


static int
GGI_quartz_checkmode_windowed(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_quartz_priv *priv;
	int default_width, default_height;
	int err = 0;

	priv = QUARTZ_PRIV(vis);

	/* handle GGI_AUTO */
	default_width = _ggi_screenwidth(priv->cur_mode) * 9 / 10;
	default_height = _ggi_screenheight(priv->cur_mode) * 9 / 10;
	_GGIhandle_ggiauto(mode, default_width, default_height);

	if (mode->visible.x > _ggi_screenwidth(priv->cur_mode)) {
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"visible.x doesn't match screen width\n");
		err = GGI_ENOMATCH;
		mode->visible.x = default_width;
	}
	if (mode->visible.y > _ggi_screenheight(priv->cur_mode)) {
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"visible.y doesn't match screen height\n");
		err = GGI_ENOMATCH;
		mode->visible.y = default_height;
	}

	if (mode->graphtype == GT_AUTO) {
		switch (_ggi_screen_gtdepth(priv->cur_mode)) {
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


	if (mode->frames == GGI_AUTO)
		mode->frames = 1;

	if (mode->frames != 1) {
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"multi-frame support not implemented\n");
		err = GGI_ENOMATCH;
		mode->frames = 1;
	}	/* if */

	/* Quartz has no virtual resolutions. */
	if (mode->virt.x != mode->visible.x) {
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"no virtual resolutions support. "
			"virt.x must match visible.x\n");
		mode->virt.x = mode->visible.x;
		err = GGI_ENOMATCH;
	}	/* if */

	if (mode->virt.y != mode->visible.y) {
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"no virtual resolutions support. "
			"virt.y must match visible.y\n");
		mode->virt.y = mode->visible.y;
		err = GGI_ENOMATCH;
	}	/* if */

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO))
	{
		DPRINT_MODE("GGI_quartz_checkmode_windowed: "
			"dpp != 1 or != GGI_AUTO\n");
		err = GGI_ENOMATCH;
	}	/* if */
	mode->dpp.x = mode->dpp.y = 1;


	return err;
}

static int
compatible_mode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_quartz_priv *priv = QUARTZ_PRIV(vis);

	if (priv->fullscreen)
		return !memcmp(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	if (mode->frames != LIBGGI_MODE(vis)->frames)
		return 0;
	if (mode->graphtype != LIBGGI_GT(vis))
		return 0;
	if (memcmp(&mode->dpp, &LIBGGI_MODE(vis)->dpp, sizeof(ggi_coord)))
		return 0;

	return 1;
}

int GGI_quartz_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	int err;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	LIB_ASSERT(mode != NULL, "invalid mode\n");
	if (priv->fullscreen) {
		err = GGI_quartz_checkmode_fullscreen(vis, mode);
	} else {
		err = GGI_quartz_checkmode_windowed(vis, mode);
	}

	if (err) return err;
	err = _ggi_physz_figure_size(mode, priv->physzflags,
				&(priv->physz),
				0, 0, mode->visible.x, mode->visible.y);

	return err;
}



static int GGI_quartz_setmode_fullscreen(struct ggi_visual *vis, ggi_mode *mode)
{
	int err;
	ggi_quartz_priv *priv;

	priv = QUARTZ_PRIV(vis);

	/* some elements of the mode setup rely on this. */
	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	if (GT_SCHEME(mode->graphtype) == GT_PALETTE) {
		if (LIBGGI_PAL(vis)->clut.data) {
			free(LIBGGI_PAL(vis)->clut.data);
			LIBGGI_PAL(vis)->clut.data = NULL;
			LIBGGI_PAL(vis)->clut.size = 0;
		}
	}

	err = CGDisplaySwitchToMode(priv->display_id, priv->suggest_mode);
	if ( err != CGDisplayNoErr ) {
		/* FIXME: I don't know, whether err is positive or negative */
		return (err >= 0) ? -err : err;
	}	/* if */

	priv->cur_mode = CGDisplayCurrentMode(priv->display_id);

	switch (_ggi_screen_gtdepth(priv->cur_mode)) {
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
		LIBGGI_PAL(vis)->clut.size = priv->ncols;
		LIBGGI_PAL(vis)->clut.data = calloc(1, sizeof(ggi_color) * priv->ncols);
		vis->opcolor->setpalvec = GGI_quartz_setpalvec;
	}	/* if */

	/* initialize gamma structure */
	vis->gamma->start = 0;
	vis->gamma->len = priv->ncols;

	return err;
}	/* GGI_quartz_setmode_fullscreen */


static void _create_menu(struct ggi_visual *vis)
{
	MenuRef windMenu;

	DPRINT_MODE("Create Menu\n");

	/* Clear Menu Bar */
	ClearMenuBar();

	/* Create Window Menu */
	CreateStandardWindowMenu(0, &windMenu);
	InsertMenu(windMenu, 0);

	DrawMenuBar();

	return;
}


static int GGI_quartz_setmode_windowed(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_quartz_priv *priv;
	CFStringRef titleKey;
	CFStringRef windowTitle;
	OSStatus result;
	const char *winname = "GGI-on-quartz";

	priv = QUARTZ_PRIV(vis);

	if (priv->theWindow != NULL) {
		/* Happens when we re-set the mode */
		DPRINT_MODE("Re-Set the window\n");
		ChangeWindowAttributes(priv->theWindow,
			~priv->windowAttrs, priv->windowAttrs);
		SizeWindow(priv->theWindow, mode->visible.x, mode->visible.y, 1);
		GGI_quartz_updateWindowContext(vis, mode);
		return GGI_OK;
	} else {
		SetRect(&priv->winRect, 0, 0,
			mode->visible.x, mode->visible.y);

#if 1
		_create_menu(vis);
#endif

		DPRINT_MODE("Create the window\n");
		CreateNewWindow(kDocumentWindowClass, priv->windowAttrs,
				&priv->winRect, &priv->theWindow);
		if (priv->theWindow == NULL) {
			/* Could not create Window */
			DPRINT("Could not create Window\n");
			return GGI_ENODEVICE;
		}

		SetPortWindowPort(priv->theWindow);
		CreateWindowGroup(0, &priv->winGroup);
		SetWindowGroup(priv->theWindow, priv->winGroup);

		if (priv->windowtitle)
			winname = priv->windowtitle;

		titleKey = CFStringCreateWithCString(NULL, winname,
					kCFStringEncodingASCII);
		windowTitle = CFCopyLocalizedString(titleKey, NULL);
		result = SetWindowTitleWithCFString(priv->theWindow, windowTitle);
		CFRelease(titleKey);
		CFRelease(windowTitle);

		GGI_quartz_updateWindowContext(vis, mode);

		if (priv->inp == NULL) {
			/* Install event handler */
			DPRINT_MODE("Do not install event handler\n");
		} else {
			struct gii_quartz_cmddata_setparam data;

			data.theWindow = priv->theWindow;

			ggControl(priv->inp->channel, GII_CMDCODE_QZSETPARAM,
				&data);
		}

		DPRINT_MODE("Show the window\n");
		SetThemeWindowBackground(priv->theWindow,
				kThemeBrushModelessDialogBackgroundActive,
				TRUE);
#if 0
		RepositionWindow(priv->theWindow, NULL,
				kWindowCascadeOnParentWindow);
#endif
		MoveWindowStructure(priv->theWindow,
				0, GGI_WINDOW_TITLEBORDER * 2); 
		ShowWindow(priv->theWindow);
	}

	return 0;
}	/* GGI_quartz_setmode_windowed */


int GGI_quartz_setmode(struct ggi_visual *vis, ggi_mode *mode)
{
	ggi_quartz_priv *priv;
	int err;
	int compatible;
	int reload_api = 0;

	priv = QUARTZ_PRIV(vis);

	DPRINT_MODE("GGI_quartz_setmode: called\n");
	APP_ASSERT(vis != NULL, "GGI_quartz_setmode: Visual == NULL");

	err = GGI_quartz_checkmode(vis, mode);
	if (err != 0)
		return err;

	if (priv->opmansync)
		MANSYNC_ignore(vis);

	_GGIfreedbs(vis);

	compatible = compatible_mode(vis, mode);
	if (!compatible) {
		_ggiZapMode(vis, 0);
		reload_api = 1;
	}

	if (priv->fullscreen) {
		err = GGI_quartz_setmode_fullscreen(vis, mode);
	} else {
		err = GGI_quartz_setmode_windowed(vis, mode);
	}

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	/* Pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	_GGIallocdbs(vis, reload_api);

	if (priv->opmansync)
		MANSYNC_cont(vis);

	DPRINT_MODE("GGI_quartz_setmode returns = %i\n", err);

	return err;
}


int GGI_quartz_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	DPRINT_MISC("GGI_quartz_getmode(%p,%p)\n", vis, mode);

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}	/* GGI_quartz_getmode */


int GGI_quartz_setflags(struct ggi_visual *vis, uint32_t flags)
{
	ggi_quartz_priv *priv;

	DPRINT_MISC("GGI_quartz_setflags(%p,%p)\n", (void *)vis, flags);

	priv = QUARTZ_PRIV(vis);
	if ((LIBGGI_FLAGS(vis) & GGIFLAG_ASYNC) && !(flags & GGIFLAG_ASYNC))
		_ggiFlush(vis);

	LIBGGI_FLAGS(vis) = flags;
	/* Unknown flags don't take. */
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC;

	if (priv->opmansync) {
		MANSYNC_SETFLAGS(vis, flags);
	}

	return GGI_OK;
}	/* GGI_quartz_setflags */


int GGI_quartz_flush(struct ggi_visual *vis,
		int x, int y, int w, int h, int tryflag)
{
	ggi_quartz_priv *priv;
	CGImageRef subimage;
	CGRect bounds;

	priv = QUARTZ_PRIV(vis);

	DPRINT("GGI_quartz_flush (%p, %i, %i, %i, %i, %i) called\n",
		(void *)vis, x, y, w, h, tryflag);

	/* do not flush on a invalid context
	 * This happens at startup when we are in
	 * sync mode and no mode has been set.
	 */
	if (priv->context == NULL) return 0;

	/* When tryflag == 0, we know we are called
	 * from mansync. So there's no need to disable
	 * and reenable mansync. This results in a
	 * speed gain on sync mode.
	 */
	if (tryflag != 0) {
		if (priv->opmansync) MANSYNC_ignore(vis);
	}

	/* XXX CGContextDrawImage() performs scaling, which I
	 * don't understand how to control.
	 * Until someone comes up with an idea/solution how to update
	 * the specified area pixel by pixel _without_ image scaling,
	 * we always update the whole window/screen. That works at least.
	 * -- Christoph
	 */
	bounds = CGRectMake(0, 0, LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));
	CGContextDrawImage(priv->context, bounds, priv->image);
	CGContextFlush(priv->context);

#if 0
	/* That at least does not perform image scaling, but the
	 * y-coord is wrong, what I do not understand.
	 */
	bounds = CGRectMake(x,y, w,h);
	subimage = CGImageCreateWithImageInRect(priv->image, bounds);
	bounds = CGRectMake(x,y, w,h);
	CGContextDrawImage(priv->context, bounds, subimage);
	CGContextFlush(priv->context);
#endif

	if (tryflag != 0) {
		if (priv->opmansync) MANSYNC_cont(vis);
	}

	return 0;
}	/* GGI_quartz_flush */
