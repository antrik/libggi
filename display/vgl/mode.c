/* $Id: mode.c,v 1.5 2002/10/27 18:26:25 skids Exp $
******************************************************************************

   FreeBSD vgl(3) target: mode management

   Copyright (C) 2000 Alcove - Nicolas Souchu <nsouch@freebsd.org>

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

#include <sys/ioccom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/vgl.h>

void _GGI_vgl_freedbs(ggi_visual *);

int GGI_vgl_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return 0;
}
	
int GGI_vgl_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	struct vgl_priv *priv = VGL_PRIV(vis);

	switch(num) {
		case 0:
			strcpy(apiname, "display-vgl");
			strcpy(arguments, "");
			return 0;
		case 1:
			strcpy(apiname, "generic-stubs");
			strcpy(arguments, "");
			return 0;
		case 2:
			strcpy(apiname, "generic-color");
			strcpy(arguments, "");
			return 0;
		case 3:
			if (!priv->vgl_use_db)
				break;

			if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
				sprintf(apiname, "generic-text-%d",
					GT_SIZE(LIBGGI_GT(vis))); 
			} else {
				sprintf(apiname, "generic-linear-%d%s", 
					GT_SIZE(LIBGGI_GT(vis)),
					(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) 
					? "-r" : "");
			}
			strcpy(arguments, "");
			return 0;
		default:
			break;
	}
			
	return -1;
}

int GGI_vgl_setmode(ggi_visual *vis, ggi_mode *tm)
{ 
	struct vgl_priv *priv = VGL_PRIV(vis);
	ggi_graphtype gt = tm->graphtype;
	video_info_t modeinfo;
	unsigned long modenum = 0;
	char sugname[256];
	char args[256];
	int err = 0;
	int id, i;
	int pixelBytes;

	err = GGI_vgl_checkmode(vis, tm);
	if (err) return err;

	/* reset the modeinfo structure as expected by query_mode */
	memset(&modeinfo, 0, sizeof(modeinfo));
	
	switch(gt) {
		case GT_1BIT : modeinfo.vi_depth = 1; pixelBytes = 1; break;
		case GT_4BIT : modeinfo.vi_depth = 4; pixelBytes = 1; break;
		case GT_8BIT : modeinfo.vi_depth = 8; pixelBytes = 1; break;
		case GT_16BIT: modeinfo.vi_depth = 16; pixelBytes = 2; break;
		case GT_32BIT: modeinfo.vi_depth = 32; pixelBytes = 4; break;

		/* Unsupported mode depths */
		case GT_15BIT:
		case GT_24BIT:
		default:
			return -1;
	}

	modeinfo.vi_width = tm->visible.x;
	modeinfo.vi_height = tm->visible.y;

	/* XXX should be added to libvgl */
	if (ioctl(0, FBIO_FINDMODE, &modeinfo))
		return -1;

	GGIDPRINT("Setting VGLlib mode %d (0x%x)\n",
			modeinfo.vi_mode, modeinfo.vi_mode);

	/* Terminate any current mode before initialising another */
	if (priv->vgl_init_done) {
		priv->vgl_init_done = 0;
		VGLEnd();
	}

	/* XXX should be in VGL */
	if ((modeinfo.vi_mode >= M_B40x25) && (modeinfo.vi_mode <= M_VGA_M90x60))
		modenum = _IO('S', modeinfo.vi_mode);
	if ((modeinfo.vi_mode >= M_TEXT_80x25) && (modeinfo.vi_mode <= M_TEXT_132x60))
		modenum = _IO('S', modeinfo.vi_mode);
	if ((modeinfo.vi_mode >= M_VESA_CG640x400) &&
		(modeinfo.vi_mode <= M_VESA_FULL_1280))
		modenum = _IO('V', modeinfo.vi_mode - M_VESA_BASE);

	if ((err = VGLInit(modenum)) != 0) {
		GGIDPRINT("display-vgl: setting mode 0x%x failed with error %d\n",
			modeinfo.vi_mode, err);
		return GGI_EFATAL;
	}

	priv->vgl_init_done = 1;

	if (priv->vgl_use_db) {
		_GGI_vgl_freedbs(vis);

		/* Set up DirectBuffer(s) */
		for (i = 0; i<tm->frames; i++) {
			if (LIBGGI_FB_SIZE(tm) >
				(VGLDisplay->Xsize*VGLDisplay->Ysize*
					pixelBytes)) {
				fprintf(stderr, "display-vgl: framebuffer too large! (%d > %d*%d*%d)\n",
					LIBGGI_FB_SIZE(tm),
					VGLDisplay->Xsize, VGLDisplay->Ysize, 
					pixelBytes);
				return GGI_ENOMEM;
			}

			_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

			LIBGGI_APPBUFS(vis)[i]->frame = i;
			LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
			LIBGGI_APPBUFS(vis)[i]->read = VGLDisplay->Bitmap;
			LIBGGI_APPBUFS(vis)[i]->write = VGLDisplay->Bitmap;
			LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
				= ((GT_SIZE(tm->graphtype) * tm->virt.x)+7)/8;
		}
	}

	/* Save mode info returned by the VESA driver */
	bcopy(&modeinfo, &priv->modeinfo, sizeof(priv->modeinfo));

	/* Palette */
	if (vis->palette) {
		free(vis->palette);
		vis->palette = NULL;
	}
	if (priv->savepalette) {
		free(priv->savepalette);
		priv->savepalette = NULL;
	}
	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		int len = 1 << GT_DEPTH(tm->graphtype);

		vis->palette = malloc(len * sizeof(ggi_color));
		if (vis->palette == NULL) return GGI_EFATAL;
		priv->savepalette = malloc(sizeof(int) * (len*3));
		if (priv->savepalette == NULL) return GGI_EFATAL;

		/* Set an initial palette */
		ggiSetColorfulPalette(vis);
	}

	/* Set up pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->size  = GT_SIZE(gt);
	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(gt);

	switch (GT_SCHEME(gt)) {

	case GT_PALETTE:
	case GT_GREYSCALE:
		LIBGGI_PIXFMT(vis)->clut_mask = (1 << GT_DEPTH(gt)) - 1;
		break;

	case GT_TRUECOLOR:
		GGIDPRINT_MODE("display-vgl: RGB %d:%d:%d offsets %d:%d:%d\n",
			priv->modeinfo.vi_pixel_fsizes[VGL_RED_INDEX],
			priv->modeinfo.vi_pixel_fsizes[VGL_GREEN_INDEX],
			priv->modeinfo.vi_pixel_fsizes[VGL_BLUE_INDEX],
			priv->modeinfo.vi_pixel_fields[VGL_RED_INDEX],
			priv->modeinfo.vi_pixel_fields[VGL_GREEN_INDEX],
			priv->modeinfo.vi_pixel_fields[VGL_BLUE_INDEX]);

		LIBGGI_PIXFMT(vis)->red_mask =
		((1 << priv->modeinfo.vi_pixel_fsizes[VGL_RED_INDEX]) - 1) <<
			priv->modeinfo.vi_pixel_fields[VGL_RED_INDEX];

		LIBGGI_PIXFMT(vis)->green_mask =
		((1 << priv->modeinfo.vi_pixel_fsizes[VGL_GREEN_INDEX]) - 1) <<
			priv->modeinfo.vi_pixel_fields[VGL_GREEN_INDEX];
			
		LIBGGI_PIXFMT(vis)->blue_mask =
		((1 << priv->modeinfo.vi_pixel_fsizes[VGL_BLUE_INDEX]) - 1) <<
			priv->modeinfo.vi_pixel_fields[VGL_BLUE_INDEX];
		break;

	case GT_TEXT:
		/* Assumes VGA text */
		LIBGGI_PIXFMT(vis)->texture_mask = 0x00ff;
		LIBGGI_PIXFMT(vis)->fg_mask = 0x0f00;
		LIBGGI_PIXFMT(vis)->bg_mask = 0xf000;
		break;
	}
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	_ggiZapMode(vis, 0);

	for(id = 1; 0 == GGI_vgl_getapi(vis, id, sugname, args); id++) {
		if (_ggiOpenDL(vis, sugname, args, NULL)) {
			fprintf(stderr,"display-vgl: Can't open the %s (%s) library.\n",
				sugname, args);
			return GGI_EFATAL;
		} else {
			GGIDPRINT("Success in loading %s (%s)\n", sugname, args);
		}
	}

	if (!priv->vgl_use_db) {
		vis->opdraw->putpixel		= GGI_vgl_putpixel;
		vis->opdraw->putpixel_nc	= GGI_vgl_putpixel_nc;
		vis->opdraw->getpixel		= GGI_vgl_getpixel;
		vis->opdraw->drawpixel		= GGI_vgl_drawpixel;
		vis->opdraw->drawpixel_nc	= GGI_vgl_drawpixel_nc;
		vis->opdraw->drawhline		= GGI_vgl_drawhline;
		vis->opdraw->drawhline_nc	= GGI_vgl_drawhline_nc;
		vis->opdraw->drawvline		= GGI_vgl_drawvline;
		vis->opdraw->drawvline_nc	= GGI_vgl_drawvline_nc;
		vis->opdraw->drawbox		= GGI_vgl_drawbox;
		vis->opdraw->drawline		= GGI_vgl_drawline;
		vis->opdraw->puthline		= GGI_vgl_puthline;
		vis->opdraw->putbox		= GGI_vgl_putbox;
	} else {
		vis->opdraw->setorigin		= GGI_vgl_setorigin;
	}

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		vis->opcolor->setpalvec = GGI_vgl_setpalvec;
	}

	if(priv->vgl_use_db) {
		for(i = 0; i<tm->frames; i++)
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat =
								LIBGGI_PIXFMT(vis);
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

#define WANT_MODELIST
#include "../common/modelist.inc"

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_vgl_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	vgl_priv *priv = VGL_PRIV(vis);
	int ret, err = 0;

	if (vis==NULL || tm==NULL)
		return -1;

	if (tm->visible.x == GGI_AUTO)
		tm->visible.x = tm->virt.x;
	if (tm->visible.y == GGI_AUTO)
		tm->visible.y = tm->virt.y;
	
	if (tm->graphtype == GGI_AUTO) {
		err=_GGIcheckautobpp(vis, tm, VGL_PRIV(vis)->availmodes);
	} else if ((ret =
		    _GGIcheckonebpp(vis, tm, VGL_PRIV(vis)->availmodes))
		   != 0) {
		err = -1;
		if (ret == 1)
			_GGIgethighbpp(vis, tm, VGL_PRIV(vis)->availmodes);
	}
	
	if(tm->virt.x==GGI_AUTO) tm->virt.x = tm->visible.x;
	if(tm->virt.y==GGI_AUTO) tm->virt.y = tm->visible.y;

	/* Force virtual to equal visible */
	if(tm->virt.x != tm->visible.x) {
		tm->virt.x = tm->visible.x;
		err = -1;
	}
	if (tm->virt.y != tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}
 
	if (priv->vgl_use_db) {
		/* Multiple frames are not implemented yet... */
		if (tm->frames != 1 && tm->frames != GGI_AUTO) {
			err = -1;
		}
		tm->frames = 1;
	} else {
		tm->frames = 0;
	}

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

	if (tm->size.x != GGI_AUTO || tm->size.y != GGI_AUTO) {
		err = -1;
	}
	tm->size.x = tm->size.y = GGI_AUTO;

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_vgl_getmode(ggi_visual *vis,ggi_mode *tm)
{
	GGIDPRINT("In GGIgetmode(%p,%p)\n",vis,tm);
	if (vis==NULL)
		return -1;

	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));
	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_vgl_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
	return 0;
}

/*********************/
/* change the origin */
/*********************/
int GGI_vgl_setorigin(ggi_visual *vis,int x,int y)
{
	ggi_mode *mode=LIBGGI_MODE(vis);

	if ( x<0 || x> mode->virt.x-mode->visible.x ||
	     y<0 || y> mode->virt.y-mode->visible.y )
	     return -1;

	if (VGLPanScreen(VGLDisplay, x, y))
		return -1;
	
	return 0;
}
