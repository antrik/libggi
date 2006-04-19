/* $Id: mode.c,v 1.22 2006/04/19 21:22:22 cegger Exp $
******************************************************************************

   Tile target: setting modes

   Copyright (C) 1998 Steve Cheng	[steve@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "config.h"
#include <ggi/display/tile.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "../common/gt-auto.inc"

int GGI_tile_flush_db(struct ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	int i, width, height;
	int nx, ny, nw, nh;
	ggi_visual_t currvis;

#if 0
	DPRINT_MISC("GGI_tile_flush_db(%p, %i, %i, %i, %i, %i) entered\n",
			vis, x, y, w, h, tryflag);

	for(i = 0; i < priv->numvis; i++) {
		currvis = priv->vislist[i].vis;
		width = priv->vislist[i].size.x;
		height = priv->vislist[i].size.y;
		
		ggiGetBox(vis, 
			priv->vislist[i].origin.x+vis->origin_x, 
			priv->vislist[i].origin.y+vis->origin_y,
			width, height, priv->buf);
		ggiPutBox(currvis, 0, 0, width, height, priv->buf);

#else

	/* This is a better CrossBlit. 
	*/

	int rowadd, stride;
	uint8_t *buf;

	DPRINT_MISC("GGI_tile_flush_db(%p, %i, %i, %i, %i, %i) entered\n",
			vis, x, y, w, h, tryflag);

	if (priv->d_frame == NULL) {
		/* happens, when mansync flushes before a mode is set */
		return GGI_OK;
	}
		
	if(priv->use_db) {
		MANSYNC_ignore(vis);
	}

	rowadd = (LIBGGI_PIXFMT(vis)->size+7)/8/sizeof(uint8_t);
	stride = priv->d_frame->buffer.plb.stride;

	for(i = 0; i<priv->numvis; i++) {
		struct ggi_visual *_vis;

		currvis = priv->vislist[i].vis;
		width = priv->vislist[i].size.x;
		height = priv->vislist[i].size.y - 1;
		_vis = GGI_VISUAL(currvis);

		buf = (uint8_t*)priv->d_frame->read +
				stride * (priv->vislist[i].origin.y + vis->origin_y + height) +
				rowadd * (priv->vislist[i].origin.x + vis->origin_x);

		do {
			ggiPutHLine(currvis, 0, height, width, buf);
			buf -= stride;
		} while(height--);
#endif

		nx = x - priv->vislist[i].origin.x;
		nw = w - priv->vislist[i].origin.x;
		ny = y - priv->vislist[i].origin.y;
		nh = h - priv->vislist[i].origin.y;
		if (nx < 0) nx = 0;
		else if (nx > LIBGGI_X(_vis)) continue;
		if (ny < 0) ny = 0;
		else if (ny > LIBGGI_Y(_vis)) continue;
		if (nx + nw > LIBGGI_X(_vis)) nw = LIBGGI_X(_vis) - nx;
		if (ny + nh > LIBGGI_Y(_vis)) nh = LIBGGI_Y(_vis) - ny;

		_ggiInternFlush(_vis, nx, ny, nw, nh, tryflag);
	}
	
	if(priv->use_db) {
		MANSYNC_cont(vis);
	}

	DPRINT_MISC("GGI_tile_flush_db: leaving\n");

	return 0;
}

int GGI_tile_getapi(struct ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';
	switch(num) {
		case 0:
			strcpy(apiname, "display-tile");
			return 0;
		case 1:
			strcpy(apiname, "generic-stubs");
			return 0;
		case 2:
			if(!TILE_PRIV(vis)->use_db)
				return GGI_ENOMATCH;

			if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
				sprintf(apiname, "generic-text-%u",
					GT_SIZE(LIBGGI_GT(vis))); 
				return 0;
			}

			sprintf(apiname, "generic-linear-%u%s", 
				GT_SIZE(LIBGGI_GT(vis)),
				(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) 
				? "-r" : "");
			return 0;
	}

	return GGI_ENOMATCH;
}


