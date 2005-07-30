/* $Id: mode.c,v 1.12 2005/07/30 11:38:50 cegger Exp $
******************************************************************************

   LibGGI GLIDE target - Mode management.

   Copyright (C) 1997-1998 Jon Taylor		[taylorj@ecs.csus.edu]
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
#include <unistd.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/glide.h>
#include "../../default/color/color.h"

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"


static void
GGI_glide_gcchanged(ggi_visual *vis, int mask)
{
	glide_priv *priv = GLIDE_PRIV(vis);

	if ((mask & GGI_GCCHANGED_CLIP)) {
		int 
			tlx = LIBGGI_GC(vis)->cliptl.x,
			tly = LIBGGI_GC(vis)->cliptl.y,
			brx = LIBGGI_GC(vis)->clipbr.x,
			bry = LIBGGI_GC(vis)->clipbr.y;
		if (tlx > 0) tlx--;
		if (tly > 0) tly--;
		if (brx < LIBGGI_VIRTX(vis)) brx--;
		if (bry < LIBGGI_VIRTY(vis)) bry--;

		grClipWindow(tlx, tly, brx, bry);
	}
	if ((mask & GGI_GCCHANGED_FG)) {
		color_truepriv *colorpriv = vis->colorpriv;
		priv->fgvertex.r
			= SSHIFT(LIBGGI_GC(vis)->fg_color
				 & LIBGGI_PIXFMT(vis)->red_mask,
				 colorpriv->red_unmap - 8);
		priv->fgvertex.g
			= SSHIFT(LIBGGI_GC(vis)->fg_color
				 & LIBGGI_PIXFMT(vis)->green_mask,
				 colorpriv->green_unmap - 8);
		priv->fgvertex.b =
			SSHIFT(LIBGGI_GC(vis)->fg_color
			       & LIBGGI_PIXFMT(vis)->blue_mask,
			       colorpriv->blue_unmap - 8);
		priv->fgvertex.a = 255;
	}
	if ((mask & GGI_GCCHANGED_BG)) {
		color_truepriv *colorpriv = vis->colorpriv;
		priv->bgvertex.r
			= SSHIFT(LIBGGI_GC(vis)->bg_color
				 & LIBGGI_PIXFMT(vis)->red_mask,
				 colorpriv->red_unmap - 8);
		priv->bgvertex.g
			= SSHIFT(LIBGGI_GC(vis)->bg_color
				 & LIBGGI_PIXFMT(vis)->green_mask,
				 colorpriv->green_unmap - 8);
		priv->bgvertex.b =
			SSHIFT(LIBGGI_GC(vis)->bg_color
			       & LIBGGI_PIXFMT(vis)->blue_mask,
			       colorpriv->blue_unmap - 8);
		priv->bgvertex.a = 255;
	}
}


static int glide_acquire(ggi_resource *res, uint32_t actype)
{
	ggi_directbuffer *dbuf;
	ggi_visual *vis;
	int bufnum;
	int type;
	GrBuffer_t buffer;
	GrLfbInfo_t inf;

	DPRINT_MISC("glide_acquire(%p, 0x%x) called\n", res, actype);

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}

#ifdef DBUF_EXCLUSIVE
	/* Despite what the Glide API says it seems we _can_ read and write
	   at the same time... */
	if ((actype & (GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) ==
	    (GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EEXCLUSIVE;
	}
#endif
	vis = res->priv;
	dbuf = res->self;
	bufnum = dbuf->frame;

	if (res->count > 0) {
#ifdef DBUF_EXCLUSIVE
		if (actype != res->curactype) return GGI_EEXCLUSIVE;
#else
		if (dbuf->write != NULL) {
			dbuf->read = dbuf->write;
		} else if (dbuf->read != NULL) {
			dbuf->write = dbuf->read;
		}
#endif
		res->count++;
		return 0;
	}

#ifndef DBUF_EXCLUSIVE
	type = GR_LFB_WRITE_ONLY | GR_LFB_IDLE;
#else
	if (actype == GGI_ACTYPE_WRITE) {
		type = GR_LFB_WRITE_ONLY | GR_LFB_IDLE;
	} else {
		type = GR_LFB_READ_ONLY | GR_LFB_IDLE;
	}
#endif

	if (bufnum == vis->d_frame_num) {
		buffer = GR_BUFFER_FRONTBUFFER;
	} else {
		buffer = GR_BUFFER_BACKBUFFER;
	}

	if (! grLfbLock(type, buffer, GLIDE_PRIV(vis)->write_mode,
			GR_ORIGIN_UPPER_LEFT, FXFALSE, &inf)) {
		return GGI_EUNKNOWN;
	}

	if (actype & GGI_ACTYPE_WRITE) {
		dbuf->write = inf.lfbPtr;
	}
	if (actype & GGI_ACTYPE_READ) {
		dbuf->read = inf.lfbPtr;
	}
	
	res->curactype |= actype;
	res->count++;

	DPRINT_MISC("glide_acquire - success, count: %d\n", res->count);

	return 0;
}


static int glide_release(ggi_resource *res)
{
	ggi_directbuffer *dbuf;
	ggi_visual *vis;
	int bufnum;
	GrBuffer_t buffer;

	DPRINT_MISC("glide_release(%p) called\n", res);

	if (res->count < 1) return GGI_ENOTALLOC;

	res->count--;

	if (res->count > 0) return 0;

	vis = res->priv;
	dbuf = res->self;
	bufnum = dbuf->frame;

	if (bufnum == vis->d_frame_num) {
		buffer = GR_BUFFER_FRONTBUFFER;
	} else {
		buffer = GR_BUFFER_BACKBUFFER;
	}

#ifndef DBUF_EXCLUSIVE
	grLfbUnlock(GR_LFB_WRITE_ONLY, buffer);
#else
	if ((res->curactype & GGI_ACTYPE_WRITE)) {
		grLfbUnlock(GR_LFB_WRITE_ONLY, buffer);
	}
	if ((res->curactype & GGI_ACTYPE_READ)) {
		grLfbUnlock(GR_LFB_READ_ONLY, buffer);
	}
#endif

	dbuf->write = NULL;
	dbuf->read  = NULL;

	return 0;
}


int GGI_glide_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	grSstIdle();
	
	return 0;
}

