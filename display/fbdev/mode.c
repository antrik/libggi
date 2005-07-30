/* $Id: mode.c,v 1.27 2005/07/30 11:38:50 cegger Exp $
******************************************************************************

   Display-FBDEV

   Copyright (C) 1998      Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2005      Joseph Crayne	[oh.hello.joe@gmail.com]

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
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <linux/fb.h>

#include "config.h"
#include <ggi/display/fbdev.h>
#include <ggi/internal/ggi_debug.h>

#include "../common/gt-auto.inc"

#ifndef MAP_FAILED
# define MAP_FAILED ((void*)-1)
#endif

#define FB_BPP_AUTO	0xffffffff
#define FB_KLUDGE_FONTX	 8
#define FB_KLUDGE_FONTY	16

extern void GGI_fbdev_color0(ggi_visual *vis);
extern void GGI_fbdev_color_free(ggi_visual *vis);
extern void GGI_fbdev_color_setup(ggi_visual *vis);
int GGI_fbdev_mode_reset(ggi_visual *vis);

#include <ggi/display/modelist.h>

#define WANT_GENERIC_CHECKMODE
#include "../common/modelist.inc"

static 
int _GGI_fbdev_do_checkmode(ggi_visual *vis, ggi_mode *mode,
                            struct ggi_fbdev_timing **timing_);

static void _GGI_free_dbs(ggi_visual *vis) 
{
	int i;

	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static void clear_fbmem(void *mem, unsigned long len)
{
	unsigned long *memptr = mem;
	long i = len / sizeof(long);

	while (i--) {
		*memptr = 0;
	}
}


int GGI_fbdev_kgicommand(ggi_visual *vis, int cmd,void *args)
{
	return fbdev_doioctl(vis, (unsigned)cmd, args);
}
        

int GGI_fbdev_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	int size = GT_SIZE(LIBGGI_GT(vis));

	*arguments = '\0';

	switch(num) {

	case 0: strcpy(apiname, "display-fbdev");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;

	case 2: strcpy(apiname, "generic-color");
		return 0;

	case 3: if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%d", size);
			return 0;
		} 

		if (priv->fix.type == FB_TYPE_PLANES) {
			strcpy(apiname, "generic-planar");
			return 0;
		}

		if (priv->fix.type == FB_TYPE_INTERLEAVED_PLANES) {
			sprintf(apiname, "generic-%s",
				(priv->fix.type_aux == 2) ? 
				"iplanar-2p" : "ilbm");
			return 0;
		}

		sprintf(apiname, "generic-linear-%d", size);
		return 0;

	case 4:
		/* These allow an opportunity for fbdev.conf to choose
		   a different generic renderer, e.g. if the driver
		   in question wants a generic-linear-4r instead of 
		   a generic-linear-4r */
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "fb-generic-%2.2x-text-%d", 
				priv->orig_fix.accel, size);
			return 0;
		} 
		
		if (priv->fix.type == FB_TYPE_PLANES) {
			sprintf(apiname, "fb-generic-%2.2x-planar",
				priv->orig_fix.accel);
			return 0;
		}
		
		if (priv->fix.type == FB_TYPE_INTERLEAVED_PLANES) {
			sprintf(apiname, "fb-generic-%2.2x-%s",
				priv->orig_fix.accel ,
				(priv->fix.type_aux == 2) ? 
				"iplanar-2p" : "ilbm");
			return 0;
		}

		sprintf(apiname, "fb-generic-%2.2x-linear-%d", 
				priv->orig_fix.accel, size);
		return 0;
		break;

	case 5:
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "fb-accel-%2.2x-text-%d", 
				priv->orig_fix.accel, size);
			return 0;
		} 

		if (priv->fix.type == FB_TYPE_PLANES) {
			sprintf(apiname, "fb-accel-%2.2x-planar",
				priv->orig_fix.accel);
			return 0;
		}

		if (priv->fix.type == FB_TYPE_INTERLEAVED_PLANES) {
			sprintf(apiname, "fb-accel-%2.2x-%s",
				priv->orig_fix.accel ,
				(priv->fix.type_aux == 2) ? 
				"iplanar-2p" : "ilbm");
			return 0;
		}

		sprintf(apiname, "fb-accel-%2.2x-linear-%d", 
				priv->orig_fix.accel, size);
		return 0;
		break;
	}

	return GGI_ENOMATCH;
}


