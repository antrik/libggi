/* $Id: mode.c,v 1.20 2004/11/13 15:56:20 cegger Exp $
******************************************************************************

   Display-FBDEV

   Copyright (C) 1998 Andrew Apted		[andrew@ggi-project.org]
   Copyright (C) 1999-2000 Marcus Sundberg	[marcus@ggi-project.org]

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

extern void GGI_fbdev_color_reset(ggi_visual *vis);
extern void GGI_fbdev_color_setup(ggi_visual *vis);
int GGI_fbdev_mode_reset(ggi_visual *vis);

static int
do_checkmode(ggi_visual *vis, ggi_mode *mode, struct fb_var_screeninfo *var);

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
var2ggimode(ggi_visual *vis, struct fb_var_screeninfo *var, ggi_mode *mode,
	    int frames)
{
	mode->visible.x = var->xres;
	mode->visible.y = var->yres;
	mode->virt.x = var->xres_virtual;
	mode->virt.y = var->yres_virtual;
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

static int
get_timing(ggi_visual *vis, struct fb_var_screeninfo *var, int frames)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	struct ggi_fbdev_timing *timing, *besttiming;
	int ret = 1;

	/* Scan for the wanted mode.
	 */
	besttiming = NULL;
	timing = priv->timings;
	while (timing) {
		if ((timing->xres == var->xres || var->xres == 0) &&
		    (timing->yres == var->yres || var->yres == 0)) {
			if ((timing->bits_per_pixel == var->bits_per_pixel ||
			     var->bits_per_pixel == FB_BPP_AUTO) &&
			    (timing->xres_virtual == var->xres_virtual ||
			     var->xres_virtual == 0)) {
				/* yres_virtual doesn't matter. */
				GGIDPRINT_MODE("display-fbdev: found exact match.\n");
				ret = 0;
				goto found_mode;
			}
			if (timing->bits_per_pixel == var->bits_per_pixel ||
			    var->bits_per_pixel == FB_BPP_AUTO) {
				GGIDPRINT_MODE("display-fbdev: found match with xres_virtual = %d.\n",
					       timing->xres_virtual);
				ret = 0;
				besttiming = timing;
			} else if (besttiming == NULL || ret == 1) {
				GGIDPRINT_MODE("display-fbdev: found first match (exact res) %dx%d.\n",
					       timing->xres, timing->yres);
				ret = 0;
				besttiming = timing;
			}
		} else if ((timing->xres >= var->xres || var->xres == 0) &&
			   (timing->yres >= var->yres || var->yres == 0)) {
			if (besttiming == NULL) {
				GGIDPRINT_MODE("display-fbdev: found first match (bigger res) %dx%d.\n",
					       timing->xres, timing->yres);
				ret = 1;
				besttiming = timing;
			} else if (timing->xres <= besttiming->xres &&
				   timing->yres <= besttiming->yres &&
				   ret == 1) {
				GGIDPRINT_MODE("display-fbdev: found better match (bigger res) %dx%d.\n",
					       timing->xres, timing->yres);
				ret = 1;
				besttiming = timing;
			}
		}
		timing = timing->next;
	}
	if (besttiming) {
		timing = besttiming;
		GGIDPRINT_MODE("display-fbdev: Using best match %dx%d.\n",
			       timing->xres, timing->yres);
		goto found_mode;
	}

	return GGI_ENOTFOUND;

  found_mode:
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

	return ret;
}


static int do_change_mode(ggi_visual *vis, struct fb_var_screeninfo *var)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	ggi_mode *mode = LIBGGI_MODE(vis);
	ggi_graphtype gt = mode->graphtype;
	int err;

	/* If we already have set a mode, restore the old palette
	 * or gamma settings before setting a new one. 
	 */
	GGI_fbdev_color_reset(vis);

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
				GGIDPRINT_MODE("display-fbdev: GT_GREYSCALE mode failed\n");
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
				GGIDPRINT_MODE("display-fbdev: GT_PALETTE mode failed\n");
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
				GGIDPRINT_MODE("display-fbdev: GT_TRUECOLOR mode failed\n");
			}
			break;
		
#ifdef FB_TYPE_TEXT
		case GT_TEXT:
			if (priv->fix.type != FB_TYPE_TEXT)
