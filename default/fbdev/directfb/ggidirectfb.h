/* $Id: ggidirectfb.h,v 1.3 2001/08/22 03:41:08 skids Exp $
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

#include <ggi/internal/ggi-dl.h>
#include <ggi/display/fbdev.h>

#include <directfb-internal/directfb_version.h>

#ifdef _FBDEV_DIRECTFB_GLOBALS
# define extern
# define directfb_major_version directfb_major_version = DIRECTFB_MAJOR_VERSION
# define directfb_minor_version directfb_minor_version = DIRECTFB_MINOR_VERSION
# define directfb_micro_version directfb_micro_version = DIRECTFB_MICRO_VERSION
# define directfb_binary_age    directfb_binary_age    = DIRECTFB_BINARY_AGE
# define directfb_interface_age directfb_interface_age = DIRECTFB_INTERFACE_AGE
# include <directfb.h>
# undef extern
# undef directfb_major_version
# undef directfb_minor_version
# undef directfb_micro_version
# undef directfb_binary_age
# undef directfb_interface_age
#else
#  define directfb_major_version fbdev_directfb_major_version_bogus
#  define directfb_minor_version fbdev_directfb_minor_version_bogus
#  define directfb_micro_version fbdev_directfb_micro_version_bogus
#  define directfb_binary_age    fbdev_directfb_binary_age_bogus
#  define directfb_interface_age fbdev_directfb_interface_age_bogus
# ifdef _FBDEV_DIRECTFB_BOGUS_GLOBALS
#  define extern
#  include <directfb.h>
#  undef extern
# else
#  include <directfb.h>
# endif
#  undef directfb_major_version
#  undef directfb_minor_version
#  undef directfb_micro_version
#  undef directfb_binary_age
#  undef directfb_interface_age
#  undef extern
#endif

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

#ifdef _FBDEV_DIRECTFB_GLOBALS
# define extern
# include <directfb-internal/core/gfxcard.h>
# undef extern
#else
# define card fbdev_directfb_dfb_card_bogus
# ifdef _FBDEV_DIRECTFB_BOGUS_GLOBALS
#  define extern
#  include <directfb-internal/core/gfxcard.h>
#  undef extern
# else
#  include <directfb-internal/core/gfxcard.h>
# endif
# undef card
#endif

#include <directfb-internal/core/state.h>
#include <directfb-internal/core/surfaces.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

/* Size of the font */
#define FWIDTH	8
#define FHEIGHT	8

struct fbdev_directfb_global {
  GfxCard	**dfb_card_ptr;
  DFBConfig	**dfb_config_ptr;
  unsigned int *dfb_major_version;
  unsigned int *dfb_minor_version;
  unsigned int *dfb_micro_version;
  unsigned int *dfb_binary_age;
  unsigned int *dfb_interface_age;
};

struct directfb_priv {
  struct fbdev_directfb_global globals;
  DFBConfig	dfbconfig;
  CardState	dfbstate;
  GfxDriver	gfxdriver;
  GfxCard	gfxcard;
  SurfaceBuffer	corebuffer;
  CoreSurface	coresurface;
  ggi_pixel	oldfg;
  ggi_pixel	oldbg;
};



/* Update GC components if needed */
static inline void
directfb_gcupdate(ggi_visual_t vis, /* Only used when unmappixel() needed. */
		  struct directfb_priv *priv,
		  ggi_mode *mode, ggi_gc *gc, 
		  int virtx, int yadd, DFBAccelerationMask accel)
		  
{
  CardState *dfbstate;
  DFBConfig *dfb_config;
  int newfg, newbg, newclip;

  GGIDPRINT("gcupdate called\n");

  dfbstate = &(priv->dfbstate);
  dfb_config = &(priv->dfbconfig);

  newfg = (gc->fg_color != priv->oldfg)
    || (dfbstate->drawingflags != DSDRAW_NOFX)
    || (dfbstate->blittingflags != DSBLIT_NOFX);
  newbg = (gc->bg_color != priv->oldbg)
    || (dfb_config->layer_bg_mode != DLBM_COLOR);
  newclip = 
    (gc->cliptl.x != dfbstate->clip.x1) ||
    (gc->clipbr.x != dfbstate->clip.x2) ||
    (gc->cliptl.y != dfbstate->clip.y1) ||
    (gc->clipbr.y != dfbstate->clip.y2);
  
  if (! (newfg || newbg || newclip)) return;

  if (newclip) {
    GGIDPRINT("updating clip\n");
    dfbstate->modified |= SMF_CLIP;
    dfbstate->clip.x1 = gc->cliptl.x; 
    dfbstate->clip.x2 = gc->clipbr.x;
    dfbstate->clip.y1 = gc->cliptl.y; 
    dfbstate->clip.y2 = gc->clipbr.y;
  }

  if (newfg) {
    ggi_color newfgcolor;

    GGIDPRINT("updating fbcolor\n");

    dfbstate->modified |= SMF_COLOR;
    ggiUnmapPixel(vis, gc->fg_color, &newfgcolor);
    dfbstate->color.a = 0; /* ?? */
    dfbstate->color.r = (uint8)(newfgcolor.r >> 8);
    dfbstate->color.g = (uint8)(newfgcolor.g >> 8);
    dfbstate->color.b = (uint8)(newfgcolor.b >> 8);
    if (dfbstate->drawingflags != DSDRAW_NOFX) {
      dfbstate->drawingflags = DSDRAW_NOFX;
      dfbstate->modified |= SMF_DRAWING_FLAGS;
    }
    if (dfbstate->blittingflags != DSBLIT_NOFX) {
      dfbstate->blittingflags = DSBLIT_NOFX;
      dfbstate->modified |= SMF_BLITTING_FLAGS;
    }

  }

  if (newbg) {
    ggi_color newbgcolor;
    GGIDPRINT("updating bgcolor\n");
    ggiUnmapPixel(vis, gc->bg_color, &newbgcolor);
    dfb_config->layer_bg_color.a = 0; /* ?? */
    dfb_config->layer_bg_color.r = (uint8)(newbgcolor.r >> 8);
    dfb_config->layer_bg_color.g = (uint8)(newbgcolor.g >> 8);
    dfb_config->layer_bg_color.b = (uint8)(newbgcolor.b >> 8);
    /* Force reread of the layer color.  Inefficient? */
    dfbstate->modified |= SMF_DESTINATION;
    priv->oldbg = gc->bg_color;
    dfb_config->layer_bg_mode = DLBM_COLOR;
  }

  priv->gfxcard.SetState(dfbstate, accel);

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