int GGI_glide_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	*arguments = '\0';
	switch(num) {
	case 0:
		strcpy(apiname, "display-glide");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		return 0;
	}
	
	return GGI_ENOMATCH;
}

/* The minimum frequency glide can do */
#define GGIGLIDE_MINFREQ 60

static const struct {
	int vfreq;
	GrScreenRefresh_t glidefreq;
} glide_frequencies[] = {
	{  60, GR_REFRESH_60Hz  },
	{  70, GR_REFRESH_70Hz  },
	{  72, GR_REFRESH_72Hz  },
	{  75, GR_REFRESH_75Hz  },
	{  80, GR_REFRESH_80Hz  },
	{  85, GR_REFRESH_85Hz  },
	{  90, GR_REFRESH_90Hz  },
	{ 100, GR_REFRESH_100Hz },
	{ 120, GR_REFRESH_120Hz },
	{   0, GR_REFRESH_NONE  }
};

static GrScreenRefresh_t
getglidefreq(glide_priv *priv, ggi_mode *mode)
{
	int i = 0;
	GrScreenRefresh_t ret = GR_REFRESH_NONE;
	
	while (mode->visible.y * glide_frequencies[i].vfreq
	       <= priv->maxhfreq * 1000 &&
	       glide_frequencies[i].vfreq <= priv->maxvfreq) {
		ret = glide_frequencies[i].glidefreq;
		i++;
	}

	return ret;
}

