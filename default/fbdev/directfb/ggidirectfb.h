/* $Id: ggidirectfb.h,v 1.10 2006/03/22 20:29:10 cegger Exp $
******************************************************************************

   LibGGI - DirectFB chipset driver support.

   Copyright (C) 2001 Brian S. Julin   [bri@calyx.com]

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

#ifndef _GGIFBDEV_DIRECTFB_H
#define _GGIFBDEV_DIRECTFB_H

#include <unistd.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>

#define FUSION_FAKE

#define directfb_major_version fbdev_directfb_major_version_bogus
#define directfb_minor_version fbdev_directfb_minor_version_bogus
#define directfb_micro_version fbdev_directfb_micro_version_bogus
#define directfb_binary_age    fbdev_directfb_binary_age_bogus
#define directfb_interface_age fbdev_directfb_interface_age_bogus
#ifdef _FBDEV_DIRECTFB_BOGUS_GLOBALS
# define extern
# include <directfb.h>
# undef extern
#else
# include <directfb.h>
#endif
#undef directfb_major_version
#undef directfb_minor_version
#undef directfb_micro_version
#undef directfb_binary_age
#undef directfb_interface_age
#undef extern

#ifdef _FBDEV_DIRECTFB_GLOBALS
# define extern
# define dfb_config dfb_config = NULL
# include <directfb-internal/misc/conf.h>
# undef dfb_config
# undef extern
#else
# define dfb_config fbdev_directfb_dfb_config_bogus
# ifdef _FBDEV_DIRECTFB_BOGUS_GLOBALS
#  define extern
#  include <directfb-internal/misc/conf.h>
#  undef extern
# else
#  include <directfb-internal/misc/conf.h>
# endif
# undef dfb_config
#endif

#include <directfb-internal/core/coretypes.h>
#include <directfb-internal/core/gfxcard.h>
#include <directfb-internal/core/fusion/list.h>
#include <directfb-internal/core/fusion/fusion_types.h>


/* These structures are lurking in a .c file so we must provide. */

typedef struct {
  FusionLink            link;

  GraphicsDriverFuncs  *funcs;

  int                   abi_version;
} GraphicsDriver;

typedef struct {
  struct fb_fix_screeninfo fix;

  GraphicsDriverInfo    driver_info;
  GraphicsDeviceInfo    device_info;
  void                 *device_data;

  FusionSkirmish        lock;

  SurfaceManager       *surface_manager;

  /*
   * Points to the current state of the graphics card.
   */
  CardState            *state;
} GraphicsDeviceShared;

struct _GraphicsDevice {
  GraphicsDeviceShared *shared;

  GraphicsDriver       *driver;
  void                 *driver_data;
  void                 *device_data; /* copy of shared->device_data */

  GraphicsDeviceFuncs   funcs;

  /* framebuffer address and size */
  struct {
    unsigned int     length;
    void            *base;
  } framebuffer;
};

#ifdef _FBDEV_DIRECTFB_GLOBALS
# define extern
# define dfb_fbdev dfb_fbdev = NULL
# include <directfb-internal/core/fbdev/fbdev.h>
# undef dfb_fbdev
# undef extern
#else
# define dfb_fbdev fbdev_directfb_dfb_fbdev_bogus
# ifdef _FBDEV_DIRECTFB_BOGUS_GLOBALS
#  define extern
#  include <directfb-internal/core/fbdev/fbdev.h>
#  undef extern
# else
#  include <directfb-internal/core/fbdev/fbdev.h>
# endif
# undef dfb_fbdev
#endif

#include <directfb-internal/core/state.h>
#include <directfb-internal/core/surfaces.h>

#undef FUSION_FAKE

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

/* Size of the font */
#define FWIDTH	8
#define FHEIGHT	8

struct fbdev_directfb_global {
  DFBConfig	**dfb_config_ptr;
  FBDev		**dfb_fbdev_ptr;
};

struct directfb_priv {
  struct fbdev_directfb_global	globals;

  DFBConfig			dfbconfig;

  FBDev				fbdev;
  FBDevShared			fbdevshared;
  VideoMode			videomode;

  GraphicsDevice		device;
  GraphicsDriver		driver;
  GraphicsDeviceShared		deviceshared;
  CardState			cardstate;
  CoreSurface			coresurface;
  SurfaceBuffer			corebuffer;

  ggi_pixel	oldfg;
  ggi_pixel	oldbg;
  CoreSurface	*oldsource;

};

