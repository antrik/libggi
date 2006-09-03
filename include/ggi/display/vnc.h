/* $Id: vnc.h,v 1.19 2006/09/03 21:00:29 pekberg Exp $
******************************************************************************

   Display-vnc: definitions

   Copyright (C) 2006 Peter Rosin	[peda@lysator.liu.se]

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

#ifndef _GGI_DISPLAY_VNC_H
#define _GGI_DISPLAY_VNC_H

#include <ggi/internal/ggi-dl.h>
#include <ggi/input/vnc.h>

typedef int (ggi_vnc_client_action)(struct ggi_visual *vis);

/* Move to a central location? */
typedef struct {
	ggi_coord tl;
	ggi_coord br;
} ggi_rect;

typedef void (ggi_vnc_encode)(struct ggi_visual *vis, ggi_rect *update);

typedef struct {
	unsigned char *buf;
	int pos;
	int size;
	int limit;
} ggi_vnc_buf;

typedef struct {
	int cfd;
	int protover;

	unsigned char buf[256];
	int buf_size;
	ggi_vnc_buf wbuf;
	int write_pending;
	ggi_vnc_client_action *action;
	uint16_t encoding_count;
	ggi_vnc_encode *encode;

	void *zlib_ctx;
	void *zrle_ctx;

	struct ggi_visual *vis;
	int palette_dirty;
	int reverse_endian;
	ggi_rect dirty;
	ggi_rect fdirty;
	ggi_rect update;

	uint8_t challenge[16];
} ggi_vnc_client;

typedef struct {
	int	display;
	int	sfd;

	struct ggi_visual *fb;
	struct gg_module *inp;

	gii_vnc_add_cfd  *add_cfd;
	gii_vnc_del_cfd  *del_cfd;
	gii_vnc_add_cwfd *add_cwfd;
	gii_vnc_del_cwfd *del_cwfd;
	gii_vnc_key      *key;
	gii_vnc_pointer  *pointer;
	void *gii_ctx;

	int           passwd;
	unsigned long cooked_key[32];
	unsigned long randomizer[32];

	int zlib_level;
	int zrle_level;

	char title[80];

	ggi_vnc_client *client;
} ggi_vnc_priv;


gii_vnc_new_client		GGI_vnc_new_client;
gii_vnc_client_data		GGI_vnc_client_data;
gii_vnc_write_client		GGI_vnc_write_client;

void GGI_vnc_new_client_finish(struct ggi_visual *vis, int cfd);
void GGI_vnc_close_client(struct ggi_visual *vis);
void GGI_vnc_invalidate_nc_xyxy(struct ggi_visual *vis,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry);
void GGI_vnc_invalidate_xyxy(struct ggi_visual *vis,
	uint16_t tlx, uint16_t tly, uint16_t brx, uint16_t bry);
void GGI_vnc_invalidate_palette(struct ggi_visual *vis);

#define VNC_PRIV(vis)	((ggi_vnc_priv *) LIBGGI_PRIVATE(vis))


/* LibGGI Interface
 */

ggifunc_gcchanged	GGI_vnc_gcchanged;

/* mode */
ggifunc_getmode		GGI_vnc_getmode;
ggifunc_checkmode	GGI_vnc_checkmode;
ggifunc_setmode		GGI_vnc_setmode;
ggifunc_getapi		GGI_vnc_getapi;
ggifunc_setflags	GGI_vnc_setflags;

/* buffer */
ggifunc_setdisplayframe	GGI_vnc_setdisplayframe;
ggifunc_setreadframe	GGI_vnc_setreadframe;
ggifunc_setwriteframe	GGI_vnc_setwriteframe;
ggifunc_setorigin	GGI_vnc_setorigin;

/* pixel */
ggifunc_drawpixel	GGI_vnc_drawpixel;
ggifunc_drawpixel	GGI_vnc_drawpixel_nc;
ggifunc_putpixel	GGI_vnc_putpixel;
ggifunc_putpixel	GGI_vnc_putpixel_nc;
ggifunc_getpixel	GGI_vnc_getpixel;

/* line */
ggifunc_drawline	GGI_vnc_drawline;
ggifunc_drawhline	GGI_vnc_drawhline;
ggifunc_drawhline	GGI_vnc_drawhline_nc;
ggifunc_puthline	GGI_vnc_puthline;
ggifunc_gethline	GGI_vnc_gethline;
ggifunc_drawvline	GGI_vnc_drawvline;
ggifunc_drawvline	GGI_vnc_drawvline_nc;
ggifunc_putvline	GGI_vnc_putvline;
ggifunc_getvline	GGI_vnc_getvline;

/* box */
ggifunc_drawbox		GGI_vnc_drawbox;
ggifunc_putbox		GGI_vnc_putbox;
ggifunc_getbox		GGI_vnc_getbox;
ggifunc_copybox		GGI_vnc_copybox;
ggifunc_crossblit	GGI_vnc_crossblit;
ggifunc_fillscreen	GGI_vnc_fillscreen;

/* gtext */
ggifunc_putc		GGI_vnc_putc;
ggifunc_puts		GGI_vnc_puts;
ggifunc_getcharsize	GGI_vnc_getcharsize;

/* color */
ggifunc_setpalvec	GGI_vnc_setpalvec;
ggifunc_getpalvec	GGI_vnc_getpalvec;
ggifunc_mapcolor	GGI_vnc_mapcolor;
ggifunc_unmappixel	GGI_vnc_unmappixel;
ggifunc_packcolors	GGI_vnc_packcolors;
ggifunc_unpackpixels	GGI_vnc_unpackpixels;

ggifunc_setgamma	GGI_vnc_setgamma;
ggifunc_getgamma	GGI_vnc_getgamma;
ggifunc_setgammamap	GGI_vnc_setgammamap;
ggifunc_getgammamap	GGI_vnc_getgammamap;

#endif /* _GGI_DISPLAY_VNC_H */