static void
ggimode2var(ggi_visual *vis, ggi_mode *mode, struct fb_var_screeninfo *var)
{
	var->xres = mode->visible.x * mode->dpp.x;
	var->yres = mode->visible.y * mode->dpp.y;
	var->xres_virtual = mode->virt.x * mode->dpp.x;
	var->yres_virtual = mode->virt.y * mode->dpp.y * mode->frames;

	var->grayscale = (GT_SCHEME(mode->graphtype) == GT_GREYSCALE) ? 1 : 0;

	if (GT_SIZE(mode->graphtype) == GT_AUTO) {
		var->bits_per_pixel = FB_BPP_AUTO;
	} else if (GT_SCHEME(mode->graphtype) == GT_TEXT) {
		var->bits_per_pixel = 0;
	} else {
		var->bits_per_pixel = GT_SIZE(mode->graphtype);
		if (GT_SIZE(mode->graphtype) == 16 &&
		    GT_DEPTH(mode->graphtype) == 15) {
			/* Yes, 15 bit mode is requested this way... */
			var->green.length = 5;
		}
	}
}

static void
timing2var( struct ggi_fbdev_timing *timing, struct fb_var_screeninfo *var,
		int frames )
{
	var->xres = timing->xres;
	var->yres = timing->yres;
	if (var->xres_virtual == 0) var->xres_virtual = timing->xres_virtual;
	if (var->yres_virtual == 0) var->yres_virtual = var->yres * frames;
	if (var->bits_per_pixel == FB_BPP_AUTO) {
		var->bits_per_pixel = timing->bits_per_pixel;
	}

	var->pixclock	  = timing->pixclock;
	var->right_margin = timing->right_margin;
	var->left_margin  = timing->left_margin;
	var->upper_margin = timing->upper_margin;
	var->lower_margin = timing->lower_margin;
	var->hsync_len    = timing->hsync_len;
	var->vsync_len    = timing->vsync_len;
	var->sync         = timing->sync;
	var->vmode        = timing->vmode;
}

static void
var2ggimode(ggi_visual *vis, struct fb_var_screeninfo *var, ggi_mode *mode,
	    int frames)
{
	mode->visible.x = var->xres;
	mode->visible.y = var->yres;
	mode->virt.x = var->xres_virtual;
	mode->virt.y = var->yres_virtual;

	/* All display-fbdev modes are fullscreen, so we will just
	 * put the framebuffer's fullscreen dimensions into the
	 * size field. */
	if( (signed)var->width >= 0 )
		mode->size.x = var->width;
	if( (signed)var->height >= 0 )
		mode->size.y = var->height;
	DPRINT_LIBS("var2ggimode stored size (%i, %i)\n", 
			mode->size.x, mode->size.y );

	if (frames) {
		mode->virt.y /= frames;
	}
	if (var->bits_per_pixel == 0) {
		mode->graphtype = GT_TEXT16;
		return;
	}
	GT_SETSIZE(mode->graphtype, var->bits_per_pixel);
	if (var->grayscale) {
		GT_SETSCHEME(mode->graphtype, GT_GREYSCALE);
		GT_SETDEPTH(mode->graphtype, var->bits_per_pixel);
	} else if (var->red.length + var->green.length + var->blue.length
		   > var->bits_per_pixel)
	{
		GT_SETSCHEME(mode->graphtype, GT_PALETTE);
		GT_SETDEPTH(mode->graphtype, var->bits_per_pixel);
	} else {
		GT_SETSCHEME(mode->graphtype, GT_TRUECOLOR);
		GT_SETDEPTH(mode->graphtype, var->red.length
			    + var->green.length + var->blue.length);
	}
            
}


#define SKIPWHITE(ptr) while(*(ptr)==' '||*(ptr)=='\t'||*(ptr)=='\n'){(ptr)++;}