static const struct {
	int16_t	x, y;
	GrScreenResolution_t glideres;
} glide_resolutions[] = {
#if 0 /* As of Glide 2.51 these modes are not supported */
	{  320,  200, GR_RESOLUTION_320x200 },
	{  320,  240, GR_RESOLUTION_320x240 },
	{  400,  256, GR_RESOLUTION_400x256 },
	{  400,  300, GR_RESOLUTION_400x300 },
	{  512,  256, GR_RESOLUTION_512x256 },
	{  640,  200, GR_RESOLUTION_640x200 },
	{  640,  350, GR_RESOLUTION_640x350 },
#endif
	{  512,  384, GR_RESOLUTION_512x384 },
	{  640,  400, GR_RESOLUTION_640x400 },
	{  640,  480, GR_RESOLUTION_640x480 },
	{  856,  480, GR_RESOLUTION_856x480 },
	{  800,  600, GR_RESOLUTION_800x600 },
	{  960,  720, GR_RESOLUTION_960x720 },
#ifdef GR_RESOLUTION_1024x768
	{ 1024,  768, GR_RESOLUTION_1024x768 },
#endif
/* The following requires more than 4MB of framebuffer memory, someone with
   an SLI board should try if they work */
#ifdef GR_RESOLUTION_1280x1024
	{ 1280, 1024, GR_RESOLUTION_1280x1024 },
	{ 1600, 1200, GR_RESOLUTION_1600x1200 },
#endif

	{    0,    0, GR_RESOLUTION_NONE }
};
	
