/* $Id: mode.c,v 1.2 2001/05/31 21:55:21 skids Exp $
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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/svgalib.h>

#include "../common/pixfmt-setup.inc"


void _GGI_svga_freedbs(ggi_visual *);


int _ggi_svgalib_setmode(int mode)
{
	extern int __svgalib_tty_fd;
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

#if 0 /* Doesn't work yet */
int GGI_svga_setorigin(ggi_visual *vis,int x,int y)
{
	if (x != 0 || y<0 || y> LIBGGI_MODE(vis)->virt.y )
		return -1;
	
	vga_setdisplaystart(y * (LIBGGI_BPP(vis)*LIBGGI_MODE(vis)->virt.x)/8
			    + (x * LIBGGI_BPP(vis))/8);
	
	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}
#endif

int GGI_svga_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	return 0;
}
	
int GGI_svga_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	switch(num) {
		case 0:
			strcpy(apiname, "display-svga");
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
			if(SVGA_PRIV(vis)->ismodex)
				return -1;
			
			if(SVGA_PRIV(vis)->isbanked) {
				strcpy(apiname, "helper-vgagl");
				strcpy(arguments, "sVgALIb");
				return 0;
			}

			/* else islinear */
			sprintf(apiname, "generic-linear-%d", GT_SIZE(LIBGGI_MODE(vis)->graphtype));
			strcpy(arguments,"");
			return 0;
	}
			
	return -1;
}

int GGI_svga_setmode(ggi_visual *vis, ggi_mode *tm)
{ 
	struct svga_priv *priv = LIBGGI_PRIVATE(vis);
	char modestr[64], *colors;
	int modenum;
	vga_modeinfo *modeinfo;
	int err = 0;
	int id;
	char sugname[256];
	char args[256];

	err = GGI_svga_checkmode(vis, tm);
	if (err) return err;
	
	/* See GGI_svga_checkmode() for details... */
	switch(tm->graphtype) {
	case GT_1BIT : colors = "2"; break;
	case GT_4BIT : colors = "16"; break;
	case GT_8BIT : colors = "256"; break;
	case GT_15BIT: colors = "32K"; break;
	case GT_16BIT: colors = "64K"; break;
	case GT_24BIT : colors = "16M"; break;
	case GT_32BIT : colors = "16M32"; break;
	default: return -1;
	}

	/* Form a SVGAlib mode number */
	sprintf(modestr, "G%dx%dx%s", tm->visible.x, tm->visible.y, colors);
 	modenum = vga_getmodenumber(modestr);
	GGIDPRINT("Setting SVGAlib mode %d: %s\n", modenum, modestr);

	if (_ggi_svgalib_setmode(modenum) != 0) return GGI_EFATAL;

	modeinfo = vga_getmodeinfo(modenum);

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
	if (priv->islinear) {
		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[0]->frame = 0;
		LIBGGI_APPBUFS(vis)[0]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[0]->read = 
		LIBGGI_APPBUFS(vis)[0]->write = vga_getgraphmem();
		LIBGGI_APPBUFS(vis)[0]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[0]->buffer.plb.stride = modeinfo->linewidth;
		LIBGGI_APPBUFS(vis)[0]->buffer.plb.pixelformat
			= LIBGGI_PIXFMT(vis);
	}

/* virt.x != visible.x should be possible with this,
   but currently it doesn't work. */
#if 0
	vga_setlogicalwidth((tm->virt.x*GT_SIZE(tm->graphtype)/8);
#endif

	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	_ggiZapMode(vis, 0);

	for(id=1;0==GGI_svga_getapi(vis,id,sugname,args);id++) {
		err = _ggiOpenDL(vis, sugname, args, NULL);
		if (err) {
			fprintf(stderr,"display-svga: Can't open the %s (%s) library.\n",
				sugname, args);
			return GGI_EFATAL;
		} else {
			GGIDPRINT("Success in loading %s (%s)\n",
				  sugname, args);
		}
	}

	/* Doesn't work correct */
	/*vis->opdraw->setorigin=GGIsetorigin;*/
	
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
	}

	if (GT_SCHEME(tm->graphtype) == GT_PALETTE) {
		vis->opcolor->setpalvec = GGI_svga_setpalvec;
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
	svga_priv *priv = LIBGGI_PRIVATE(vis);

	int ret, err = 0;

	if (vis==NULL || tm==NULL)
		return -1;
	
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
 
	/* Multiple frames are not implemented yet... */
	if (tm->frames != 1 && tm->frames != GGI_AUTO) {
		err = -1;
	}
	tm->frames = 1;

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	tm->dpp.x = tm->dpp.y = 1;

	err = _ggi_figure_physz(tm, priv->physzflags, &(priv->physz),
				0, 0, tm->visible.x, tm->visible.y);

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_svga_getmode(ggi_visual *vis,ggi_mode *tm)
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
int GGI_svga_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;
	return 0;
}