static int do_change_mode(ggi_visual *vis, struct fb_var_screeninfo *var)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	ggi_mode *mode = LIBGGI_MODE(vis);
	ggi_graphtype gt = mode->graphtype;
	int err;

	/* If we already have set a mode, restore the old palette
	 * or gamma settings before setting a new one. 
	 */
	//GGI_fbdev_color0(vis);
	GGI_fbdev_color_free(vis);

	/* We need this to set virtual resolution correct */
	ggimode2var(vis, mode, var);

	var->activate = FB_ACTIVATE_NOW;

	/* Enable user space acceleration */
	var->accel_flags = 0;

	/* Tell framebuffer to change mode */
	if (fbdev_doioctl(vis, FBIOPUT_VSCREENINFO, var) < 0) {
		perror("display-fbdev: FBIOPUT_VSCREENINFO");
		return -1;
	}
	memcpy(&priv->var, var, sizeof(*var));

	if (fbdev_doioctl(vis, FBIOGET_FSCREENINFO, &priv->fix) < 0) {
		perror("display-fbdev: FBIOGET_FSCREENINFO");
		return GGI_EFATAL;
	}

	/* check whether we got what we asked for */
	err = 0;
	switch (GT_SCHEME(gt)) {

		case GT_GREYSCALE:
			if (! priv->var.grayscale ||
			    GT_DEPTH(gt) != priv->var.bits_per_pixel ||
#ifdef FB_TYPE_TEXT
			    priv->fix.type == FB_TYPE_TEXT ||
#endif
			    priv->fix.visual == FB_VISUAL_TRUECOLOR ||
			    priv->fix.visual == FB_VISUAL_DIRECTCOLOR)
			{
				err = GGI_EFATAL;
				DPRINT_MODE("display-fbdev: GT_GREYSCALE mode failed\n");
			}
			break;

		case GT_PALETTE:
			if (GT_DEPTH(gt) != priv->var.bits_per_pixel ||
#ifdef FB_TYPE_TEXT
			    priv->fix.type == FB_TYPE_TEXT ||
#endif
			    priv->fix.visual == FB_VISUAL_TRUECOLOR ||
			    priv->fix.visual == FB_VISUAL_DIRECTCOLOR)
			{
				err = GGI_EFATAL;
				DPRINT_MODE("display-fbdev: GT_PALETTE mode failed\n");
			}
			break;

		case GT_TRUECOLOR:
			if ((GT_SIZE(gt)  != priv->var.bits_per_pixel &&
			     GT_DEPTH(gt) != priv->var.bits_per_pixel) ||
#ifdef FB_TYPE_TEXT
			    priv->fix.type  == FB_TYPE_TEXT ||
#endif
			    (priv->fix.visual != FB_VISUAL_TRUECOLOR &&
			     priv->fix.visual != FB_VISUAL_DIRECTCOLOR))
			{
				err = GGI_EFATAL;
				DPRINT_MODE("display-fbdev: GT_TRUECOLOR mode failed\n");
			}
			break;
		
#ifdef FB_TYPE_TEXT
		case GT_TEXT:
			if (priv->fix.type != FB_TYPE_TEXT)
#endif
				err = GGI_EFATAL;
	}
	
	if (err) {
		DPRINT_MODE("display-fbdev: Santa passed us by :-(\n");
	} else {
		DPRINT_MODE("display-fbdev: Change mode OK.\n");
	}

	if (! err && (priv->fix.ypanstep == 0) && (mode->frames > 1)) {
		mode->frames = 1;
		DPRINT_MODE("display-fbdev: cannot do vertical panning "
				"-- frames reduced to 1.\n");
	}

	/* Reset panning and frames */
	vis->d_frame_num = vis->origin_x = vis->origin_y = 0;

	return err;
}


