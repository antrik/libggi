/* $Id: mode.c,v 1.27 2005/06/17 11:45:07 cegger Exp $
******************************************************************************

   SVGAlib target: mode management

   Copyright (C) 1997      Jason McMullan	[jmcc@ggi-project.org]
   Copyright (C) 1998      Steve Cheng		[steve@ggi-project.org]
   Copyright (C) 1998-1999 Marcus Sundberg	[marcus@ggi-project.org]

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
#include <termios.h>

#include "config.h"
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */
#include <ggi/display/svgalib.h>

#include "../common/pixfmt-setup.inc"


/* This is exported from svgalib */
extern int __svgalib_tty_fd;


int _ggi_svgalib_setmode(int mode)
{
	struct termios temp_term;
	int dorestore = 1;
	int ret;

	/* Save and restore the termios on stdin (fd 0) around the call
	   to vga_setmode().  This is a workaround because SVGAlib
	   messes with the termios in vga_setmode() and this interferes
	   with linux-kbd input driver.
	 */
	if (tcgetattr(__svgalib_tty_fd, &temp_term) < 0) {
		dorestore = 0;
		perror("display-svga: tcgetattr failed");
	}

	ret = vga_setmode(mode);

	if (dorestore) {
		if (tcsetattr(__svgalib_tty_fd, TCSANOW, &temp_term) < 0) {
			perror("display-svga: tcsetattr failed");
		}
	}

	return ret;
}

static int GGI_svga_setorigin(ggi_visual *vis,int x,int y)
{
	struct svga_priv *priv = SVGA_PRIV(vis);
	if (x != 0 || y<0 || y > LIBGGI_VIRTY(vis) )
		return GGI_ENOSPACE;
	
	vga_setdisplaystart(priv->frame_size * vis->d_frame_num +
			    GT_ByPP(LIBGGI_GT(vis)) * 
			    (y * LIBGGI_VIRTX(vis) + x));
	
	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}

int GGI_svga_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return 0;
}
	
int GGI_svga_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';
	switch(num) {
		case 0:
			strcpy(apiname, "display-svga");
			return 0;
		case 1:
			strcpy(apiname, "generic-stubs");
			return 0;
		case 2:
			strcpy(apiname, "generic-color");
			return 0;
		case 3:
			if(SVGA_PRIV(vis)->ismodex)
				return GGI_ENOMATCH;
			
			if(SVGA_PRIV(vis)->isbanked) {
				strcpy(apiname, "helper-vgagl");
				strcpy(arguments, "sVgALIb");
				return 0;
			}

			/* else islinear */
			sprintf(apiname, "generic-linear-%d", GT_SIZE(LIBGGI_GT(vis)));
			return 0;
	}
			
	return GGI_ENOMATCH;
}

static int GGI_svga_make_modeline(ggi_mode *tm)
{
	int modenum;
	char modestr[64];
	const char *colors;

	DPRINT("SVGAlib trying for bitdepth %d, %d.\n", tm->graphtype, GT_DEPTH(tm->graphtype));
	/* See GGI_svga_checkmode() for details... */
	switch(tm->graphtype) {
	case GT_1BIT : colors = "2"; break;
	case GT_4BIT : colors = "16"; break;
	case GT_8BIT : colors = "256"; break;
	case GT_15BIT: colors = "32K"; break;
	case GT_16BIT: colors = "64K"; break;
	case GT_24BIT : colors = "16M"; break;
	case GT_32BIT : colors = "16M32"; break;
	default: return GGI_ENOMATCH;
	}

	/* Form a SVGAlib mode number */

	snprintf(modestr, 64, "G%dx%dx%s",
		tm->visible.x, tm->visible.y, colors);
	DPRINT("SVGAlib trying modeline=%s.\n", modestr);

 	modenum = vga_getmodenumber(modestr);
	DPRINT("SVGAlib modeline=%s returns modenum=%d.\n", modestr, modenum);

	return modenum;
}