static int
res2glideres(glide_priv *priv, ggi_mode *mode,
	     GrScreenResolution_t *glideres)
{
	int i, err = -1, x = 640, y = 480;
	GrScreenResolution_t res = GR_RESOLUTION_640x480;

	for (i = 0; glide_resolutions[i].x != 0; i++) {
		if (glide_resolutions[i].x * glide_resolutions[i].y * 4
		    > priv->fbmem) {
			DPRINT_MODE("res2glideres: breaking on nomem\n");
			break;
		}
		if (glide_resolutions[i].y * GGIGLIDE_MINFREQ
		    > priv->maxhfreq * 1000) {
			DPRINT_MODE("res2glideres: breaking on highfreq\n");
			break;
		}
			
		x = glide_resolutions[i].x;
		y = glide_resolutions[i].y;
		res = glide_resolutions[i].glideres;

		if ((mode->visible.x == glide_resolutions[i].x || 
		     mode->visible.x == GGI_AUTO) &&
		    (mode->visible.y == glide_resolutions[i].y ||
		     mode->visible.y == GGI_AUTO)) {
			err = 0;
			break;
		}
		if (mode->visible.x <= glide_resolutions[i].x &&
		    mode->visible.y <= glide_resolutions[i].y) {
			err = -1;
			break;
		}
	}

	mode->visible.x = x;
	mode->visible.y = y;
	*glideres = res;

	return err;
}

		
int GGI_glide_setmode(ggi_visual *vis, ggi_mode *mode)
{
	int rc = GGI_OK;
	glide_priv *priv = GLIDE_PRIV(vis);
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];
	GrScreenResolution_t resolution;
	GrScreenRefresh_t    freq;
	GrLfbInfo_t inf;
	int id;
	int i;

	rc = GGI_glide_checkmode(vis, mode);
	if (rc < 0) return rc;

	if (res2glideres(priv, mode, &resolution) != 0) {
		LIB_ASSERT(0, "Invalid Glide resolution!");
	}
	freq = getglidefreq(priv, mode);

	if (priv->setmodesuccess) {
		grSstWinClose();
	}
	
	DPRINT_MODE("resolution: 0x%02x, freq: 0x%02x\n", resolution, freq);
	if (grSstWinOpen(0, resolution, freq,
			 GR_COLORFORMAT_ABGR, GR_ORIGIN_UPPER_LEFT, 2, 0)
	    != FXTRUE) {
		DPRINT_MODE("FAIL!!!");
		return GGI_ENOMATCH;
	}

	/* Fill in ggi_pixelformat */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), mode->graphtype);

	switch(mode->graphtype)	{
	case GT_15BIT:
		priv->src_format = GR_LFB_SRC_FMT_555;
		priv->write_mode = GR_LFBWRITEMODE_555;
		priv->bytes_per_pixel = 2;
		break;
		
	case GT_16BIT:
		priv->src_format = GR_LFB_SRC_FMT_565;
		priv->write_mode = GR_LFBWRITEMODE_565;
		priv->bytes_per_pixel = 2;
		break;
		
	case GT_24BIT:
		priv->src_format = GR_LFB_SRC_FMT_888;
		priv->write_mode = GR_LFBWRITEMODE_888;
		priv->bytes_per_pixel = 3;
		break;
		
	case GT_32BIT:
		priv->src_format = GR_LFB_SRC_FMT_8888;
		priv->write_mode = GR_LFBWRITEMODE_8888;
		priv->bytes_per_pixel = 4;
		break;
	default:
		LIB_ASSERT(0, "Glide: Illegal mode!");
	}
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	vis->d_frame_num = 0;
	vis->r_frame_num = 0;
	vis->w_frame_num = 0;
	priv->readbuf = GR_BUFFER_FRONTBUFFER;
	priv->writebuf = GR_BUFFER_FRONTBUFFER;
	grRenderBuffer(GR_BUFFER_FRONTBUFFER);
	
	vis->d_frame_num = 0;
	
	_GGI_glide_freedbs(vis);

	if (mode->graphtype == GT_16BIT && /* 3DFX always have 16bpp framebuffer */
	    grLfbLock(GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER,
			GLIDE_PRIV(vis)->write_mode,
			GR_ORIGIN_UPPER_LEFT, FXFALSE, &inf)) {

		/* Set up directbuffers */
		for (i = 0; i < mode->frames; i++) {
			ggi_resource *res;
		
			res = malloc(sizeof(ggi_resource));
			if (res == NULL) return GGI_EFATAL;
			LIBGGI_APPLIST(vis)->last_targetbuf
				= _ggi_db_add_buffer(LIBGGI_APPLIST(vis),
						     _ggi_db_get_new());
			LIBGGI_APPBUFS(vis)[i]->resource = res;
			LIBGGI_APPBUFS(vis)[i]->resource->acquire
				= glide_acquire;
			LIBGGI_APPBUFS(vis)[i]->resource->release
				= glide_release;
			LIBGGI_APPBUFS(vis)[i]->resource->self
				= LIBGGI_APPBUFS(vis)[i];
			LIBGGI_APPBUFS(vis)[i]->resource->priv = vis;
			LIBGGI_APPBUFS(vis)[i]->resource->count = 0;
			LIBGGI_APPBUFS(vis)[i]->resource->curactype = 0;
		
			LIBGGI_APPBUFS(vis)[i]->frame = i;
			LIBGGI_APPBUFS(vis)[i]->type
				= GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
			LIBGGI_APPBUFS(vis)[i]->read
				= LIBGGI_APPBUFS(vis)[i]->write
				= NULL;
			LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride
				= inf.strideInBytes;
			LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat
				= LIBGGI_PIXFMT(vis);
			DPRINT_MODE("DB: %d, addr: %p, stride: %d\n", i, 
				       LIBGGI_APPBUFS(vis)[i]->read,
				       LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride);
		}
		LIBGGI_APPLIST(vis)->first_targetbuf
			= LIBGGI_APPLIST(vis)->last_targetbuf - (mode->frames-1);
		grLfbUnlock(GR_LFB_WRITE_ONLY, GR_BUFFER_FRONTBUFFER);
	}

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	_ggiZapMode(vis, 0);
	
	for (id = 1; GGI_glide_getapi(vis, id, libname, libargs) == 0; id++) {
		int err;

		err = _ggiOpenDL(vis, libname, libargs, NULL);
		if (err) {
			fprintf(stderr, "display-glide: Error opening the "
				"%s (%s) library\n", libname, libargs);
			return err;
		}
		DPRINT ("Success in loading %s (%s)\n", libname, libargs);
	}
	
	/* Drawing functions */
		/* framebuffer */
	vis->opdraw->setreadframe=GGI_glide_setreadframe;
	vis->opdraw->setwriteframe=GGI_glide_setwriteframe;
	vis->opdraw->setdisplayframe=GGI_glide_setdisplayframe;
		/* misc */
	vis->opdraw->fillscreen=GGI_glide_fillscreen;
		/* pixels */
	vis->opdraw->drawpixel_nc = GGI_glide_drawpixel;
	vis->opdraw->drawpixel    = GGI_glide_drawpixel;
	vis->opdraw->putpixel_nc  = GGI_glide_putpixel;
	vis->opdraw->putpixel     = GGI_glide_putpixel;
	vis->opdraw->getpixel     = GGI_glide_getpixel;
		/* lines */
	vis->opdraw->drawline     = GGI_glide_drawline;
	vis->opdraw->drawhline_nc = GGI_glide_drawhline;
	vis->opdraw->drawhline    = GGI_glide_drawhline;
	vis->opdraw->drawvline_nc = GGI_glide_drawvline;
	vis->opdraw->drawvline    = GGI_glide_drawvline;
	vis->opdraw->puthline     = GGI_glide_puthline;
	vis->opdraw->putvline     = GGI_glide_putvline;
	vis->opdraw->gethline     = GGI_glide_gethline;
	vis->opdraw->getvline     = GGI_glide_getvline;
		/* boxes */
	vis->opdraw->drawbox=GGI_glide_drawbox;