static int do_mmap(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	ggi_graphtype gt;
	int xres_in_bytes;
	int i;

	/* Unmap previous mapping */
	if (priv->fb_ptr != NULL) {
		_GGI_free_dbs(vis);
		/* Clear old contents */
		clear_fbmem(priv->fb_ptr, (unsigned)priv->mmap_size);
		munmap(priv->fb_ptr, (unsigned)priv->mmap_size);
	}

        /* calculate framebuffer size */
	gt = LIBGGI_GT(vis);

	switch (priv->fix.type) {
	case FB_TYPE_PLANES:
		if (priv->fix.line_length) {
			xres_in_bytes = priv->fix.line_length * GT_SIZE(gt);
		} else {
			xres_in_bytes = priv->var.xres_virtual * GT_SIZE(gt)/8;
		}
		break;
	case FB_TYPE_INTERLEAVED_PLANES:
		if (priv->fix.line_length) {
			xres_in_bytes = priv->fix.line_length *
				priv->var.bits_per_pixel * GT_SIZE(gt);
		} else {
			xres_in_bytes = priv->var.xres_virtual *
				priv->var.bits_per_pixel * GT_SIZE(gt) / 8;
		}
		break;
	default:
		if (priv->fix.line_length) {
			xres_in_bytes = priv->fix.line_length;
		} else {
			xres_in_bytes = priv->var.xres_virtual *
				priv->var.bits_per_pixel/8;
		}
		break;
	}

	priv->frame_size = xres_in_bytes * LIBGGI_VIRTY(vis);
	priv->fb_size = priv->frame_size * LIBGGI_MODE(vis)->frames;
#ifdef GGIFBDEV_MAP_EXACT
	priv->mmap_size = (priv->fb_size + 0x0fff) & ~0x0fff;
#else	
	priv->mmap_size = priv->fix.smem_len;
#endif
	
	DPRINT_MODE("display-fbdev: frame_size=0x%x fb_size=0x%x "
		    "mmap_size=0x%x\n", priv->frame_size,
		    priv->fb_size, priv->mmap_size);

	priv->fb_ptr = mmap(NULL, (unsigned)priv->mmap_size,
			    PROT_READ | PROT_WRITE, 
			    MAP_SHARED, LIBGGI_FD(vis), 0);

	DPRINT_MODE("display-fbdev: FB_PTR=%p\n", priv->fb_ptr);

	if (priv->fb_ptr == MAP_FAILED) {
		priv->fb_ptr = NULL;
		return GGI_EFATAL;
	}

	/* clear all frames */
	clear_fbmem(priv->fb_ptr, (unsigned)priv->fb_size);

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
		DPRINT_MODE("fbdev: RGB %d:%d:%d offsets %d:%d:%d\n",
			priv->var.red.length, priv->var.green.length,
			priv->var.blue.length, priv->var.red.offset,
			priv->var.green.offset, priv->var.blue.offset);

		LIBGGI_PIXFMT(vis)->red_mask =
		((1 << priv->var.red.length) - 1) << priv->var.red.offset;
			
		LIBGGI_PIXFMT(vis)->green_mask =
		((1 << priv->var.green.length) - 1) << priv->var.green.offset;
			
		LIBGGI_PIXFMT(vis)->blue_mask =
		((1 << priv->var.blue.length) - 1) << priv->var.blue.offset;
		break;

	case GT_TEXT:
		/* Assumes VGA text */
		LIBGGI_PIXFMT(vis)->texture_mask = 0x00ff;
		LIBGGI_PIXFMT(vis)->fg_mask = 0x0f00;
		LIBGGI_PIXFMT(vis)->bg_mask = 0xf000;
		break;
	}
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* Set up DirectBuffers */
	for (i=0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf;

		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

		buf = LIBGGI_APPBUFS(vis)[i];

		buf->frame = i;
		buf->type  = GGI_DB_NORMAL;
		buf->read  = (uint8_t *) priv->fb_ptr + i * priv->frame_size;
		buf->write = buf->read;

		if (priv->fix.type == FB_TYPE_PLANES) {
			buf->layout = blPixelPlanarBuffer;

			buf->buffer.plan.next_line =
				xres_in_bytes /	GT_SIZE(gt);
			buf->buffer.plan.next_plane = 
				buf->buffer.plan.next_line * LIBGGI_VIRTY(vis);
			buf->buffer.plan.pixelformat = LIBGGI_PIXFMT(vis);

		} else if (priv->fix.type == FB_TYPE_INTERLEAVED_PLANES) {
			buf->layout = blPixelPlanarBuffer;

			buf->buffer.plan.next_line =
				xres_in_bytes / GT_SIZE(gt);
			buf->buffer.plan.next_plane = priv->fix.type_aux;
			buf->buffer.plan.pixelformat = LIBGGI_PIXFMT(vis);

		} else {
			buf->type  |= GGI_DB_SIMPLE_PLB;
			buf->layout = blPixelLinearBuffer;

			buf->buffer.plb.stride = xres_in_bytes;
			buf->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);
		}
	}

	return 0;
}


