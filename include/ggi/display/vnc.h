/* $Id: vnc.h,v 1.2 2006/08/21 21:05:41 pekberg Exp $
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

typedef struct {
	int	display;
	int	sfd;
	int	cfd;
	int	protover;

	unsigned char *fb;

	struct gg_module *inp;

	gii_vnc_add_cfd *add_cfd;
	gii_vnc_del_cfd *del_cfd;
	gii_vnc_key     *key;
	void *gii_ctx;

	unsigned char buf[256];
	int buf_size;
	ggi_vnc_client_action *client_action;

	struct ggi_visual *client_vis;
	int palette_dirty;
	int reverse_endian;

	int           passwd;
	unsigned long cooked_key[32];
	uint8_t       challenge[16];
} ggi_vnc_priv;


gii_vnc_new_client		GGI_vnc_new_client;
gii_vnc_client_data		GGI_vnc_client_data;


#define VNC_PRIV(vis)	((ggi_vnc_priv *) LIBGGI_PRIVATE(vis))


/* LibGGI Interface
 */

ggifunc_getmode			GGI_vnc_getmode;
ggifunc_setmode			GGI_vnc_setmode;
ggifunc_checkmode		GGI_vnc_checkmode;
ggifunc_getapi			GGI_vnc_getapi;
ggifunc_setflags		GGI_vnc_setflags;
ggifunc_setPalette		GGI_vnc_setPalette;

#endif /* _GGI_DISPLAY_VNC_H */