static int _GGIdomode(struct ggi_visual *vis)
{
	int err, id;
	char sugname[GGI_MAX_APILEN], args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);

	for (id=1; 0==GGI_tile_getapi(vis, id, sugname, args); id++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(), sugname, args, NULL);
		if (err) {
			fprintf(stderr, "display-tile: Can't open the %s (%s) library.\n",
				sugname, args);
			return GGI_EFATAL;
		} else {
			DPRINT("Success in loading %s (%s)\n", sugname, args);
		}
	}

	if (!TILE_PRIV(vis)->use_db) {
		vis->opdraw->drawpixel_nc=GGI_tile_drawpixel_nc;
		vis->opdraw->drawpixel=GGI_tile_drawpixel;
		vis->opdraw->putpixel_nc=GGI_tile_putpixel_nc;
		vis->opdraw->putpixel=GGI_tile_putpixel;
		vis->opdraw->getpixel=GGI_tile_getpixel;

		vis->opdraw->drawhline_nc=GGI_tile_drawhline_nc;
		vis->opdraw->drawhline=GGI_tile_drawhline;
		vis->opdraw->puthline=GGI_tile_puthline;
		vis->opdraw->gethline=GGI_tile_gethline;

		vis->opdraw->drawvline_nc=GGI_tile_drawvline_nc;
		vis->opdraw->drawvline=GGI_tile_drawvline;
		vis->opdraw->putvline=GGI_tile_putvline;
		vis->opdraw->getvline=GGI_tile_getvline;

		vis->opdraw->drawbox=GGI_tile_drawbox;
		vis->opdraw->putbox=GGI_tile_putbox;
		vis->opdraw->getbox=GGI_tile_getbox;

		vis->opdraw->copybox=GGI_tile_copybox;
		vis->opdraw->fillscreen=GGI_tile_fillscreen;

		vis->opdraw->setdisplayframe=GGI_tile_setdisplayframe;
		vis->opdraw->setreadframe=GGI_tile_setreadframe;
		vis->opdraw->setwriteframe=GGI_tile_setwriteframe;

		vis->opdraw->drawline=GGI_tile_drawline;
#if 0	/* Not implemented */
		vis->opdraw->putc=GGI_tile_putc;
		vis->opdraw->puts=GGI_tile_puts;
#endif

		vis->opgc->gcchanged=GGI_tile_gcchanged;
	} 
	else {
		vis->opdraw->setdisplayframe=GGI_tile_setdisplayframe_db;
		vis->opdraw->setorigin=GGI_tile_setorigin;
	}

	vis->opcolor->mapcolor=GGI_tile_mapcolor;
	vis->opcolor->unmappixel=GGI_tile_unmappixel;
	vis->opcolor->setpalvec=GGI_tile_setpalvec;
	vis->opcolor->getpalvec=GGI_tile_getpalvec;
	
	ggiIndicateChange(vis->stem, GGI_CHG_APILIST);

	return 0;
}