static int do_setmode(ggi_visual *vis, struct fb_var_screeninfo *var)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	char libname[GGI_MAX_APILEN], libargs[GGI_MAX_APILEN];
	int err, id;

	err = do_change_mode(vis, var);
	if (err) {
		return err;
	}

	err = do_mmap(vis); 
	if (err) {
		return err;
	}

	_ggiZapMode(vis, 0);
	priv->have_accel = 0;
	priv->flush = NULL;
	priv->idleaccel = NULL;
	vis->needidleaccel = 1; /* Temp hack until we work out the */
	vis->accelactive = 0;   /* new changed() traversal for renderers */

	for (id=1; GGI_fbdev_getapi(vis, id, libname, libargs) == 0; id++) {
		if (_ggiOpenDL(vis, libname, libargs, NULL)) {
			DPRINT_LIBS("display-fbdev: Error opening the "
				       "%s (%s) library.\n", libname, libargs);
			if (id < 4) {
				fprintf(stderr, 
					"A needed sublib was not found.\n");
				return GGI_EFATAL;
			}
			continue;
		}
		else if (id == 5) {
			priv->have_accel = 1;
		}

		DPRINT_LIBS("Success in loading %s (%s)\n",
			       libname, libargs);
	}
	if (!priv->have_accel) LIBGGI_GC(vis) = priv->normalgc;
	vis->accelactive = 0;

	GGI_fbdev_color_setup(vis);
	if (vis->opcolor->setpalvec) ggiSetColorfulPalette(vis);
	if (vis->opcolor->setgammamap) ggiSetGamma(vis, 1.0, 1.0, 1.0);

	vis->opdraw->setorigin = GGI_fbdev_setorigin;
	vis->opdraw->setdisplayframe = GGI_fbdev_setdisplayframe;

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	DPRINT_MODE("display-fbdev: do_setmode SUCCESS\n");

	return 0;
}


int GGI_fbdev_setmode(ggi_visual *vis, ggi_mode *mode)
{
	struct fb_var_screeninfo var;
	struct ggi_fbdev_timing *timing;
	int err;
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);

	err = _GGI_fbdev_do_checkmode( vis, mode, &timing );
        if (err != 0) return err;

	/* Construct var from timing and mode and priv->orig_var */
	var = priv->orig_var;
	var.nonstd = 0;
	var.xoffset = var.yoffset = 0;
	ggimode2var(vis, mode, &var);
	timing2var(timing, &var, mode->frames );

	DPRINT_MODE("display-fbdev: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	/* Now actually set the mode */
	err = do_setmode(vis, &var);
	if (err != 0) return err;

	DPRINT_MODE("display-fbdev: setmode success.\n");

	return 0;
}


int GGI_fbdev_mode_reset(ggi_visual *vis)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);

	if (priv->fb_ptr != NULL) {
		_GGI_free_dbs(vis);
		/* Clear framebuffer */
		clear_fbmem(priv->fb_ptr, (unsigned)priv->mmap_size);
		munmap(priv->fb_ptr, (unsigned)priv->mmap_size);
	}
	fbdev_doioctl(vis, FBIOPUT_VSCREENINFO, &priv->orig_var);
	if (priv->fix.xpanstep != 0 || priv->fix.ypanstep != 0) {
		fbdev_doioctl(vis, FBIOPAN_DISPLAY, &priv->orig_var);
	}

	return 0;
}

/* Convert the fbdev specific ggi_fbdev_timing structure
 * into a generic ggi_mode structure suitable for 
 * passing as suggested mode to _GGI_generic_mode_update()
 */