/* Update GC components if needed */
static inline void
directfb_gcupdate(struct ggi_visual *vis, /* Only used when unmappixel() needed. */
		  struct directfb_priv *priv,
		  ggi_mode *mode, ggi_gc *gc,
		  int virtx, int yadd, DFBAccelerationMask accel)

{
  CardState *cardstate;
  DFBConfig *dfb_config;
  int newfg, newbg, newclip, newsource;

  cardstate = &(priv->cardstate);
  dfb_config = &(priv->dfbconfig);

  newfg = (gc->fg_color != priv->oldfg)
    || (cardstate->drawingflags != DSDRAW_NOFX)
    || (cardstate->blittingflags != DSBLIT_NOFX);
  newbg = (gc->bg_color != priv->oldbg)
    || (dfb_config->layer_bg_mode != DLBM_COLOR);
  newclip =
    (gc->cliptl.x != cardstate->clip.x1) ||
    (gc->clipbr.x != cardstate->clip.x2) ||
    (gc->cliptl.y != cardstate->clip.y1) ||
    (gc->clipbr.y != cardstate->clip.y2);
  newsource = (cardstate->source != priv->oldsource);

  if (! (newfg || newbg || newclip || newsource)) return;

  if (newclip) {
    cardstate->modified |= SMF_CLIP;
    cardstate->clip.x1 = gc->cliptl.x;
    cardstate->clip.x2 = gc->clipbr.x;
    cardstate->clip.y1 = gc->cliptl.y;
    cardstate->clip.y2 = gc->clipbr.y;
  }

  if (newfg) {
    ggi_color newfgcolor;

    cardstate->modified |= SMF_COLOR;
    ggiUnmapPixel(vis, gc->fg_color, &newfgcolor);
    cardstate->color.a = 0; /* ?? */
    cardstate->color.r = (uint8_t)(newfgcolor.r >> 8);
    cardstate->color.g = (uint8_t)(newfgcolor.g >> 8);
    cardstate->color.b = (uint8_t)(newfgcolor.b >> 8);
    if (cardstate->drawingflags != DSDRAW_NOFX) {
      cardstate->drawingflags = DSDRAW_NOFX;
      cardstate->modified |= SMF_DRAWING_FLAGS;
    }
    if (cardstate->blittingflags != DSBLIT_NOFX) {
      cardstate->blittingflags = DSBLIT_NOFX;
      cardstate->modified |= SMF_BLITTING_FLAGS;
    }

  }

  if (newbg) {
    ggi_color newbgcolor;

    ggiUnmapPixel(vis, gc->bg_color, &newbgcolor);
    dfb_config->layer_bg_color.a = 0; /* ?? */
    dfb_config->layer_bg_color.r = (uint8_t)(newbgcolor.r >> 8);
    dfb_config->layer_bg_color.g = (uint8_t)(newbgcolor.g >> 8);
    dfb_config->layer_bg_color.b = (uint8_t)(newbgcolor.b >> 8);
    /* Force reread of the layer color.  Inefficient? */
    cardstate->modified |= SMF_DESTINATION;
    priv->oldbg = gc->bg_color;
    dfb_config->layer_bg_mode = DLBM_COLOR;
  }

  if (newsource) {
    cardstate->modified = SMF_SOURCE;
    priv->oldsource = cardstate->source;
  }

  priv->device.funcs.SetState(priv->device.driver_data,
			      priv->device.device_data,
			      &(priv->device.funcs),
			      &(priv->cardstate),
			      accel);
}

#define DIRECTFB_PRIV(vis) ((struct directfb_priv*)FBDEV_PRIV(vis)->accelpriv)

ggifunc_getcharsize	GGI_directfb_getcharsize;
ggifunc_putc		GGI_directfb_putc;
ggifunc_puts		GGI_directfb_puts;
ggifunc_putc		GGI_directfb_fastputc;
ggifunc_puts		GGI_directfb_fastputs;

ggifunc_drawhline	GGI_directfb_drawhline;
ggifunc_drawvline	GGI_directfb_drawvline;
ggifunc_drawline	GGI_directfb_drawline;
ggifunc_drawbox		GGI_directfb_drawbox;
ggifunc_copybox		GGI_directfb_copybox;
ggifunc_fillscreen	GGI_directfb_fillscreen;
ggifunc_crossblit	GGI_directfb_crossblit;

#endif /* _GGIFBDEV_DIRECTFB_H */