int GGI_tile_setmode(struct ggi_visual *vis,ggi_mode *tm)
{ 
	ggi_tile_priv *priv;
	/*int, currbuf, maxbuf=0;*/
	ggi_visual_t currvis;
	ggi_mode sugmode;
	int depth, err, i;

	DPRINT_MODE("GGI_tile_setmode(%p, %p) entered\n",
			(void *)vis, (void *)tm);
	err = GGI_tile_checkmode(vis, tm);
	if (err) return err;
	
	priv = TILE_PRIV(vis);

	depth = GT_SIZE(tm->graphtype);

	if (priv->use_db) {
		char *fbaddr;

		MANSYNC_ignore(vis);

		_GGI_tile_freedbs(vis);

		/* Set up DirectBuffer(s) */
		for (i = 0; i<tm->frames; i++) {
			fbaddr = malloc(LIBGGI_FB_SIZE(tm));

			if (!fbaddr) {
				fprintf(stderr, "display-tile: Out of memory for framebuffer!\n");
				return GGI_ENOMEM;
			}

			_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

			LIBGGI_APPBUFS(vis)[i]->frame = i;
			LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
			LIBGGI_APPBUFS(vis)[i]->read = LIBGGI_APPBUFS(vis)[i]->write = fbaddr;
			LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
				= GT_ByPPP(tm->virt.x, tm->graphtype);
		}
	}
	
	for (i = 0; i<priv->numvis; i++) {
		struct ggi_visual *_vis;

		currvis = priv->vislist[i].vis;
		sugmode = *tm;
		sugmode.visible.x = priv->vislist[i].size.x;
		sugmode.visible.y = priv->vislist[i].size.y;
		sugmode.virt.x = sugmode.virt.y = GGI_AUTO;
		_vis = GGI_VISUAL(currvis);

		/* Multiple buffering is handled by us in DB mode */
		if(priv->use_db)
			sugmode.frames = 1;

		DPRINT("Setting mode for visual #%d...\n", i);

		/* Set mode mantra from lib/libggi/mode.c.  Be careful here.
		   See GGIcheckmode() for why we do this. */

		err = ggiSetMode(currvis, &sugmode);
		if (err) {
			fprintf(stderr, "display-tile: Error setting mode on visual #%d!\n", i);
			return err;
		}

		DPRINT("Success setting mode for visual #%d\n", i);

		if(!priv->use_db) {
			/* Adjust clipping rectangle for mode dimensions. */

			priv->vislist[i].clipbr.x = priv->vislist[i].origin.x + priv->vislist[i].size.x;
			if(priv->vislist[i].clipbr.x > tm->virt.x)
				priv->vislist[i].clipbr.x = tm->virt.x;

			priv->vislist[i].clipbr.y = priv->vislist[i].origin.y + priv->vislist[i].size.y;
			if(priv->vislist[i].clipbr.y > tm->virt.y)
				priv->vislist[i].clipbr.y = tm->virt.y;
		}

#if 0
		/* This is to determine the largest buffer size needed for copybox */
		currbuf = priv->vislist[i].size.x * priv->vislist[i].size.y;
		if(currbuf > maxbuf) maxbuf=currbuf;
	}

	if (priv->buf) {
		free(priv->buf);
		priv->buf=NULL;
	}
	
	priv->buf = malloc(maxbuf*depth/8);
	if (!priv->buf) {
		fprintf(stderr, "display-tile: Out of memory for copybox buffer\n");
		return GGI_ENOMEM;
#endif
	}

	/* Assume first visual's pixelformat properties */
	memcpy(LIBGGI_PIXFMT(vis), LIBGGI_PIXFMT(GGI_VISUAL(priv->vislist[0].vis)), 
		sizeof(ggi_pixelformat));

	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	err = _GGIdomode(vis);
	if (err != 0) return err;

	if (priv->use_db) {
		DPRINT_MISC("GGI_tile_setmode: setting up directbuffer\n");
		for (i = 0; i<tm->frames; i++)
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

		priv->d_frame = LIBGGI_APPBUFS(vis)[0];

		DPRINT_MISC("GGI_tile_setmode: call MANSYNC_SETFLAGS");
		MANSYNC_SETFLAGS(vis, LIBGGI_FLAGS(vis));
		MANSYNC_cont(vis);
	}

	DPRINT_MISC("GGI_tile_setmode: leaving\n");	
	return 0;
}

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_tile_checkmode(struct ggi_visual *vis,ggi_mode *tm)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);
	ggi_mode sugmode;
	int err, i;

	/* Find out the "bounding rectangle" for GGI_AUTO values */
	if(tm->virt.x==GGI_AUTO) {
		int x;
		tm->virt.x=0;
		for(i = 0; i<priv->numvis; i++) {
			x = priv->vislist[i].origin.x
				+ priv->vislist[i].size.x;
			if (x > tm->virt.x) tm->virt.x = x;
		}
	}
	if (tm->virt.y==GGI_AUTO) {
		int y;
		tm->virt.y=0;
		for(i = 0; i<priv->numvis; i++) {
			y = priv->vislist[i].origin.y
				+ priv->vislist[i].size.y;
			if (y > tm->virt.y) tm->virt.y = y;
		}
	}

	/* We ignore visible size */
	if(tm->visible.x==GGI_AUTO)
		tm->visible.x=tm->virt.x;
	if(tm->visible.y==GGI_AUTO)
		tm->visible.y=tm->virt.y;

	if(tm->frames==GGI_AUTO)
		tm->frames = 1;

	if (tm->size.x != GGI_AUTO || tm->size.y != GGI_AUTO) {
		err = -1;
	}
	tm->size.x = tm->size.y = GGI_AUTO;

	for(i = 0; i < priv->numvis; i++) {
		/* Note that we don't use ggiCheckMode() since we don't want 
		   GGI_AUTO to be substituted by GGI_DEFMODE values */
		   
		sugmode.frames = priv->use_db ? 1 : tm->frames;
		sugmode.visible.x = priv->vislist[i].size.x;
		sugmode.visible.y = priv->vislist[i].size.y;
		sugmode.virt.x = sugmode.virt.y = GGI_AUTO;
		sugmode.size.x = sugmode.size.y = GGI_AUTO;
		sugmode.graphtype = tm->graphtype;
		sugmode.dpp  = tm->dpp;
		sugmode.size = tm->size;

		err = ggiCheckMode(priv->vislist[i].vis, &sugmode);
		if (err) {
			/* Forget searching all visuals for the source of
			   error, it's way too complicated. Just say fail
			   with impossible mode */
			memset(tm, 0, sizeof(ggi_mode));

			fprintf(stderr,
				"display-tile: ggiCheckMode() on visual #%d error -- please explicitly specify correct mode instead.\n",
				i);
			return err;
		}
                /* Fill out any remaining GT_AUTO fields in the
                 * graphtype.
                 */
		tm->graphtype = _GGIhandle_gtauto(sugmode.graphtype);
	}

	return 0;
}

/************************/
/* get the current mode */
/************************/
int GGI_tile_getmode(struct ggi_visual *vis,ggi_mode *tm)
{
	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_tile_setflags(struct ggi_visual *vis,ggi_flags flags)
{
	ggi_tile_priv *priv = TILE_PRIV(vis);

	LIBGGI_FLAGS(vis) = flags;
	/* Unkown flags don't take. */
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC;

	if (priv->use_db) {
		MANSYNC_SETFLAGS(vis, flags);
	} else {
		int i;
		for (i = 0; i<priv->numvis; i++) {
			ggiSetFlags(priv->vislist[i].vis, flags);
		}
	}	

	return 0;
}

int GGI_tile_setorigin(struct ggi_visual *vis,int x,int y)
{
	ggi_mode *mode=LIBGGI_MODE(vis);

	if ( x<0 || x> mode->virt.x-mode->visible.x ||
	     y<0 || y> mode->virt.y-mode->visible.y )
	     return GGI_ENOSPACE;

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}