static
void _GGI_fbdev_checkmode_adapt( ggi_mode * m, 
				struct ggi_fbdev_timing * timing,
				ggi_fbdev_priv * priv ) 
{
	m->visible.x = timing->xres;
	m->visible.y = timing->yres;
	m->virt.x = timing->xres_virtual;
	m->virt.y = timing->yres_virtual;

	/* In the future we may want to support smaller than fullscreen
	 * visuals to emulate unsupported modes.  If so, we'll need to
	 * handle size in the adjust function since it depends on the 
	 * requested resolution... Something like this:
	_ggi_physz_figure_size(sug, priv->physzflags, 
				     &(priv->physz),
				(signed)(DPI(width, x)),
				(signed)(DPI(height,y)), 
				sug->visible.x, sug->visible.y);
	*/
	/* For now, let's just use the framebuffer's values for the
	 * size of the whole screen...
	 * Note: the cast is neccessary because orig_var.width has an
	 * unsigned type! */
	m->size.x = ((signed)priv->orig_var.width <= 0 ) 
		? GGI_AUTO : priv->orig_var.width;
	m->size.y = ((signed)priv->orig_var.height <= 0 ) 
		?  GGI_AUTO : priv->orig_var.height;

	/* handled by the adjust function: 
	 * frames, graphtype, dpp 
	 */
}

/* Adjust suggested mode sug to match against
 * requested mode req for generic modecheck. 
 */
static
void _GGI_fbdev_checkmode_adjust( ggi_mode *req,
				  ggi_mode *sug,
				  ggi_fbdev_priv *priv )
{

	ggi_graphtype gt = req->graphtype;
	int xdpp, ydpp;
	int maxframes;

	/* graphtype... */
	/* Handle GGI_AUTO first */
	if (gt == GT_AUTO) {
#ifdef FB_TYPE_TEXT
		if (priv->orig_fix.type == FB_TYPE_TEXT) {
			GT_SETSCHEME(gt, GT_TEXT);
		} else
#endif
		switch (priv->orig_fix.visual) {

		case FB_VISUAL_MONO01:
		case FB_VISUAL_MONO10:
			GT_SETSCHEME(gt, GT_GREYSCALE);
			break;

		case FB_VISUAL_PSEUDOCOLOR:
		case FB_VISUAL_STATIC_PSEUDOCOLOR:
			GT_SETSCHEME(gt, priv->orig_var.grayscale ? 
				GT_GREYSCALE : GT_PALETTE);
			break;

		case FB_VISUAL_TRUECOLOR:
		case FB_VISUAL_DIRECTCOLOR:
			GT_SETSCHEME(gt, GT_TRUECOLOR);
			break;

		default:
			fprintf(stderr, "display-fbdev: WARNING: unknown "
				"visual (0x%02x) of framebuffer.\n",
				priv->orig_fix.visual);
			break;
		}
	}

	if (GT_DEPTH(gt) == GT_AUTO) {
		if ((GT_SCHEME(gt) == GT_TRUECOLOR) &&
		    (priv->orig_fix.visual == FB_VISUAL_DIRECTCOLOR ||
		     priv->orig_fix.visual == FB_VISUAL_TRUECOLOR)) {

			GT_SETDEPTH(gt, priv->orig_var.red.length   +
					priv->orig_var.green.length +
					priv->orig_var.blue.length);
		} else {
			GT_SETDEPTH(gt, priv->orig_var.bits_per_pixel);
		}
	}

	sug->graphtype = _GGIhandle_gtauto(gt);

	/* dpp... */
	xdpp = (GT_SCHEME(sug->graphtype) == GT_TEXT) ? FB_KLUDGE_FONTX : 1;
	ydpp = (GT_SCHEME(sug->graphtype) == GT_TEXT) ? FB_KLUDGE_FONTY : 1;

	sug->dpp.x = xdpp;
	sug->dpp.y = ydpp;

	/* frames... */
	sug->frames = req->frames;
	if (sug->frames == GGI_AUTO) {
		sug->frames = 1;
	} else if (priv->orig_fix.ypanstep == 0 && sug->frames > 1) {
		DPRINT_MODE("display-fbdev: "
			    "%d frames but no vertical panning\n", sug->frames);
		sug->frames = 1;
	}

	maxframes = priv->orig_fix.smem_len /
		(sug->virt.x * sug->virt.y *
		 GT_ByPP(sug->graphtype));
	if (sug->frames > maxframes) {
		sug->frames = maxframes;
	}


}


