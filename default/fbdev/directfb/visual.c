/* $Id: visual.c,v 1.15 2004/02/23 14:24:41 pekberg Exp $
******************************************************************************

   LibGGI - fbdev directfb acceleration

   Copyright (C) 2001 Brian S. Julin    [bri@calyx.com]

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

#define _FBDEV_DIRECTFB_BOGUS_GLOBALS
#include "ggidirectfb.h"
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <dlfcn.h>

/* The default LibGGI font */
#include <ggi/internal/font/8x8>

static int directfb_acquire(ggi_resource *res, uint32 actype)
{
	ggi_visual *vis;

	if (actype & ~(GGI_ACTYPE_READ | GGI_ACTYPE_WRITE)) {
		return GGI_EARGINVAL;
	}

	res->count++;
	res->curactype |= actype;
	if (res->count > 1) return 0;

	vis = res->priv;
	LIBGGIIdleAccel(vis);

	return 0;
}

static int directfb_release(ggi_resource *res)
{
	if (res->count < 1) return GGI_ENOTALLOC;

	res->count--;
	if (res->count == 0) {
		res->curactype = 0;
	}

	return 0;
}

static int
directfb_idleaccel(ggi_visual *vis)
{

	GraphicsDevice *device = &(DIRECTFB_PRIV(vis)->device);

	device->funcs.EngineSync(device->driver_data, device->device_data);

	vis->accelactive = 0;

	return 0;
}


