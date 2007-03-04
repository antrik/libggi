/* $Id: mode.c,v 1.23 2007/03/04 18:26:42 soyt Exp $
******************************************************************************

   Graphics library for GGI.  Events for AA target.

   Copyright (C) 1998-2000 Steve Cheng     [steve@ggi-project.org]
   Copyright (C) 1997 Andreas Beck         [becka@ggi-project.org]

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
#include <ggi/display/aa.h>
#include <ggi/internal/ggi_debug.h>

#include <ggi/input/aa.h>


int GGI_aa_getapi(struct ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	*arguments = '\0';
	switch(num) {
		case 0: strcpy(apiname, "display-aa");
			return 0;
		case 1: strcpy(apiname, "generic-stubs");
			return 0;
		case 2: strcpy(apiname, "generic-color");
			return 0;
		case 3:
			if (LIBGGI_GT(vis) != GT_8BIT)
				return GGI_ENOMATCH;

			sprintf(apiname, "generic-linear-8");
			return 0;
	}
	return GGI_ENOMATCH;
}


static int _GGIdomode(struct ggi_visual *vis)
{
	int err,id;
	char sugname[GGI_MAX_APILEN],args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);

	if(LIBGGI_PAL(vis)->priv){
		free(LIBGGI_PAL(vis)->priv);
		LIBGGI_PAL(vis)->priv = NULL;
	}			
		
	LIBGGI_PAL(vis)->priv = _ggi_malloc(sizeof(aa_palette));
		
	if(LIBGGI_PAL(vis)->clut.data){
		free(LIBGGI_PAL(vis)->clut.data);
		LIBGGI_PAL(vis)->clut.data = NULL;			
	}
		
	LIBGGI_PAL(vis)->clut.data = _ggi_malloc(256*sizeof(ggi_color));
	LIBGGI_PAL(vis)->clut.size = 256;	

	for(id=1;0==GGI_aa_getapi(vis,id,sugname,args);id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				sugname, args, NULL);
		if (err) {
			fprintf(stderr,"display-aa: Can't open the %s (%s) library.\n",
				sugname,args);
			/* In our special case, fail is always fatal. */
			return GGI_EFATAL;
		} else {
			DPRINT("Success in loading %s (%s)\n",sugname,args);
		}
	}

  LIBGGI_PAL(vis)->getPrivSize = GGI_aa_getPrivSize;
  LIBGGI_PAL(vis)->setPalette  = GGI_aa_setPalette;
	
	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);

	return 0;
}

int GGI_aa_flush(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_aa_priv *priv = AA_PRIV(vis);
	aa_palette  *pal  = (aa_palette*)(LIBGGI_PAL(vis)->priv);
		
	if (tryflag == 0) {
		if (ggTryLock(priv->aalock))
			return 0;
	} else {
		ggLock(priv->aalock);
	}
	
	aa_renderpalette(priv->context, *pal, &aa_defrenderparams, 
			 x / AA_SCRMULT_X,
			 y / AA_SCRMULT_Y,
			 (x + w + AA_SCRMULT_X-1) / AA_SCRMULT_X,
			 (y + h + AA_SCRMULT_Y-1) / AA_SCRMULT_Y);
	
	aa_flush(priv->context);

	ggUnlock(priv->aalock);

	return 0;
}

/* AAlib's mode-setting is broken in several ways, in that
 * it is impossible to query a device's capabilities without
 * actually setting a mode and at least the terminal driver takes
 * suggestions too seriously and sets impossible modes like (80x25
 * on a 100x50 console).
 *
 * We try to work around as much as possible.
 */