int GGI_svga_setmode(ggi_visual *vis, ggi_mode *tm)
{ 
	struct svga_priv *priv = SVGA_PRIV(vis);
	int modenum;
	vga_modeinfo *modeinfo;
	int err = 0;
	int id, i;
	char sugname[GGI_MAX_APILEN];
	char args[GGI_MAX_APILEN];

	err = GGI_svga_checkmode(vis, tm);
	if (err) return err;
	
	modenum = GGI_svga_make_modeline(tm);
	if (_ggi_svgalib_setmode(modenum) != 0) return GGI_EFATAL;

	modeinfo = vga_getmodeinfo(modenum);
	DPRINT("Setting SVGAlib mode number %d.\n", modenum);

	/* Set requested mode for visual. */
	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	/* Palette */
	if (LIBGGI_PAL(vis)->clut.data) {
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	if (LIBGGI_PAL(vis)->priv) {
		free(LIBGGI_PAL(vis)->priv);
		LIBGGI_PAL(vis)->priv = NULL;
	}
	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		int len = 1 << GT_DEPTH(tm->graphtype);

		LIBGGI_PAL(vis)->clut.size = len;
		LIBGGI_PAL(vis)->clut.data = malloc(len * sizeof(ggi_color));
		if (LIBGGI_PAL(vis)->clut.data == NULL) return GGI_EFATAL;
		LIBGGI_PAL(vis)->priv = malloc(sizeof(int) * (len*3));
		if (LIBGGI_PAL(vis)->priv == NULL) return GGI_EFATAL;

		/* Set an initial palette. */
		ggiSetColorfulPalette(vis);
	}

	/* Layout of display */
	priv->islinear = 0;
	priv->ismodex  = 0;
	priv->isbanked = 0;
	if (((modeinfo->flags & CAPABLE_LINEAR)
	     && vga_setlinearaddressing() >= tm->virt.x*tm->virt.y)) {
		priv->islinear = 1;
  	} else {
		if ((modeinfo->flags & IS_MODEX)) {
			priv->ismodex = 1;
		} else {
			if (modeinfo->linewidth*tm->virt.y <= 1<<16)
				priv->islinear = 1;
			else {
				priv->isbanked = 1;
			}
		}
	}
	
	/* Pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), tm->graphtype);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* DirectBuffers */
	_GGI_svga_freedbs(vis);
	priv->frame_size = tm->virt.x * tm->virt.y * modeinfo->bytesperpixel;
	DPRINT("Setting up DirectBuffers, islinear=%d, frame_size=%d, frames=%d\n",
		  priv->islinear, priv->frame_size, LIBGGI_MODE(vis)->frames);
	for (i=0; priv->islinear && (i < tm->frames); i++) {
		ggi_directbuffer *buf;

		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

		buf = LIBGGI_APPBUFS(vis)[i];

		if (0 == i) {
			buf->read = vga_getgraphmem();
			buf->write = buf->read;
		}

		buf->frame = i;
		buf->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		buf->read = ((char *)LIBGGI_APPBUFS(vis)[0]->read) + i * priv->frame_size;
		buf->write = buf->read;
		buf->layout = blPixelLinearBuffer;
		buf->buffer.plb.stride = modeinfo->linewidth;
		buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

		DPRINT("Setting up DirectBuffer %d, stride=%d\n",
			i, buf->buffer.plb.stride);

		err = vga_claimvideomemory(priv->frame_size * tm->frames);
		if (err) {
			fprintf(stderr, "display-svga: "
				"Can't allocate enough display memory:"
				"%d bytes.\n", modeinfo->bytesperpixel * 
				tm->virt.x * tm->virt.y * tm->frames);
			return GGI_EFATAL;
		}
	}

/* virt.x != visible.x should be possible with this,
   but currently it doesn't work. */
#if 0
	vga_setlogicalwidth( GT_ByPPP(tm->virt.x, tm->graphtype) );
#endif

	_ggiZapMode(vis, 0);

	for(id=1;0==GGI_svga_getapi(vis,id,sugname,args);id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
		if (err) {
			fprintf(stderr,"display-svga: "
				"Can't open the %s (%s) library.\n",
				sugname, args);
			return GGI_EFATAL;
		} else {
			DPRINT("Success in loading %s (%s)\n",
				  sugname, args);
		}
	}

	vis->opdraw->setorigin		= GGI_svga_setorigin;
	vis->opdraw->setdisplayframe    = GGI_svga_setdisplayframe;
	
	if (priv->ismodex) {
		vis->opdraw->putpixel_nc	= GGI_svga_putpixel_nc;
		vis->opdraw->putpixel		= GGI_svga_putpixel;
		vis->opdraw->getpixel		= GGI_svga_getpixel;
		vis->opdraw->drawpixel_nc	= GGI_svga_drawpixel_nc;
		vis->opdraw->drawpixel		= GGI_svga_drawpixel;
		vis->opdraw->drawhline_nc	= GGI_svga_drawhline_nc;
		vis->opdraw->drawhline		= GGI_svga_drawhline;
		vis->opdraw->drawvline_nc	= GGI_svga_drawvline_nc;
		vis->opdraw->drawvline		= GGI_svga_drawvline;
		vis->opdraw->drawbox		= GGI_svga_drawbox;
		vis->opdraw->puthline		= GGI_svga_puthline;
		vis->opdraw->putbox		= GGI_svga_putbox;
		vis->opdraw->setreadframe       = GGI_svga_setreadframe;
		vis->opdraw->setwriteframe      = GGI_svga_setwriteframe;
	}

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
	        LIBGGI_PAL(vis)->setPalette  = GGI_svga_setPalette;
		LIBGGI_PAL(vis)->getPrivSize = GGI_svga_getPrivSize;
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

#define WANT_MODELIST
#include "../common/modelist.inc"

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_svga_checkmode(ggi_visual *vis,ggi_mode *tm)
{
	svga_priv *priv = SVGA_PRIV(vis);
	vga_modeinfo *vmi = NULL;
	int mode = 0;


	int ret, err = 0;

	if (vis==NULL || tm==NULL)
		return GGI_EARGINVAL;
	

	if (tm->visible.x == GGI_AUTO)
		tm->visible.x = tm->virt.x;
	if (tm->visible.y == GGI_AUTO)
		tm->visible.y = tm->virt.y;
	
	if (tm->graphtype == GGI_AUTO) {
		err=_GGIcheckautobpp(vis, tm, SVGA_PRIV(vis)->availmodes);
	} else if ((ret =
		    _GGIcheckonebpp(vis, tm, SVGA_PRIV(vis)->availmodes))
		   != 0) {
 		err = -1;
		if (ret == 1)
			_GGIgethighbpp(vis, tm, SVGA_PRIV(vis)->availmodes);
	}
	
	if(tm->virt.x==GGI_AUTO) tm->virt.x = tm->visible.x;
	if(tm->virt.y==GGI_AUTO) tm->virt.y = tm->visible.y;

	mode = GGI_svga_make_modeline(tm);
	vmi = vga_getmodeinfo(mode);

	/* SVGAlib doesn't seem to support virtual dimensions
	   Force them to be the same as visible size */
	if(tm->virt.x != tm->visible.x) {
		tm->virt.x = tm->visible.x;
		err = -1;
	}
	if (tm->virt.y != tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}

	if (tm->frames == GGI_AUTO) {
	  tm->frames = 1;
	}
	/* Only support frames when we can support linear access 
	 * and we have enough video memory. */
	if (!(vmi->flags & CAPABLE_LINEAR)
	   || (vmi->memory < (vmi->bytesperpixel * tm->virt.x * 
		tm->virt.y * tm->frames)))
	{
		tm->frames = 1;
	}

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

	err = _ggi_physz_figure_size(tm, priv->physzflags, &(priv->physz),
				0, 0, tm->visible.x, tm->visible.y);

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_svga_getmode(ggi_visual *vis,ggi_mode *tm)
{
	DPRINT("In GGIgetmode(%p,%p)\n",vis,tm);
	if (vis==NULL)
		return GGI_EARGINVAL;

	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));
	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_svga_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */
	return 0;
}
