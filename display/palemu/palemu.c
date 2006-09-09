/* $Id: palemu.c,v 1.12 2006/09/09 16:55:13 cegger Exp $
******************************************************************************

   Display-palemu: palette emulation on true-color modes

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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
#include <ggi/display/palemu.h>
#include <ggi/internal/ggi_debug.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/**************************************************
 ***
 ***  Internal functions 
 ***
 **************************************************/


static void blitter_1(ggi_palemu_priv *priv, void *dest, void *src, int w)
{
	uint8_t *s = (uint8_t *) src;
	uint8_t *d = (uint8_t *) dest;

	for (; w > 0; w--) {
		*d++ = priv->lookup[*s++];
	}
}

static void blitter_2(ggi_palemu_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t  *) src;
	uint16_t *d = (uint16_t *) dest;

	for (; w > 0; w--) {
		*d++ = priv->lookup[*s++];
	}
}

static void blitter_3(ggi_palemu_priv *priv, void *dest, void *src, int w)
{
	uint8_t *s = (uint8_t *) src;
	uint8_t *d = (uint8_t *) dest;

	for (; w > 0; w--) {
		ggi_pixel pix = priv->lookup[*s++];

		*d++ = pix; pix >>= 8;
		*d++ = pix; pix >>= 8;
		*d++ = pix;
	}
}

static void blitter_4(ggi_palemu_priv *priv, void *dest, void *src, int w)
{
	uint8_t  *s = (uint8_t  *) src;
	uint32_t *d = (uint32_t *) dest;

	for (; w > 0; w--) {
		*d++ = priv->lookup[*s++];
	}
}


/**************************************************
 ***
 ***  Exported functions 
 ***
 **************************************************/

/* !!! flesh out all four possibilities: (a) direct access to source
 * (Y/N), and (b) direct access to destination (Y/N).
 */
		
int _ggi_palemu_Transfer(struct ggi_visual *vis, int x, int y, int w, int h)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);
	int old_r_frame = vis->r_frame_num;
	uint8_t src_buf[8192];
	uint8_t dest_buf[8192];

	priv->mem_opdraw->setreadframe(vis, vis->d_frame_num);
	
	/* do transfer */
	for (; h > 0; h--, y++) {

		ggiGetHLine(vis->stem, x, y, w, src_buf);
			
		(* priv->do_blit)(priv, dest_buf, src_buf, w);

		ggiPutHLine(priv->parent, x, y, w, dest_buf);
	}

	priv->mem_opdraw->setreadframe(vis, old_r_frame);

	return 0;
}

int _ggi_palemu_Flush(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	int sx = priv->dirty_tl.x; int sy = priv->dirty_tl.y;
	int ex = priv->dirty_br.x; int ey = priv->dirty_br.y;


	/* clear the `dirty region' */

	priv->dirty_tl.x = LIBGGI_VIRTX(vis);
	priv->dirty_tl.y = LIBGGI_VIRTY(vis);
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;


	/* When write_frame != display_frame, then there is no need to
	 * update the parent since the affected area(s) are not visible.
	 */
	 
	if ((vis->w_frame_num == vis->d_frame_num) && 
	    (sx < ex) && (sy < ey)) {

		return _ggi_palemu_Transfer(vis, sx, sy, ex-sx, ey-sy);
	}

	return 0;
}

int _ggi_palemu_Open(struct ggi_visual *vis)
{
	int rc;
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);


	DPRINT("display-palemu: Open %dx%d#%dx%d\n", LIBGGI_X(vis), 
		LIBGGI_Y(vis), LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));


	/* set the parent mode */
	
	rc = ggiSetMode(priv->parent, &priv->parent_mode);
	if (rc < 0) {
		DPRINT("display-palemu: Couldn't set parent mode.\n");
		return rc;
	}

	DPRINT("display-palemu: parent is %d/%d\n",
		GT_DEPTH(priv->parent_mode.graphtype), 
		GT_SIZE(priv->parent_mode.graphtype));
	

	/* setup tables and choose blitter function */

	switch (GT_ByPP(priv->parent_mode.graphtype)) {

	case 1: priv->do_blit = &blitter_1;
		break;
		
	case 2: priv->do_blit = &blitter_2;
		break;
		
	case 3: priv->do_blit = &blitter_3;
		break;
		
	case 4: priv->do_blit = &blitter_4;
		break;
		
	default:
		DPRINT("Unsupported pixel size '%d'.\n",
			GT_SIZE(priv->parent_mode.graphtype));
		return GGI_ENOMATCH;
	}


	priv->palette = _ggi_malloc(256 * sizeof(ggi_color));
	priv->lookup  = _ggi_malloc(256 * sizeof(ggi_pixel));

	priv->red_gamma = priv->green_gamma = priv->blue_gamma = 1.0;

	
	/* clear the 'dirty region' */

	priv->dirty_tl.x = LIBGGI_VIRTX(vis);
	priv->dirty_tl.y = LIBGGI_VIRTY(vis);
	priv->dirty_br.x = 0;
	priv->dirty_br.y = 0;

	return 0;
}

int _ggi_palemu_Close(struct ggi_visual *vis)
{
	ggi_palemu_priv *priv = PALEMU_PRIV(vis);

	if (priv->palette != NULL) {
		free(priv->palette);
		priv->palette = NULL;
	}
	if (priv->lookup != NULL) {
		free(priv->lookup);
		priv->lookup = NULL;
	}

	return 0;
}