/* Iterate over the timings list obtained from the fb.modes 
 * file and search for the best matching mode. 
 */
static 
int _GGI_fbdev_do_checkmode(ggi_visual *vis, ggi_mode *mode,
                            struct ggi_fbdev_timing **timing_)
{
	int err = GGI_OK;
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	ggi_checkmode_t * cm;

	struct ggi_fbdev_timing *timing; /* iteration variable */

	/* Keep track of the previous node while we iterate in order to 
	 * delete bad modes from our list. */
	struct ggi_fbdev_timing *prev = NULL;
	struct ggi_fbdev_timing *saved_timing;


	cm = _GGI_generic_checkmode_create();
	

	DPRINT_MODE("display-fbdev: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);


	/* This puts the contents of mode into cm->req */
	_GGI_generic_checkmode_init( cm, mode );

	/* The first timing will be saved in case all nodes are deleted.
	 * We will use this as a fallback mode suggestion since it was 
	 * ostensibly obtained by querying the framebuffer for the current 
	 * mode. */
	saved_timing = priv->timings;

	timing = priv->timings;
	while( timing )  {
		struct fb_var_screeninfo var;

		/* Convert timing structure to a suggested mode */
		_GGI_fbdev_checkmode_adapt( mode, timing, priv );

		/* Use the requested mode to tailor the suggestion */
		_GGI_fbdev_checkmode_adjust( &cm->req, mode, priv );


		/* construct structure for passing to ioctl */
		var =  priv->orig_var;
		var.nonstd = 0;
		var.xoffset = var.yoffset = 0;
		ggimode2var( vis, mode, &var );
		timing2var( timing, &var, mode->frames );


		/* Check mode with framebuffer */
		var.activate = FB_ACTIVATE_TEST; /* just check */
		if (fbdev_doioctl(vis, FBIOPUT_VSCREENINFO, &var) >= 0) {
			/* The framebuffer seems to like it, so suggest 
			 * it as a valid mode to the checkmode API. */
			var2ggimode(vis, &var, mode, mode->frames);
			_GGI_generic_checkmode_update( cm, mode, (intptr_t)timing );
		}
		else {
			/* The framebuffer didn't like this mode, so
			 * let's delete it from the timings list... */
			struct ggi_fbdev_timing *next;
			next = timing->next;
			if( timing == priv->timings ) 
				priv->timings = next;
			else
				prev->next = next;
			/* if it's the saved_timing, it will be freed
			 * later after we are certain we haven't freed
			 * the entire list... */
			if( timing != saved_timing )
				free( timing );
			timing = next;
			continue;
		}
		prev = timing;
		timing = timing->next;
	}

	if( priv->timings == NULL )
	{
		/* No good modes.. perhaps the docs for ggiCheckMode should
		 * specify a return value for when this seems to be the case.
		 * Let's suggest the saved_timing... */
		DPRINT_MODE( "Error! FB_ACTIVATE_TEST failed for all modes\n" );

		_GGI_fbdev_checkmode_adapt( mode, saved_timing, priv );
		_GGI_fbdev_checkmode_adjust( &cm->req, mode, priv );
		_GGI_generic_checkmode_update( cm, mode, 
					       (intptr_t)saved_timing );

		/* ...and restore it to the list. */
		saved_timing->next = NULL;
		priv->timings = saved_timing;

		/* QUESTION: If the saved_timing mode just happens to
		 * match the reqested mode, _GGI_generic_checkmode_finish
		 * will return success.  Is this the proper behavior? */
	}
	else if( saved_timing != priv->timings )
		free( saved_timing );

	err = _GGI_generic_checkmode_finish( cm, mode, (intptr_t *)timing_);

	_GGI_generic_checkmode_destroy( cm );
	return err;
}

int GGI_fbdev_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	return _GGI_fbdev_do_checkmode( vis, mode, NULL );
}


int GGI_fbdev_getmode(ggi_visual *vis, ggi_mode *mode)
{
	DPRINT_MODE("display-fbdev: getmode\n");
	
	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}


int GGI_fbdev_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}