static int _GGIcursorycheckmode(struct ggi_visual *vis, ggi_mode *tm)
{
	int err = 0;
	
	/* FIXME AAlib can do greyscale but LibGGI has no rendering libs
	 * for it ! */
#if 0
	ggi_graphtype gt = tm->graphtype;
	if(GT_SIZE(gt) != 8) {
		if(GT_SIZE(gt) != GT_AUTO) err = -1;
		GT_SETSIZE(gt, 8);
	}

	if(GT_DEPTH(gt) != 4 && GT_DEPTH(gt) != 8) {
		if(GT_DEPTH(gt) != GT_AUTO) err = -1;
		GT_SETDEPTH(gt, 8);
	}

	switch(GT_SCHEME(gt)) {
	case GT_GREYSCALE:
		/* Okay at both 4-bit and 8-bit */
		break;

	case GT_PALETTE:
		if(GT_DEPTH(gt)==8) break;	
		/* else fallthrough to fail */

	default:
		if(GT_SCHEME(gt)!=GGI_AUTO) err = -1;
		if(GT_DEPTH(gt)==4)
			GT_SETSCHEME(gt, GT_GREYSCALE);
		else
			GT_SETSCHEME(Gt, GT_PALETTE);
		break;
	}
	tm->graphtype = gt;
#else
	if(tm->graphtype!=GT_8BIT) {
		if(tm->graphtype!=GGI_AUTO) err = -1;
		tm->graphtype = GT_8BIT;
	}
#endif

	/* Yeah, right ... */
	if(tm->frames != 1) {
		if(tm->frames!=GGI_AUTO) err = -1;
		tm->frames = 1;
	}
	
	/* Satisfy visible size over virtual size, unless that is
	 * GGI_AUTO then use virtual size for both.
	 * So what comes out is either:
	 *  1. vis=virt=GGI_AUTO
	 *  2. vis=virt=somevalue
	 */
	
	if(tm->visible.x==GGI_AUTO) {
		/* Round up to fit screen resolution */
		if(tm->virt.x != GGI_AUTO && tm->virt.x % AA_SCRMULT_X) {
			err = -1;
			tm->virt.x = AA_SCRMULT_X*(tm->virt.x/AA_SCRMULT_X+1);
		}

		tm->visible.x = tm->virt.x;
	} else {
		if(tm->virt.x!=tm->visible.x && tm->virt.x!=GGI_AUTO)
			err = -1;

		if(tm->visible.x % AA_SCRMULT_X) {
			err = -1;
			tm->visible.x = AA_SCRMULT_X*(tm->visible.x/AA_SCRMULT_X+1);
		}
		
		tm->virt.x = tm->visible.x;
	}
			
	if(tm->visible.y==GGI_AUTO) {
		if(tm->virt.y != GGI_AUTO && tm->virt.y % AA_SCRMULT_Y) {
			err = -1;
			tm->virt.y = AA_SCRMULT_Y*(tm->virt.y/AA_SCRMULT_Y+1);
		}

		tm->visible.y = tm->virt.y;
	} else {
		if(tm->virt.y!=tm->visible.y && tm->virt.y!=GGI_AUTO)
			err = -1;
		
		if(tm->visible.y % AA_SCRMULT_Y) {
			err = -1;
			tm->visible.y = AA_SCRMULT_Y*(tm->visible.y/AA_SCRMULT_Y+1);
		}
		
		tm->virt.y = tm->visible.y;
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



int GGI_aa_setmode(struct ggi_visual *vis,ggi_mode *tm)
{ 
	ggi_aa_priv *priv; 
	struct aa_hardware_params ap = aa_defparams;
	int nx, ny, err;

	priv = AA_PRIV(vis);

	err = _GGIcursorycheckmode(vis, tm);
	if (err) {
		DPRINT_MODE("display-aa: setmode: cursory checkmode failed\n");
		return err;
	}

	if (priv->opmansync) MANSYNC_ignore(vis);

	_GGI_aa_freedbs(vis);

    	if (priv->context)
    		aa_close(priv->context);
    	
	if (tm->visible.x != GGI_AUTO) {
		ap.width = 
#if 0 /* for some reason this doesn't work */
		ap.minwidth =
		ap.maxwidth = 
#endif
		tm->visible.x / AA_SCRMULT_X;
	}
	if (tm->visible.y != GGI_AUTO) {
		ap.height = 
#if 0
		ap.minheight =
		ap.maxheight = 
#endif
		tm->visible.y / AA_SCRMULT_Y;
	}

	priv->context = aa_autoinit(&ap);
	if (!priv->context) {
		DPRINT_MODE("display-aa: setmode: aa_autoinit failed\n");
		/* If user is negotiating with setmode, assume: */
		tm->visible.x = tm->virt.x = 80*AA_SCRMULT_X;
		tm->visible.y = tm->virt.y = 25*AA_SCRMULT_Y;
		return GGI_ENODEVICE;
	}

	nx = aa_imgwidth(AA_PRIV(vis)->context);
	ny = aa_imgheight(AA_PRIV(vis)->context);
	
	err = 0;
	if(nx != tm->visible.x) {
		if(tm->visible.x != GGI_AUTO) err = -1;
		tm->visible.x = tm->virt.x = nx;
	}
	if(ny != tm->visible.y) {
		if(tm->visible.y != GGI_AUTO) err = -1;
		tm->visible.y = tm->virt.y = ny;
	}
	if(err)	{	
		DPRINT_MODE("display-aa: setmode: AAlib returned different"
				"mode than requested, failing\n");
		aa_close(priv->context);
		return GGI_ENOMATCH;
	}

	/* Set up pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	LIBGGI_PIXFMT(vis)->depth = GT_DEPTH(tm->graphtype);
	LIBGGI_PIXFMT(vis)->size = GT_SIZE(tm->graphtype);
	LIBGGI_PIXFMT(vis)->clut_mask = 0xFF;

	_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
	LIBGGI_APPBUFS(vis)[0]->frame = 0;
	LIBGGI_APPBUFS(vis)[0]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
	LIBGGI_APPBUFS(vis)[0]->read = 
	LIBGGI_APPBUFS(vis)[0]->write = aa_image(priv->context);
	LIBGGI_APPBUFS(vis)[0]->layout = blPixelLinearBuffer;
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.stride = tm->virt.x;
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	aa_autoinitkbd(priv->context,AA_SENDRELEASE);
	aa_autoinitmouse(priv->context,AA_MOUSEALLMASK);

	if (priv->inp) {
		struct gii_aa_cmddata_setparam data;

		data.context = priv->context;

		ggControl(priv->inp->channel, GII_CMDCODE_AASETPARAM,
			&data);
	}
	
	if (priv->opmansync) MANSYNC_cont(vis);
	
	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	return _GGIdomode(vis);
}




/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_aa_checkmode(struct ggi_visual *vis,ggi_mode *tm)
{
	int nx, ny;
	int err = _GGIcursorycheckmode(vis, tm);
	
	if(AA_PRIV(vis)->context) {
		/* Don't shut down existing mode, just copy
		 * its current parameters */
		nx = aa_imgwidth(AA_PRIV(vis)->context);
		ny = aa_imgheight(AA_PRIV(vis)->context);
	}
	else {
		/* Test by setting mode , ARGH */
		struct aa_hardware_params ap = aa_defparams;
		aa_context *c;

		if(tm->visible.x != GGI_AUTO) 
			ap.width = tm->visible.x / AA_SCRMULT_X;
		
		if(tm->visible.y != GGI_AUTO)
			ap.height = tm->visible.y / AA_SCRMULT_Y;
		
		c = aa_autoinit(&ap);
		if(c) {
			nx = aa_imgwidth(c);
			ny = aa_imgheight(c);
			aa_close(c);
		} else {
			DPRINT_MODE("display-aa: checkmode: aa_autoinit failed\n");
			/* Last ditch attempt */
			nx = 80*AA_SCRMULT_X;
			ny = 25*AA_SCRMULT_Y;
		}
	}

	if((tm->visible.x!=nx && tm->visible.x!=GGI_AUTO) ||
	   (tm->visible.y!=ny && tm->visible.y!=GGI_AUTO))
	{
		err = -1;
	}
	
	tm->visible.x = tm->virt.x = nx;
	tm->visible.y = tm->virt.y = ny;

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_aa_getmode(struct ggi_visual *vis,ggi_mode *tm)
{
	DPRINT("In GGI_aa_getmode(%p,%p)\n",vis,tm);
	if (vis==NULL)
		return GGI_EARGINVAL;
	
	/* We assume the mode in the visual to be o.k. */
	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));

	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_aa_setflags(struct ggi_visual *vis, uint32_t flags)
{
	ggi_aa_priv *priv = AA_PRIV(vis);

	LIBGGI_FLAGS(vis)=flags;

	if (priv->opmansync) {
		MANSYNC_SETFLAGS(vis,flags);
	}
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

/*************************/
/* get private cmap size */
/*************************/
size_t GGI_aa_getPrivSize(struct ggi_visual *vis)
{
 	return sizeof(aa_palette);
}
