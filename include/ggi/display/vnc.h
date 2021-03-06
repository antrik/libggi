/* $Id: vnc.h,v 1.47 2009/09/21 12:20:20 pekberg Exp $
******************************************************************************

   Display-vnc: definitions

   Copyright (C) 2008 Peter Rosin	[peda@lysator.liu.se]

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

struct ggi_vnc_client_t;
typedef int (ggi_vnc_client_action)(struct ggi_vnc_client_t *client);

/* Move to a central location? */
typedef struct {
	ggi_coord tl;
	ggi_coord br;
} ggi_rect;

typedef int (ggi_vnc_encode)(
	struct ggi_vnc_client_t *client, ggi_rect *update);

typedef struct {
	unsigned char *buf;
	int pos;
	int size;
	int limit;
} ggi_vnc_buf;

typedef struct ggi_vnc_client_t {
	GG_LIST_ENTRY(ggi_vnc_client_t) siblings;

	struct ggi_visual *owner;

	int cfd;
	int cwfd;
	int protover;

	int input;

	int (*read_ready)(struct ggi_vnc_client_t *client);
	int (*write_ready)(struct ggi_vnc_client_t *client);
	int (*safe_write)(struct ggi_vnc_client_t *client,
		int close_on_error);
	unsigned char *buf;
	int buf_size;
	int buf_limit;
	ggi_vnc_buf wbuf;
	int write_pending;
	ggi_vnc_client_action *action;
	ggi_vnc_encode *encode;

	int tight_extension;
	void *vencrypt;
	int copy_rect;
	void *rre_ctx;
	void *hextile_ctx;
	void *zlib_ctx;
	void *zlibhex_ctx;
	void *trle_ctx;
	void *zrle_ctx;
	void *tight_ctx;
	int desktop_size;
	int gii;
	int desktop_name;

	ggi_pixelformat pixfmt;
	ggi_pixelformat requested_pixfmt;
	int update_pixfmt;
	struct ggi_visual *vis;
	int palette_dirty;
	int reverse_endian;
	ggi_rect dirty;
	ggi_rect fdirty;
	ggi_rect update;
	ggi_coord origin;

	uint8_t challenge[16];
} ggi_vnc_client;

typedef struct {
	int	display;
	int	sfd;

	int	view_only;

	struct ggi_visual *fb;
	struct gg_instance *inp;

	gii_vnc_add_cfd  *add_cfd;
	gii_vnc_del_cfd  *del_cfd;
	gii_vnc_add_cwfd *add_cwfd;
	gii_vnc_del_cwfd *del_cwfd;
	gii_vnc_add_crfd *add_crfd;
	gii_vnc_del_crfd *del_crfd;
	gii_vnc_key      *key;
	gii_vnc_pointer  *pointer;
	gii_vnc_inject   *inject;
	void *gii_ctx;

	int           passwd;
	int           viewpw;
	void         *passwd_ks;
	void         *viewpw_ks;
	char          random17[9];

	int kill_on_last_disconnect;

	int copyrect;
	int rre;
	int corre;
	int hextile;
	int zlib_level;
	int zlibhex_level;
	int trle;
	int zrle_level;
	int tight;
	int desktop_size;
	int gii;
	int desktop_name;
	int wmvi;
	void *vencrypt;

	char title[80];

	GG_LIST_HEAD(clients, ggi_vnc_client_t) clients;
} ggi_vnc_priv;

#define GGI_VNC_DESKTOP_NAME (0)

gii_vnc_new_client		GGI_vnc_new_client;
gii_vnc_client_data		GGI_vnc_client_data;
gii_vnc_write_client		GGI_vnc_write_client;
gii_vnc_safe_write		GGI_vnc_safe_write;

void GGI_vnc_new_client_finish(struct ggi_visual *vis, int cfd, int cwfd);
void GGI_vnc_close_client(ggi_vnc_client *client);
int GGI_vnc_change_pixfmt(ggi_vnc_client *client);
void GGI_vnc_client_invalidate_nc_xyxy(ggi_vnc_client *client,
	int tlx, int tly, int brx, int bry);
void GGI_vnc_invalidate_nc_xyxy(struct ggi_visual *vis,
	int tlx, int tly, int brx, int bry);
void GGI_vnc_invalidate_xyxy(struct ggi_visual *vis,
	int tlx, int tly, int brx, int bry);
void GGI_vnc_invalidate_palette(struct ggi_visual *vis);

#define VNC_PRIV(vis)	((ggi_vnc_priv *) LIBGGI_PRIVATE(vis))


struct ggi_vnc_cmddata_clipboard {
	uint8_t *data;
	int size;
};
#define GGI_VNC_CLIPBOARD (0 | GII_CMDFLAG_PRIVATE)


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
ggifunc_getpixel_nc	GGI_vnc_getpixel_nc;

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