#endif
				err = GGI_EFATAL;
	}
	
	if (err) {
		GGIDPRINT_MODE("display-fbdev: Santa passed us by :-(\n");
	} else {
		GGIDPRINT_MODE("display-fbdev: Change mode OK.\n");
	}

	if (! err && (priv->fix.ypanstep == 0) && (mode->frames > 1)) {
		mode->frames = 1;
		GGIDPRINT_MODE("display-fbdev: cannot do vertical panning "
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
	
	GGIDPRINT_MODE("display-fbdev: frame_size=0x%x fb_size=0x%x "
		    "mmap_size=0x%x\n", priv->frame_size,
		    priv->fb_size, priv->mmap_size);

	priv->fb_ptr = mmap(NULL, (unsigned)priv->mmap_size,
			    PROT_READ | PROT_WRITE, 
			    MAP_SHARED, LIBGGI_FD(vis), 0);

	GGIDPRINT_MODE("display-fbdev: FB_PTR=%p\n", priv->fb_ptr);

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
		GGIDPRINT_MODE("fbdev: RGB %d:%d:%d offsets %d:%d:%d\n",
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
		buf->read  = (uint8 *) priv->fb_ptr + i * priv->frame_size;
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
			GGIDPRINT_LIBS("display-fbdev: Error opening the "
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

		GGIDPRINT_LIBS("Success in loading %s (%s)\n",
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

	GGIDPRINT_MODE("display-fbdev: do_setmode SUCCESS\n");

	return 0;
}


int GGI_fbdev_setmode(ggi_visual *vis, ggi_mode *mode)
{
	struct fb_var_screeninfo var;
	int err;

	err = do_checkmode(vis, mode, &var);
        if (err != 0) return err;

	GGIDPRINT_MODE("display-fbdev: setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	memcpy(LIBGGI_MODE(vis), mode, sizeof(ggi_mode));

	/* Now actually set the mode */
	err = do_setmode(vis, &var);
	if (err != 0) return err;

	GGIDPRINT_MODE("display-fbdev: setmode success.\n");

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


static int
do_checkmode(ggi_visual *vis, ggi_mode *mode, struct fb_var_screeninfo *var)
{
	ggi_fbdev_priv *priv = FBDEV_PRIV(vis);
	ggi_graphtype gt = mode->graphtype;
	ggi_mode oldmode = *mode;
	int xdpp, ydpp;
	int maxframes;
	int err = 0, ret;

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

	mode->graphtype = _GGIhandle_gtauto(gt);

	xdpp = (GT_SCHEME(mode->graphtype) == GT_TEXT) ? FB_KLUDGE_FONTX : 1;
	ydpp = (GT_SCHEME(mode->graphtype) == GT_TEXT) ? FB_KLUDGE_FONTY : 1;
	if ((mode->dpp.x != xdpp && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != ydpp && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = xdpp;
	mode->dpp.y = ydpp;

	if (mode->frames == GGI_AUTO) {
		mode->frames = 1;
	} else if (priv->orig_fix.ypanstep == 0 && mode->frames > 1) {
		GGIDPRINT_MODE("display-fbdev: %d frames but no vertical panning\n", mode->frames);
		mode->frames = 1;
		err = -1;
	}

	*var = priv->orig_var;

	var->nonstd = 0;
	var->xoffset = var->yoffset = 0;

	ggimode2var(vis, mode, var);

	/* Try to get the timing from the standard database */
	ret = get_timing(vis, var, mode->frames);
	if (ret < 0) {
		GGIDPRINT_MODE("display-fbdev: unable to find timing for mode\n");
		if (priv->need_timings &&
		    (var->xres != priv->orig_var.xres ||
		     var->yres != priv->orig_var.yres)) {
			var2ggimode(vis, &priv->orig_var, mode, mode->frames);
			return GGI_ENOMATCH;
		}
	} else if (ret != 0) {
		err = -1;
	}

	/* Just check the mode */
	var->activate = FB_ACTIVATE_TEST;

	/* Check mode with framebuffer */
	if (fbdev_doioctl(vis, FBIOPUT_VSCREENINFO, var) < 0) {
		GGIDPRINT_MODE("display-fbdev: FB_ACTIVATE_TEST failed\n");
		err = -1;
	}

	var2ggimode(vis, var, mode, mode->frames);

	if ((oldmode.visible.x && mode->visible.x != oldmode.visible.x) ||
	    (oldmode.visible.y && mode->visible.y != oldmode.visible.y) ||
	    (oldmode.virt.x && mode->virt.x != oldmode.virt.x) ||
	    (oldmode.virt.y && mode->virt.y != oldmode.virt.y) ||
	    (GT_DEPTH(oldmode.graphtype) &&
	     GT_DEPTH(mode->graphtype) != GT_DEPTH(oldmode.graphtype)) ||
	    (GT_SIZE(oldmode.graphtype) &&
	     GT_SIZE(mode->graphtype) != GT_SIZE(oldmode.graphtype)) ||
	    (GT_SCHEME(oldmode.graphtype) &&
	     GT_SCHEME(mode->graphtype) != GT_SCHEME(oldmode.graphtype))) {
		GGIDPRINT_MODE("display-fbdev: checkmode error\n");
		err = -1;
	}

#define DPI(dim, d) \
	((priv->orig_var.dim <= 0) ? 0 : \
	 (mode->visible.d * mode->dpp.d * 254 / priv->orig_var.dim / 10))

	if (!err) {
		err = _ggi_physz_figure_size(mode, priv->physzflags, &(priv->physz),
					(signed)(DPI(width, x)),
					(signed)(DPI(height,y)), 
					mode->visible.x, mode->visible.y);
	}

#undef DPI

	maxframes = priv->orig_fix.smem_len /
		(mode->virt.x * mode->virt.y *
		 GT_ByPP(mode->graphtype));
	if (mode->frames > maxframes) {
		mode->frames = maxframes;
		err = -1;
	}

	GGIDPRINT_MODE("display-fbdev: checkmode returns %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	return err;
}


int GGI_fbdev_checkmode(ggi_visual *vis, ggi_mode *mode)
{
	struct fb_var_screeninfo var;

	GGIDPRINT_MODE("display-fbdev: checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	return do_checkmode(vis, mode, &var);
}


int GGI_fbdev_getmode(ggi_visual *vis, ggi_mode *mode)
{
	GGIDPRINT_MODE("display-fbdev: getmode\n");
	
	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}


int GGI_fbdev_setflags(ggi_visual *vis, ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}