/*	vis->opdraw->copybox=GGI_glide_copybox;*/
	vis->opdraw->putbox=GGI_glide_putbox;
	vis->opdraw->getbox=GGI_glide_getbox;

	/* GC management */
	vis->opgc->gcchanged = GGI_glide_gcchanged;

	/* Text */
	switch (GT_SIZE(mode->graphtype)) {
	case 16:
		vis->opdraw->putc = GGI_glide16_putc;
		break;
	case 32:
		vis->opdraw->putc = GGI_glide32_putc;
		break;
	default:
		break;
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	priv->setmodesuccess = 1;

	return 0;
}

int GGI_glide_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	GrScreenResolution_t resolution; /* Dummy */
	int err = 0;

	APP_ASSERT(vis != NULL, "glide: Visual NULL in GGIcheckmode");

	/* handle AUTO */
	_GGIhandle_ggiauto(mode, 640, 480);

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	if (tm->frames == GGI_AUTO) {
		tm->frames = 1;
	} else if (tm->frames < 1) {
		err = GGI_ENOMATCH;
		tm->frames = 1;
	} else if (tm->frames > 2) {
		err = GGI_ENOMATCH;
		tm->frames = 2;
	}

	if (res2glideres(GLIDE_PRIV(vis), tm, &resolution) != 0) {
		err = GGI_ENOMATCH;
	}

	if (tm->virt.x < tm->visible.x) {
		tm->virt.x = tm->visible.x;
		err = GGI_ENOMATCH;
	}
	if (tm->virt.y < tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = GGI_ENOMATCH;
	}

	if ((tm->dpp.x != 1 && tm->dpp.x != GGI_AUTO) ||
	    (tm->dpp.y != 1 && tm->dpp.y != GGI_AUTO)) {
		err = GGI_ENOMATCH;
	}
	tm->dpp.x = tm->dpp.y = 1;
	if (err) return err;
#if 0
	err = _ggi_physz_figure_size(mode, GLIDE_PRIV(vis)->physzflags,
				&(GLIDE_PRIV(vis)->physz),
				0, 0, mode->visible.x, mode->visible.y);
#endif
	return err;
}

/*
** Get the current mode
*/
int GGI_glide_getmode(ggi_visual *vis, ggi_mode *tm)
{
	APP_ASSERT(vis != NULL,
		"display-glide: GGIgetmode: Visual == NULL");
	
	/* We assume the mode in the visual to be OK 
	*/
	memcpy(tm, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}
