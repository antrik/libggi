/* $Id: builtins.c,v 1.7 2006/05/05 21:20:15 cegger Exp $
******************************************************************************

   Libggi builtin targets bindings.

   Copyright (C) 2005 Eric Faurot	[eric.faurot@gmail.com]

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
#include <ggi/gg.h>
#include <ggi/internal/ggi_debug.h>
#include <string.h>

typedef int (ggifunc_dlinit)(int, void **);

#ifdef BUILTIN_DEFAULT
ggifunc_dlinit GGIdl_color;
ggifunc_dlinit GGIdl_ilbm;
ggifunc_dlinit GGIdl_iplanar_2p;
ggifunc_dlinit GGIdl_linear_1;
ggifunc_dlinit GGIdl_linear_16;
ggifunc_dlinit GGIdl_linear_1_r;
ggifunc_dlinit GGIdl_linear_2;
ggifunc_dlinit GGIdl_linear_24;
ggifunc_dlinit GGIdl_linear_32;
ggifunc_dlinit GGIdl_linear_4;
ggifunc_dlinit GGIdl_linear_4_r;
ggifunc_dlinit GGIdl_linear_8;
ggifunc_dlinit GGIdl_planar;
ggifunc_dlinit GGIdl_pseudo_stubs;
ggifunc_dlinit GGIdl_stubs;
ggifunc_dlinit GGIdl_text_16;
ggifunc_dlinit GGIdl_text_32;
#endif

#ifdef BUILTIN_DISPLAY
#endif

#ifdef BUILTIN_DEFAULT_FBDEV_ACCEL
ggifunc_dlinit GGIdl_fbdev_3dlabs_pm2;
ggifunc_dlinit GGIdl_fbdev_mach64;
ggifunc_dlinit GGIdl_fbdev_m2164w;
ggifunc_dlinit GGIdl_fbdev_mga_g400;
#endif
#ifdef BUILTIN_DEFAULT_FBDEV_DIRECTFB
ggifunc_dlinit GGIdl_fbdev_directfb;
ggifunc_dlinit GGIdl_fbdev_directfbglobal;
#endif

#ifdef BUILTIN_DEFAULT_KGI_ACCEL
ggifunc_dlinit GGIdl_kgi_mach64;
ggifunc_dlinit GGIdl_kgi_radeon;
ggifunc_dlinit GGIdl_kgi_Gx00;
#endif

#ifdef BUILTIN_DISPLAY_AA
ggifunc_dlinit GGIdl_aa;
#endif
#ifdef BUILTIN_DISPLAY_DIRECTX
ggifunc_dlinit GGIdl_directx;
#endif
#ifdef BUILTIN_DISPLAY_FBDEV
ggifunc_dlinit GGIdl_fbdev;
#endif
#ifdef BUILTIN_DISPLAY_FILE
ggifunc_dlinit GGIdl_file;
#endif
#ifdef BUILTIN_DISPLAY_GLIDE
ggifunc_dlinit GGIdl_glide;
#endif
#ifdef BUILTIN_DISPLAY_IPC
ggifunc_dlinit GGIdl_ipc;
#endif
#ifdef BUILTIN_DISPLAY_KGI
ggifunc_dlinit GGIdl_kgi;
#endif
#ifdef BUILTIN_DISPLAY_LCD823
ggifunc_dlinit GGIdl_lcd823;
#endif
#ifdef BUILTIN_DISPLAY_LIBKGI
ggifunc_dlinit GGIdl_libkgi;
#endif
#ifdef BUILTIN_HELPER_LINVTSW
ggifunc_dlinit GGIdl_linvtsw;
#endif
#ifdef BUILTIN_HELPER_MANSYNC
ggifunc_dlinit GGIdl_mansync;
#endif
#ifdef BUILTIN_DISPLAY_MEMORY
ggifunc_dlinit GGIdl_memory;
#endif
#ifdef BUILTIN_DISPLAY_MONOTEXT
ggifunc_dlinit GGIdl_monotext;
#endif
#ifdef BUILTIN_DISPLAY_PALEMU
ggifunc_dlinit GGIdl_palemu;
#endif
#ifdef BUILTIN_DISPLAY_QUARTZ
ggifunc_dlinit GGIdl_quartz;
#endif
#ifdef BUILTIN_DISPLAY_SUB
ggifunc_dlinit GGIdl_sub;
#endif
#ifdef BUILTIN_DISPLAY_SVGALIB
ggifunc_dlinit GGIdl_svgalib;
#endif
#ifdef BUILTIN_DISPLAY_TELE
ggifunc_dlinit GGIdl_tele;
#endif
#ifdef BUILTIN_DISPLAY_TERMINFO
ggifunc_dlinit GGIdl_terminfo;
#endif
#ifdef BUILTIN_DISPLAY_TILE
ggifunc_dlinit GGIdl_multi;
ggifunc_dlinit GGIdl_tile;
#endif
#ifdef BUILTIN_DISPLAY_TRUEEMU
ggifunc_dlinit GGIdl_trueemu;
#endif
#ifdef BUILTIN_DISPLAY_VCSA
ggifunc_dlinit GGIdl_vcsa;
#endif
#ifdef BUILTIN_HELPER_VGAGL
ggifunc_dlinit GGIdl_vgagl;
#endif
#ifdef BUILTIN_DISPLAY_VGL
ggifunc_dlinit GGIdl_vgl;
#endif
#ifdef BUILTIN_DISPLAY_WSFB
ggifunc_dlinit GGIdl_wsfb;
#endif
#ifdef BUILTIN_DISPLAY_X
ggifunc_dlinit GGIdl_X;
#endif
#ifdef BUILTIN_HELPER_X_DBE
ggifunc_dlinit GGIdl_helper_x_dbe;
#endif
#ifdef BUILTIN_HELPER_X_DGA
ggifunc_dlinit GGIdl_helper_x_dga;
#endif
#ifdef BUILTIN_HELPER_X_EVI
ggifunc_dlinit GGIdl_helper_x_evi;
#endif
#ifdef BUILTIN_HELPER_X_SHM
ggifunc_dlinit GGIdl_helper_x_shm;
#endif
#ifdef BUILTIN_HELPER_X_VIDMODE
ggifunc_dlinit GGIdl_helper_x_vidmode;
#endif

struct target { 
	const char     *target;
	ggifunc_dlinit *func;
};

static struct target _targets[] = {

#ifdef BUILTIN_DEFAULT
        { "GGIdl_color", &GGIdl_color },
        { "GGIdl_ilbm", &GGIdl_ilbm },
        { "GGIdl_iplanar_2p", &GGIdl_iplanar_2p },
        { "GGIdl_linear_1", &GGIdl_linear_1 },
        { "GGIdl_linear_16", &GGIdl_linear_16 },
        { "GGIdl_linear_1_r", &GGIdl_linear_1_r },
        { "GGIdl_linear_2", &GGIdl_linear_2 },
        { "GGIdl_linear_24", &GGIdl_linear_24 },
        { "GGIdl_linear_32", &GGIdl_linear_32 },
        { "GGIdl_linear_4", &GGIdl_linear_4 },
        { "GGIdl_linear_4_r", &GGIdl_linear_4_r },
        { "GGIdl_linear_8", &GGIdl_linear_8 },
        { "GGIdl_planar", &GGIdl_planar },
        { "GGIdl_pseudo_stubs", &GGIdl_pseudo_stubs },
        { "GGIdl_stubs", &GGIdl_stubs },
        { "GGIdl_text_16", &GGIdl_text_16 },
        { "GGIdl_text_32", &GGIdl_text_32 },
#endif

#ifdef BUILTIN_DISPLAY
#endif

#ifdef BUILTIN_DEFAULT_FBDEV_ACCEL
	{ "GGIdl_3dlabs_pm2", &GGIdl_fbdev_3dlabs_pm2 },
        { "GGIdl_mach64", &GGIdl_fbdev_mach64 },
        { "GGIdl_m2164w", &GGIdl_fbdev_m2164w },
        { "GGIdl_mga_g400", &GGIdl_fbdev_mga_g400 },
#endif
#ifdef BUILTIN_DEFAULT_FBDEV_DIRECTFB
        { "GGIdl_directfb", &GGIdl_directfb },
        { "GGIdl_directfb", &GGIdl_directfbglobal },
#endif

#ifdef BUILTIN_DEFAULT_KGI_ACCEL
        { "GGIdl_mach64", &GGIdl_kgi_mach64 },
        { "GGIdl_radeon", &GGIdl_kgi_radeon },
        { "GGIdl_Gx00", &GGIdl_kgi_Gx00 },
#endif

#ifdef BUILTIN_DISPLAY_AA
        { "GGIdl_aa", &GGIdl_aa },
#endif
#ifdef BUILTIN_DISPLAY_DIRECTX
        { "GGIdl_directx", &GGIdl_directx },
#endif
#ifdef BUILTIN_DISPLAY_FBDEV
        { "GGIdl_fbdev", &GGIdl_fbdev },
#endif
#ifdef BUILTIN_DISPLAY_FILE
        { "GGIdl_file", &GGIdl_file },
#endif
#ifdef BUILTIN_DISPLAY_GLIDE
        { "GGIdl_glide", &GGIdl_glide },
#endif
#ifdef BUILTIN_DISPLAY_IPC
        { "GGIdl_ipc", &GGIdl_ipc },
#endif
#ifdef BUILTIN_DISPLAY_KGI
        { "GGIdl_kgi", &GGIdl_kgi },
#endif
#ifdef BUILTIN_DISPLAY_LCD823
        { "GGIdl_lcd823", &GGIdl_lcd823 },
#endif
#ifdef BUILTIN_DISPLAY_LIBKGI
        { "GGIdl_libkgi", &GGIdl_libkgi },
#endif
#ifdef BUILTIN_HELPER_LINVTSW
        { "GGIdl_linvtsw", &GGIdl_linvtsw },
#endif
#ifdef BUILTIN_HELPER_MANSYNC
        { "GGIdl_mansync", &GGIdl_mansync },
#endif
#ifdef BUILTIN_DISPLAY_MEMORY
        { "GGIdl_memory", &GGIdl_memory },
#endif
#ifdef BUILTIN_DISPLAY_MONOTEXT
        { "GGIdl_monotext", &GGIdl_monotext },
#endif
#ifdef BUILTIN_DISPLAY_PALEMU
        { "GGIdl_palemu", &GGIdl_palemu },
#endif
#ifdef BUILTIN_DISPLAY_QUARTZ
        { "GGIdl_quartz", &GGIdl_quartz },
#endif
#ifdef BUILTIN_DISPLAY_SUB
        { "GGIdl_sub", &GGIdl_sub },
#endif
#ifdef BUILTIN_DISPLAY_SVGALIB
        { "GGIdl_svgalib", &GGIdl_svgalib },
#endif
#ifdef BUILTIN_DISPLAY_TELE
        { "GGIdl_tele", &GGIdl_tele },
#endif
#ifdef BUILTIN_DISPLAY_TERMINFO
        { "GGIdl_terminfo", &GGIdl_terminfo },
#endif
#ifdef BUILTIN_DISPLAY_TILE
        { "GGIdl_multi", &GGIdl_multi },
        { "GGIdl_tile", &GGIdl_tile },
#endif
#ifdef BUILTIN_DISPLAY_TRUEEMU
        { "GGIdl_trueemu", &GGIdl_trueemu },
#endif
#ifdef BUILTIN_DISPLAY_VCSA
        { "GGIdl_vcsa", &GGIdl_vcsa },
#endif
#ifdef BUILTIN_HELPER_VGAGL
        { "GGIdl_vgagl", &GGIdl_vgagl },
#endif
#ifdef BUILTIN_DISPLAY_VGL
        { "GGIdl_vgl", &GGIdl_vgl },
#endif
#ifdef BUILTIN_DISPLAY_WSFB
        { "GGIdl_wsfb", &GGIdl_wsfb },
#endif
#ifdef BUILTIN_DISPLAY_X
        { "GGIdl_X", &GGIdl_X },
#endif
#ifdef BUILTIN_HELPER_X_DBE
        { "GGIdl_helper_x_dbe", &GGIdl_helper_x_dbe },
#endif
#ifdef BUILTIN_HELPER_X_DGA
        { "GGIdl_helper_x_dga", &GGIdl_helper_x_dga },
#endif
#ifdef BUILTIN_HELPER_X_EVI
        { "GGIdl_helper_x_evi", &GGIdl_helper_x_evi },
#endif
#ifdef BUILTIN_HELPER_X_SHM
        { "GGIdl_helper_x_shm", &GGIdl_helper_x_shm },
#endif
#ifdef BUILTIN_HELPER_X_VIDMODE
        { "GGIdl_helper_x_vidmode", &GGIdl_helper_x_vidmode },
#endif

	{ NULL, NULL }
};

static void * _builtins_get(void * _, const char *symbol) {
	struct target * t;
	for(t = _targets; t->target != NULL; t++) {
		if(!strcmp(t->target, symbol)) {
			return t->func;
		}
	}
	return NULL;
}

static gg_scope _builtins;

void _ggiInitBuiltins(void);
void _ggiExitBuiltins(void);

void _ggiInitBuiltins(void)
{
	_builtins = ggNewScope("@libggi", NULL, &_builtins_get,  NULL);
}

void _ggiExitBuiltins(void)
{
	ggDelScope(_builtins);
}

#ifndef HAVE_CONFFILE
const char const *_ggibuiltinconf[] = {
#include "builtins.inc"
       NULL
};
#endif /* HAVE_CONFFILE */
