/* $Id: visual.c,v 1.11 2002/01/27 09:18:03 cegger Exp $
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

	DIRECTFB_PRIV(vis)->gfxcard.EngineSync();
	
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

	if (priv->gfxdriver.DeInit != NULL) priv->gfxdriver.DeInit();

	/* Free DB resource structures */
	for (i = LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		if (LIBGGI_APPBUFS(vis)[i]->resource) {
			free(LIBGGI_APPBUFS(vis)[i]->resource);
			LIBGGI_APPBUFS(vis)[i]->resource = NULL;
		}
	}

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
  int pixbytes;
  int fd = LIBGGI_FD(vis);
  int i;
  GfxCard *dfb_card;
  DFBConfig *dfb_config;
  GfxDriver *driver;
  DIR *dir;
  struct dirent *entry;
  char *driver_dir = DIRECTFB_DRIVER_DIR;
  int null_int = 0;

  GGIDPRINT("GGIopen for DirectFB started!\n");

  pixbytes = GT_SIZE(LIBGGI_GT(vis)) / 8;
  
  priv = calloc(sizeof(struct directfb_priv), 1);
  if (priv == NULL) {
    return GGI_ENOMEM;
  }

  if(_ggiAddDL(vis, "helper-fbdev-directfb-global", 
	       NULL, &(priv->globals), GGI_DLTYPE_GLOBAL)) {
    free(priv);
    return GGI_ENOFUNC;
  }
  dfb_card = &(priv->gfxcard);
  *(priv->globals.dfb_card_ptr) = &(priv->gfxcard);
  driver = &(priv->gfxdriver);
  dfb_config = &(priv->dfbconfig);
  *(priv->globals.dfb_config_ptr) = &(priv->dfbconfig);
  dfb_config->layer_bg_mode = DLBM_COLOR;
  /* dfb_config->matrox_sgram = 1;  Potentially dangerous, make into option */
  if (!_ggiDebugState) {
    dfb_config->quiet = 1;
    dfb_config->no_debug = 1;
  }
  fprintf(stderr,"card = %p, config = %p\n", dfb_card, dfb_config);

  sprintf(dfb_card->info.driver_vendor, "convergence integrated media GmbH" );
  dfb_card->info.driver_version.major = 0;
  dfb_card->info.driver_version.major = 3;
  
  memcpy(&dfb_card->fix, &(fbdevpriv->orig_fix), sizeof(dfb_card->fix));
  dfb_card->framebuffer.length = dfb_card->fix.smem_len;
  dfb_card->framebuffer.base = fbdevpriv->fb_ptr;

  /* Load the DirectFB driver module.  Code as per DirectFB core/gfxcard.c */
  dir = opendir( driver_dir );
  if (!dir) {
    GGIDPRINT( "Could not open DirectFB driver directory `%s'!\n", 
	       driver_dir );
    do_cleanup(vis);
    return GGI_ENOFUNC;
  }

  while ( (entry = readdir(dir) ) != NULL ) {
    void *handle;
    char buf[4096];

    if (entry->d_name[strlen(entry->d_name)-1] != 'o' ||
	entry->d_name[strlen(entry->d_name)-2] != 's')
      continue;

#ifdef HAVE_SNPRINTF
    snprintf( buf, 4096, "%s/%s", driver_dir, entry->d_name );
#else
    sprintf( buf, "%s/%s", driver_dir, entry->d_name );
#endif
    GGIDPRINT("Opening '%s'.\n", buf);

    handle = dlopen( buf, RTLD_LAZY );
    if (handle) {
      driver->Probe  = dlsym( handle, "driver_probe"  );
      GGIDPRINT("Probing driver at %p...\n", driver->Probe);
      if (driver->Probe) {
	if ( driver->Probe( fd, dfb_card )) {
	  driver->Init       = dlsym( handle, "driver_init"   );
	  driver->InitLayers = dlsym( handle, "driver_init_layers" );
	  driver->DeInit     = dlsym( handle, "driver_deinit" );
	  if (driver->Init  &&  driver->DeInit) {
	    GGIDPRINT("DirectFB driver, Init:%p InitLayers:%p DeInit:%p\n",
		      driver->Init, driver->InitLayers, driver->DeInit);
	    break;
	  }
	  else {
	    GGIDPRINT( "DirectFB/core/gfxcards: "
		       "Probe succeeded but Init/DeInit "
		       "symbols not found in `%s'!\n", buf );
	    goto abort;
	  }
	}
      } else {
	
	GGIDPRINT( "DirectFB/core/gfxcards: "
		   "Could not link probe function of `%s'!\n",
		   buf );
	goto abort;
      }
      dlclose( handle );
    }
    else {
      GGIDPRINT("DirectFB/core/gfxcards: Unable to dlopen %s\n", dlerror() );
      goto abort;
    }
  }

  closedir( dir );
  dfb_card->info.driver = driver;

  if (priv->gfxdriver.Init(fd, dfb_card)) {
    GGIDPRINT("DirectFB driver init function failed\n");
    goto abort;
  }

  GGIDPRINT("DirectFB/GfxCard: %s %d.%d (%s)\n", dfb_card->info.driver_name,
	    dfb_card->info.driver_version.major, 
	    dfb_card->info.driver_version.minor,
	    dfb_card->info.driver_vendor);

  priv->corebuffer.policy = CSP_VIDEOONLY;
  priv->corebuffer.video.health = CSH_STORED;
  priv->corebuffer.video.pitch = 
    LIBGGI_VIRTX(vis) * GT_SIZE(LIBGGI_MODE(vis)->graphtype) / 8;
  priv->corebuffer.video.offset = 0;
  priv->corebuffer.surface = &(priv->coresurface);
  priv->dfbstate.destination = &(priv->coresurface);
  priv->dfbstate.source = &(priv->coresurface);
  switch (GT_DEPTH(LIBGGI_GT(vis))) {
  case 15:
    priv->coresurface.format = DSPF_RGB15;
    break;
  case 16:
    priv->coresurface.format = DSPF_RGB16;
    break;
  case 24:
    priv->coresurface.format = DSPF_RGB24;
    break;
  case 32:
    priv->coresurface.format = DSPF_RGB32;
    break;
  default:
    /* priv->coresurface.format = DSPF_UNKNOWN; */
    GGIDPRINT("DirectFB: Unknown Core Surface Format\n");
    goto abort;
  };
  priv->coresurface.width = LIBGGI_VIRTX(vis);
  priv->coresurface.height = LIBGGI_VIRTY(vis);
  priv->coresurface.back_buffer = priv->coresurface.front_buffer = 
    &(priv->corebuffer);
  priv->coresurface.caps &= ~DSCAPS_FLIPPING;
  if (dfb_card->AfterSetVar) dfb_card->AfterSetVar();

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
  if (dfb_card->caps.accel & DFXL_FILLRECTANGLE) {
    vis->opdraw->drawhline   = GGI_directfb_drawhline;
    vis->opdraw->drawvline   = GGI_directfb_drawvline;
    vis->opdraw->drawbox     = GGI_directfb_drawbox;
    vis->opdraw->fillscreen  = GGI_directfb_fillscreen;
  };
  if (dfb_card->caps.accel & DFXL_FILLRECTANGLE) {
    vis->opdraw->drawline    = GGI_directfb_drawline;
  }
  if (dfb_card->caps.accel & DFXL_BLIT) {
    vis->opdraw->copybox     = GGI_directfb_copybox;
#if 0
    /* These will follow.  First let's get the rest of the stuff working. */
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
  closedir(dir);
  do_cleanup(vis);
  return GGI_ENOFUNC;
}


static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	return do_cleanup(vis);
}


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