static int do_cleanup(ggi_visual *vis)
{
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct directfb_priv *priv = NULL;
	int i;

	GGIDPRINT_MISC("fbdev-directfb: Starting cleanup\n");

	if (fbdevpriv != NULL) {
		priv = DIRECTFB_PRIV(vis);
	}

	/* We may be called more than once due to the LibGG cleanup stuff */
	if (priv == NULL) return 0;
	if (priv->driver.funcs != NULL) {
		if (priv->driver.funcs->CloseDevice != NULL)
		  	priv->driver.funcs->CloseDevice(&(priv->device),
							priv->device.driver_data,
							priv->device.device_data);
		if (priv->driver.funcs->CloseDriver != NULL) 
		  	priv->driver.funcs->CloseDriver(&(priv->device),
							priv->device.driver_data
							);
	}
	/* Free DB resource structures */
	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i]->resource) {
			free(LIBGGI_APPBUFS(vis)[i]->resource);
			LIBGGI_APPBUFS(vis)[i]->resource = NULL;
		}
	}

	if (priv->device.device_data) free(priv->device.device_data);
	if (priv->device.driver_data) free(priv->device.driver_data);
	free(priv);
	DIRECTFB_PRIV(vis) = NULL;

	ggUnregisterCleanup((ggcleanup_func *)do_cleanup, vis);

	return 0;
}
	

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
	DFBResult res;
	ggi_fbdev_priv *fbdevpriv = FBDEV_PRIV(vis);
	struct directfb_priv *priv;
	unsigned long usedmemend;
	int fontlen;
	int fd = LIBGGI_FD(vis);
	int i;
	DIR *dir;
	struct dirent *entry;
	char *driver_dir = DIRECTFB_DRIVER_DIR;
	int null_int = 0;
	void *handle = NULL;

	DFBConfig *dfb_config;
	FBDev	*dfb_fbdev;
	GraphicsDevice *dfb_device;
	GraphicsDriver *dfb_driver;

	GGIDPRINT("GGIopen for DirectFB started!\n");

	priv = calloc(sizeof(struct directfb_priv), 1);
	if (priv == NULL) return GGI_ENOMEM;

	/* Get the global symbols that DirectFB drivers need
	 * by loading our helper lib as a global symbol source.
	 */
	if(_ggiAddDL(vis, "helper-fbdev-directfb-global", 
		NULL, &(priv->globals), GGI_DLTYPE_GLOBAL)) {
		free(priv);
		return GGI_ENOFUNC;
	}

	/* Link the global dfb_config (and our local mask) to point to our 
	 * counterfeit DFBConfig 
	 */
	*(priv->globals.dfb_config_ptr) = dfb_config = &(priv->dfbconfig);
	/* Translate GGI options to dfb_config options for drivers. */
	dfb_config->layer_bg_mode = DLBM_COLOR;
	/* TODO: dfb_config->matrox_sgram = 1;  
	   Potentially damaging, make into option */
	if (!_ggiDebugState) {
		dfb_config->quiet = 1;
#if 0
		/* no_debug no longer exists in the structure */
		dfb_config->no_debug = 1;
#endif
	}

	/* Link the global dfb_fbdev (and our local mask) to point to our 
	 * counterfeit FBDev and link that to our counterfeit FBDevShared
	 * and link that to our counterfeit VideoMode.
	 */
	dfb_fbdev = *(priv->globals.dfb_fbdev_ptr) = &(priv->fbdev);
	dfb_fbdev->fd = fd;
	dfb_fbdev->shared = &(priv->fbdevshared);
	dfb_fbdev->shared->current_mode = dfb_fbdev->shared->modes = 
		&(priv->videomode);
	memcpy(&(priv->fbdevshared.current_var), &(fbdevpriv->var),
	       sizeof(struct fb_var_screeninfo));
	memcpy(&(priv->fbdevshared.orig_var), &(fbdevpriv->orig_var),
	       sizeof(struct fb_var_screeninfo)); /* Not used, but... */
	/* Skipping orig_cmap */

	priv->videomode.xres		= fbdevpriv->var.xres; /* Virtual ? */
        priv->videomode.yres		= fbdevpriv->var.yres; /* Virtual ? */
	priv->videomode.bpp		= fbdevpriv->var.bits_per_pixel;
        priv->videomode.pixclock	= fbdevpriv->var.pixclock;
        priv->videomode.left_margin	= fbdevpriv->var.left_margin;
        priv->videomode.right_margin	= fbdevpriv->var.right_margin;
        priv->videomode.upper_margin	= fbdevpriv->var.upper_margin;
        priv->videomode.lower_margin	= fbdevpriv->var.lower_margin;
        priv->videomode.hsync_len	= fbdevpriv->var.hsync_len;
        priv->videomode.vsync_len	= fbdevpriv->var.vsync_len;
	priv->videomode.hsync_high	= 
		1 && (fbdevpriv->var.sync & FB_SYNC_HOR_HIGH_ACT);
	priv->videomode.vsync_high	= 
		1 && (fbdevpriv->var.sync & FB_SYNC_VERT_HIGH_ACT);
	priv->videomode.csync_high	= 
		1 && (fbdevpriv->var.sync & FB_SYNC_COMP_HIGH_ACT);
	priv->videomode.sync_on_green	= 
		1 && (fbdevpriv->var.sync & FB_SYNC_ON_GREEN);
	priv->videomode.external_sync	= 
		1 && (fbdevpriv->var.sync & FB_SYNC_EXT);
        priv->videomode.laced		= 
		1 && (fbdevpriv->var.vmode & FB_VMODE_INTERLACED);
        priv->videomode.doubled		= 
		1 && (fbdevpriv->var.vmode & FB_VMODE_DOUBLE);

	switch (GT_DEPTH(LIBGGI_GT(vis))) {
		/* TODO: map 8-bit truecolor */
	case 15:
		priv->coresurface.format= DSPF_RGB15;
		break;
	case 16:
		priv->coresurface.format= DSPF_RGB16;
		break;
	case 24:
		priv->coresurface.format= DSPF_RGB24;
		break;
	case 32:
		priv->coresurface.format= DSPF_RGB32;
		break;
	default:
		/* priv->videomode.format = DSPF_UNKNOWN; */
		GGIDPRINT("DirectFB: Unknown Core Surface Format\n");
		do_cleanup(vis);
		return GGI_ENOFUNC;
	};

	/* Perform some more DirectFB internal structure nesting */
	dfb_device = &(priv->device);
	dfb_device->shared = &(priv->deviceshared);
	dfb_device->shared->state = &(priv->cardstate);
	dfb_driver = dfb_device->driver = &(priv->driver);
	priv->cardstate.destination = &(priv->coresurface);
	priv->cardstate.source = &(priv->coresurface);
	
	priv->coresurface.width = LIBGGI_VIRTX(vis);
	priv->coresurface.height = LIBGGI_VIRTY(vis);
	priv->coresurface.back_buffer = priv->coresurface.front_buffer = 
	  &(priv->corebuffer);
	priv->coresurface.caps &= ~DSCAPS_FLIPPING;

	priv->corebuffer.policy = CSP_VIDEOONLY;
	priv->corebuffer.video.health = CSH_STORED;
	priv->corebuffer.video.pitch = 
	  LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_MODE(vis)->graphtype) / 8;
	priv->corebuffer.video.offset = 0;
	priv->corebuffer.surface = &(priv->coresurface);

	memcpy(&(priv->deviceshared.fix), &(fbdevpriv->fix),
	       sizeof(struct fb_fix_screeninfo));

	dfb_device->framebuffer.base = fbdevpriv->fb_ptr;
	dfb_device->framebuffer.length = fbdevpriv->fb_size; /* mmap_size ?? */

	priv->oldbg = !LIBGGI_GC(vis)->bg_color;
	priv->oldfg = !LIBGGI_GC(vis)->fg_color;
 
	/* Load the DirectFB driver module. */
	dir = opendir(driver_dir);
	if (!dir) {
		GGIDPRINT( "Could not open DirectFB driver directory `%s'!\n", 
			   driver_dir );
		do_cleanup(vis);
		return GGI_ENOFUNC;
	}

	/* sprintf/strlen is safe here because driver_dir is compiled in, 
	 * and points to a root-owned/controlled system directory 
	 * which should have known good filenames even if readdir untrusted.
	 * (We have bigger problems if either assumption isn't true :-)
	 */
  
	while ((entry = readdir(dir)) != NULL) {
		int   entry_len = strlen(entry->d_name);
		int   buf_len   = strlen(driver_dir) + entry_len + 2;
		char buf[buf_len]; /* I feel dirty. Is this legal? */

		if (entry_len < 4 ||
		    entry->d_name[strlen(entry->d_name)-1] != 'o' ||
		    entry->d_name[strlen(entry->d_name)-2] != 's')
			continue;

		sprintf(buf, "%s/%s", driver_dir, entry->d_name);

		GGIDPRINT("Opening '%s'.\n", buf);

		handle = dlopen(buf, RTLD_LAZY);
		if (handle) {
			void (*dfbfunc)(void);
			char *ptr = strstr(entry->d_name, "directfb_");
			if (ptr == NULL) {
				GGIDPRINT("Bad .so filename '%s'.\n", buf);
				dlclose(handle);
				continue;
			}
			ptr[strlen(ptr) - 3] = '\0';
			dfbfunc = dlsym(handle, ptr);
			if (!dfbfunc) {
				GGIDPRINT("No main dlsym in '%s'.\n", buf);
				dlclose(handle);
				continue;
			}
			dfbfunc(); /* Hackackahkkackitiehack... */
			dfb_driver->funcs = 
			  (GraphicsDriverFuncs *)dfb_config->mouse_protocol;
			dfb_config->mouse_protocol = NULL;
			if (!dfb_driver->funcs) {
				GGIDPRINT("No driver_funcs '%s'.\n", buf);
				dlclose(handle);
				continue;
			}
			GGIDPRINT("Trying driver at "
				  "%p...\n", dfb_driver->funcs);
			if (dfb_driver->funcs->Probe) {
				if (dfb_driver->funcs->Probe(dfb_device)) {
					/* TODO: ensure all functions exist. */
				  	GGIDPRINT("driver_probe success for "
						  "`%s'\n", buf);
					break;
				}
				else {
					GGIDPRINT("driver_probe failed for "
						  "`%s'!\n", buf );
				}
			} else {
				GGIDPRINT( "No driver_probe in `%s'!\n", buf );
			}
		} else {
			GGIDPRINT("dlopen failed for `%s'!\n", buf );
			continue;
		}
		dlclose(handle);
	}	/* while */
	closedir( dir );
	if (entry == NULL) {
		/* Prevent deinit functions from being called */
		dfb_driver->funcs = NULL;
		do_cleanup(vis);
		return GGI_ENOFUNC;
	}

	/* Skipping ABI version check until DirectFB gets out of Beta. */

	/* Allow the driver to fill in information about itself */
	dfb_driver->funcs->GetDriverInfo(dfb_device, 
					 &(dfb_device->shared->driver_info));
	if (dfb_device->shared->driver_info.driver_data_size) { 
		dfb_device->driver_data = 
			calloc(dfb_device->shared->driver_info.driver_data_size, 1);
		if (!dfb_device->driver_data) {
			/* Prevent deinit functions from being called */
			dfb_driver->funcs = NULL;
			do_cleanup(vis);
			return GGI_ENOMEM;
		}
	}
	if (dfb_device->shared->driver_info.driver_data_size) {
		dfb_device->shared->device_data = dfb_device->device_data = 
			calloc(dfb_device->shared->driver_info.device_data_size, 1);
		if (!dfb_device->device_data) {
			/* Prevent deinit functions from being called */
			dfb_driver->funcs = NULL;
			do_cleanup(vis);
			return GGI_ENOMEM;
		}
	}

	if (dfb_driver->funcs->InitDriver(dfb_device, &(dfb_device->funcs), 
					  dfb_device->driver_data)) {
		GGIDPRINT("DirectFB init_driver function failed\n");
		/* Prevent deinit functions from being called */
		dfb_driver->funcs = NULL; 
		do_cleanup(vis);
		return GGI_ENOMEM;
	}
	
	if (dfb_driver->funcs->InitDevice(dfb_device, 
					  &(dfb_device->shared->device_info), 
					  dfb_device->driver_data, 
					  dfb_device->device_data)) {
		GGIDPRINT("DirectFB init_device function failed\n");
		/* Prevent close_device from being called. */
		dfb_driver->funcs->CloseDriver(dfb_device, 
					      dfb_device->driver_data);
		dfb_driver->funcs = NULL;
		do_cleanup(vis);
		return GGI_ENOMEM;
	}


	if (dfb_device->funcs.AfterSetVar) 
		dfb_device->funcs.AfterSetVar(dfb_device->driver_data,
					      dfb_device->device_data);

	/* Set up DirectBuffers */
	for (i=0; i < LIBGGI_MODE(vis)->frames; i++) {
		ggi_directbuffer *buf = LIBGGI_APPBUFS(vis)[i];
		ggi_resource *res;
	  
		res = malloc(sizeof(ggi_resource));
		if (res == NULL) goto abort;
		
		buf->resource = res;
		buf->resource->acquire = directfb_acquire;
		buf->resource->release = directfb_release;
		buf->resource->self = buf;
		buf->resource->priv = vis;
		buf->resource->count = 0;
		buf->resource->curactype = 0;
	}
	
	/* Initialize function pointers */
	vis->needidleaccel = 1;
	fbdevpriv->idleaccel = directfb_idleaccel;

	vis->opdraw->getcharsize = GGI_directfb_getcharsize;
	if (dfb_device->shared->device_info.caps.accel & DFXL_FILLRECTANGLE) {
		vis->opdraw->drawhline   = GGI_directfb_drawhline;
		vis->opdraw->drawvline   = GGI_directfb_drawvline;
		vis->opdraw->drawbox     = GGI_directfb_drawbox;
		vis->opdraw->fillscreen  = GGI_directfb_fillscreen;
	};
	if (dfb_device->shared->device_info.caps.accel & DFXL_FILLRECTANGLE) {
		vis->opdraw->drawline    = GGI_directfb_drawline;
	}
	if (dfb_device->shared->device_info.caps.accel & DFXL_BLIT) {
		vis->opdraw->copybox     = GGI_directfb_copybox;
#if 0
		/* These will require kernel-served/mlocked buffers. */
		vis->opdraw->puthline   = GGI_directfb_puthline;
		vis->opdraw->putvline   = GGI_directfb_putvline;
		vis->opdraw->putbox     = GGI_directfb_putbox;
		vis->opdraw->crossblit   = GGI_directfb_crossblit;
#endif
	}

	DIRECTFB_PRIV(vis) = priv;
  
	/* Register cleanup handler */
	ggRegisterCleanup((ggcleanup_func *)do_cleanup, vis);
  
	*dlret = GGI_DL_OPDRAW;
	
	return 0;
	
 abort:
	dlclose(handle);
	do_cleanup(vis);
	return GGI_ENOFUNC;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


EXPORTFUNC
int GGIdl_directfb(int func, void **funcptr);

int GGIdl_directfb(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
