/* $Id: mode.c,v 1.3 2003/02/14 17:51:38 fries Exp $
******************************************************************************
 *
 * wsfb(3) target: mode management
 *
 *
 * Copyright (c) 2003 Todd T. Fries <todd@OpenBSD.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL
 * THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
******************************************************************************/

#include <sys/ioccom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/wsfb.h>

#include "../common/pixfmt-setup.inc"

static int do_mmap(ggi_visual *);

int GGI_wsfb_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	//struct wsfb_priv *priv = WSFB_PRIV(vis);
	GGIDPRINT("GGI_wsfb_getapi called\n");

	switch(num) {
		case 0:
			strcpy(apiname, "display-wsfb");
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
			sprintf(apiname, "generic-linear-%d%s", 
				GT_SIZE(LIBGGI_GT(vis)),
				(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) 
				? "-r" : "");
			strcpy(arguments, "");
			return 0;
		default:
			break;
	}
			
	return -1;
}

int GGI_wsfb_setmode(ggi_visual *vis, ggi_mode *tm)
{ 
	struct wsfb_priv *priv = WSFB_PRIV(vis);
	//ggi_graphtype gt = tm->graphtype;
	//unsigned long modenum = 0;
	//char sugname[256];
	//char args[256];
	int err = 0;
	//int id, i;
	//int pixelBytes;

	if (vis == NULL) {
		GGIDPRINT("vis == NULL");
		return -1;
	}

	GGIDPRINT_MODE("display-wsfb: setmode %dx%d#V%dx%d.F%d[0x%02x]\n",
		tm->visible.x, tm->visible.y,
		tm->virt.x, tm->virt.y,
		tm->frames, tm->graphtype);

	err = GGI_wsfb_checkmode(vis, tm);
	if (err) {
		GGIDPRINT("error from checkmode during GGI_wsfb_setmode\n");
		return -1;
	}

	switch(tm->graphtype) {
		case GT_8BIT : break;
		default:
			return -1;
	}
	if ((GT_SCHEME(tm->graphtype) == GT_PALETTE)) {
		int nocols = 1 << GT_DEPTH(tm->graphtype);

		vis->palette = _ggi_malloc(nocols * sizeof(ggi_color));
		vis->opcolor->setpalvec = GGI_wsfb_setpalvec;
		/* Initialize palette */
		ggiSetColorfulPalette(vis);
	}

	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	_ggiZapMode(vis, 0);

	do_mmap(vis);

	{
		int i;
		/* show some data */
		for(i=0; i<priv->size/4/16; i++) {
			priv->base[i]=0;
		}
		for(; i < priv->size/4/8; i++) {
			priv->base[i]=0xffffffff;
		}
	}
   


	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

#define WANT_MODELIST
#include "../common/modelist.inc"

/**********************************/
/* check any mode (text/graphics) */
/* return -1 on any error, but set things to what should be working values */
/**********************************/
int GGI_wsfb_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	wsfb_priv *priv = WSFB_PRIV(vis);
	int err = 0;

	if (vis == NULL)
		return -1;

	if(tm->visible.x != priv->info.width && tm->visible.x != GGI_AUTO)
		err = -1;
	else if(tm->virt.x != priv->info.width && tm->virt.x != GGI_AUTO)
		err = -1;

	if(tm->visible.y != priv->info.height && tm->visible.y != GGI_AUTO)
		err = -1;
	else if(tm->virt.y != priv->info.height && tm->virt.y != GGI_AUTO)
		err = -1;

	if(tm->graphtype != GGI_AUTO && tm->graphtype != GT_8BIT)
		err = -1;	

	if(tm->frames != GGI_AUTO && tm->frames != 1)
		err = -1;

	if(tm->dpp.x != GGI_AUTO && tm->dpp.x != 1)
		err = -1;
	else if(tm->dpp.y != GGI_AUTO && tm->dpp.y != 1)
		err = -1;

	tm->visible.x = tm->virt.x = priv->info.width;
	tm->visible.y = tm->virt.y = priv->info.height;

	tm->graphtype = GT_8BIT;
	
	tm->frames = 1;

	tm->dpp.x = 1;
	tm->dpp.y = 1;

	return err;

	/* FIXKME */
#if 0
	int err = 0;

	GGIDPRINT_MODE("display-wsfb: setmode %dx%d#V%dx%d.F%d[0x%02x]\n",
		tm->visible.x, tm->visible.y,
		tm->virt.x, tm->virt.y,
		tm->frames, tm->graphtype);

	if (vis==NULL || tm==NULL)
		return -1;

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
 

	return err;
#endif
}

/************************/
/* get the current mode */
/************************/
int GGI_wsfb_getmode(ggi_visual *vis,ggi_mode *tm)
{
	LIBGGI_APPASSERT(vis != NULL, "GGIgetmode(wsfb): Visual == NULL");

	GGIDPRINT("In GGIgetmode(%p,%p)\n",vis,tm);

	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));
	return 0;
}


int
GGI_wsfb_setpalvec(ggi_visual *vis, int start, int len, ggi_color *colormap)
{
	wsfb_priv *priv = LIBGGI_PRIVATE(vis);
	int nocols = 1 << GT_DEPTH(LIBGGI_GT(vis));

	GGIDPRINT_COLOR("display-wsfb: SetPalVec(%d,%d)\n", start, len);
	
	if (start == GGI_PALETTE_DONTCARE) {
		start = 0;
	}

	if ((start < 0) || (len < 0) || (start+len > nocols)) {
		return -1;
	}

	memcpy(vis->palette+start, colormap, len*sizeof(ggi_color));

	priv->cmap.index  = start;
	priv->cmap.count  = len;

	for (; len > 0; start++, colormap++, len--) {
		priv->cmap.red[start]   = colormap->r;
		priv->cmap.green[start] = colormap->g;
		priv->cmap.blue[start]  = colormap->b;
	}

	if (ioctl(priv->fd, WSDISPLAYIO_PUTCMAP, &priv->cmap) < 0) {
		GGIDPRINT_COLOR("display-wsfb: PUTCMAP failed.");
		return -1;
	}

	return 0;
}


static int
do_mmap(ggi_visual *vis)
{
	wsfb_priv *priv = LIBGGI_PRIVATE(vis);
	ggi_mode *mode = LIBGGI_MODE(vis);
	ggi_graphtype gt = mode->graphtype;
	ggi_directbuffer *buf;

	if (ioctl(priv->fd, WSDISPLAYIO_GETCMAP, &priv->ocmap) < 0) {
		GGIDPRINT("getcmap failed\n");
		return -1;
	}
	priv->cmap.red   = (char *)malloc(256);
	priv->cmap.green = (char *)malloc(256);
	priv->cmap.blue  = (char *)malloc(256);
	
	priv->base = mmap(0, priv->mapsize, PROT_READ|PROT_WRITE, MAP_SHARED,
		priv->fd, priv->Base);

	if (priv->base == (void *)-1) {
		GGIDPRINT("mmap failed: %s %s\n",
			priv->devname, strerror(errno));
		return -1;
	}


	fprintf(stderr,"mmap offset: 0x%lx\n",priv->base);

	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);

	_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

	buf = LIBGGI_APPBUFS(vis)[0];

	buf->frame = 0;
	buf->type  = GGI_DB_SIMPLE_PLB|GGI_DB_NORMAL;
	buf->read  = (uint8 *) priv->base;
	buf->write = buf->read;
	buf->page_size = 0;

	buf->layout = blPixelLinearBuffer;

	buf->buffer.plb.stride = priv->linebytes;
	buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));


	return 0;
}
