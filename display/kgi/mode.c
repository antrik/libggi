/* $ix: mode.c,v 1.4 2002/12/23 13:14:54 ortalo Exp $
******************************************************************************

   Display-kgi: mode management

   Copyright (C) 1997 Jason McMullan    [jmcc@ggi-project.org]

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

#include "kgi/config.h"
#include <ggi/display/kgi.h>
#include <ggi/internal/font/8x8>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

#include "../common/pixfmt-setup.inc"
#include "../common/gt-auto.inc"

static ggi_graphtype image_mode_to_graphtype(kgi_image_mode_t *mode)
{
	int bits, i;

	if (!mode) 
		return GT_INVALID;
		
	if (mode->flags & KGI_IF_TEXT16)
		return GT_TEXT16;
		
	bits = 0;
	for (i = 0; mode->bpfa[i] && (i < __KGI_MAX_NR_ATTRIBUTES); i++)
		bits += mode->bpfa[i];
		
	switch (bits) {

	case 1:  return GT_1BIT;
	case 2:  return GT_2BIT;
	case 4:  return GT_4BIT;
	case 8:  return GT_8BIT;
	case 15: return GT_15BIT;
	case 16: return GT_16BIT;
	case 24: return GT_24BIT;
	case 32: return GT_32BIT;
	}
	
	return GT_INVALID;
}

static void GGI_kgi_mode2ggi(ggi_mode *tm, kgi_image_mode_t *mode)
{
	tm->graphtype = image_mode_to_graphtype(mode);
	tm->visible.x = mode->size.x;
	tm->visible.y = mode->size.y;
	tm->virt.x = mode->virt.x;
	tm->virt.y = mode->virt.y;
	tm->size.x = GGI_AUTO;
	tm->size.y = GGI_AUTO;
	tm->dpp.x = 1;
	tm->dpp.y = 1;
}

static void install_font(uint8 *ptr) {
	int i;

	for (i = 0; i < 256 * 8; i++) {
		int j;
		j = (i % 8) * 256 * 8 + (i / 8) * 8;
		ptr[j++] = (font[i] & 0x80) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x40) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x20) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x10) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x08) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x04) ? 0xff : 0x00;
		ptr[j++] = (font[i] & 0x02) ? 0xff : 0x00;
		ptr[j] =   (font[i] & 0x01) ? 0xff : 0x00;
	}
}

int GGI_kgi_set_origin(ggi_visual *vis, int x, int y)
{
	ggi_kgi_priv *priv;

	priv = LIBGGI_PRIVATE(vis);

	if (x > (LIBGGI_VIRTX(vis) - LIBGGI_X(vis))) return GGI_EARGINVAL;
	if (y > (LIBGGI_VIRTY(vis) - LIBGGI_Y(vis))) return GGI_EARGINVAL;
	vis->origin_x = x;
	vis->origin_y = y;

	if (priv->origin_changed) return priv->origin_changed(vis);

	/* Place for Paul to put the default KGI interface */

	return 0;
}

int GGI_kgi_set_display_frame(ggi_visual *vis, int num)
{
	ggi_kgi_priv *priv;

	priv = LIBGGI_PRIVATE(vis);

	if ((num < 0) || (num >= LIBGGI_MODE(vis)->frames))
		return GGI_EARGINVAL;
	vis->d_frame_num = num;

	if (priv->origin_changed) return priv->origin_changed(vis);

	/* Place for Paul to put the default KGI interface */

	return 0;
}

int GGI_kgi_set_read_frame(ggi_visual *vis, int num)
{
	ggi_kgi_priv *priv;
        ggi_directbuffer *db;

	db = _ggi_db_find_frame(vis, num);

        if (db == NULL) return -1;

        vis->r_frame_num = num;
        vis->r_frame = db;

	priv = LIBGGI_PRIVATE(vis);

	if (priv->rwframes_changed) return priv->rwframes_changed(vis);

	return 0;
}

int GGI_kgi_set_write_frame(ggi_visual *vis, int num)
{
	ggi_kgi_priv *priv;
        ggi_directbuffer *db;

	db = _ggi_db_find_frame(vis, num);

        if (db == NULL) return -1;

        vis->w_frame_num = num;
        vis->w_frame = db;

	priv = LIBGGI_PRIVATE(vis);

	if (priv->rwframes_changed) return priv->rwframes_changed(vis);

	return 0;
}


int GGI_kgi_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	switch(num){
	case 0:
		strcpy(apiname, "display-kgi");
		strcpy(arguments, "");
		return 0;
	case 1:
		strcpy(apiname, "generic-stubs");
		strcpy(arguments, "");
		return 0;
	case 2:
		strcpy(apiname, "generic-color");
		strcpy(arguments, "");
		return 0;
	case 3:
		if (GT_SCHEME(LIBGGI_GT(vis)) == GT_TEXT) {
			sprintf(apiname, "generic-text-%d",
				GT_SIZE(LIBGGI_GT(vis)));
		}
		else {
			sprintf(apiname, "generic-linear-%d%s",
				GT_SIZE(LIBGGI_GT(vis)),
				(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT)
				? "-r" : "");
		}
		strcpy(arguments, "");
		return 0;
	case 4:
	{
		const kgic_mapper_resource_info_result_t *accel;
		int name_size;
		char *space;

		accel = kgiGetResource(&KGI_CTX(vis), 0, KGI_RT_ACCEL);
		if (! accel) {
		
			GGIDPRINT("Didn't find an accelerator\n");
			return -1;
		}
		
		space = strchr(accel->name, ' ');
		if (space)
			name_size = space - accel->name;
		else
			name_size = strlen(accel->name);
		
		strcpy(apiname, "kgi-");
		strncat(apiname, accel->name, name_size);
		strcpy(arguments, "");
		
		return 0;
	}
	default:
		break;
	}
	
	return -1;
}

int GGI_kgi_getmode(ggi_visual *vis, ggi_mode *mode)
{
        memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));
        return 0;
}

int GGI_kgi_setmode(ggi_visual *vis, ggi_mode *tm)
{
	const kgic_mapper_resource_info_result_t *fb;
	int id, i, stride;
	char sugname[256], args[256];
	int err;
	kgi_u8_t *fb_ptr;
	kgi_size_t pad;
	ggi_kgi_priv *priv;

	if (vis == NULL) {
		GGIDPRINT("Visual==NULL\n");
		return GGI_EARGINVAL;
	}

	priv = KGI_PRIV(vis);

	err = GGI_kgi_checkmode(vis, tm);
	if (err) return err;

	if(kgiSetMode(&KGI_CTX(vis)) != KGI_EOK){
		GGIDPRINT_LIBS("Unable to set mode \n");
		return -1;
	}
	GGIDPRINT_LIBS("Mode set\n");

	if(LIBGGI_APPLIST(vis)->num){
		for (i = 0; i < LIBGGI_APPLIST(vis)->num; ++i) {
			_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
			_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
		}
	}
	
	if(priv->fb)
		munmap(priv->fb, priv->fb_size);

	fb = kgiGetResource(&KGI_CTX(vis), 0, KGI_RT_MMIO);
	if (! fb) {
		GGIDPRINT_LIBS("No framebuffer resource available");
		return -1;
	}

	GGIDPRINT("Found fb as resource %d", fb->resource);

	kgiSetupMmapFB(&KGI_CTX(vis), fb->resource);

	priv->fb_size = fb->info.mmio.size;
	priv->fb = mmap(NULL, priv->fb_size,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			KGI_CTX(vis).mapper.fd, 0);

	if (priv->fb == MAP_FAILED){
		GGIDPRINT_LIBS("Unable to map the frambuffer\n");
		return -1;
	}

	*LIBGGI_MODE(vis) = *tm;
	
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), tm->graphtype);

	GGIDPRINT_LIBS("Pixelformat set\n");

	fb_ptr = priv->fb;

	/* "Page" align the frames.  Some accels need this. */
	pad = 0;
	stride = (GT_SIZE(tm->graphtype) * tm->virt.x + 7) / 8;
	if ((stride * tm->virt.y) & 4095)
		pad = 4096 - ((stride * tm->virt.y) % 4096);
	for (i = 0; i < tm->frames; ++i) {
		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type = 
			GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->write = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = stride;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = 
			LIBGGI_PIXFMT(vis);

		fb_ptr += stride * tm->virt.y + pad;
	}

	/* Set up swatches. */
	if (priv->use3d && (priv->swatch_size >= 0)) {
		kgi_size_t avail;

		avail = priv->fb_size;
		avail -= fb_ptr - priv->fb;

		/* We want at least 10 scanlines and space for the font. */

		if (priv->swatch_size) {
			if (sizeof(font)*8 + stride*10 > priv->swatch_size) {
				fprintf(stderr, "More swatch needed\n");
				return GGI_EARGINVAL;
			}
			if (priv->swatch_size > avail) {
				fprintf(stderr, "No space for swatch\n");
				return GGI_ENOMEM;
			}
		}
		else {
			priv->swatch_size = avail;
			if (stride * tm->virt.y / 2 + sizeof(font)*8 < avail) {
				priv->swatch_size = 
				  stride * tm->virt.y / 2 + sizeof(font)*8;
			}
			if (sizeof(font)*8 + stride*10 > priv->swatch_size) {
				fprintf(stderr, "No space for swatch\n");
				return GGI_EARGINVAL;
			}
		}
		priv->font = fb_ptr;
		priv->swatch = fb_ptr + sizeof(font)*8;
		priv->swatch_size -= sizeof(font)*8;
		
		GGIDPRINT("Font at %p, %i byte swatch at %p.\n", 
			  priv->font, priv->swatch_size, priv->swatch);

		install_font((uint8 *)priv->font);
	}
	priv->swatch_gp = (kgi_u8_t *)(priv->swatch - priv->fb);
	priv->font_gp = (kgi_u8_t *)(priv->font - priv->fb);

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	_ggiZapMode(vis, 0);

	/* TODO: Do not assume that an accelerator is always present */
	vis->needidleaccel = 1; /* Temp hack until we work out the */
	vis->accelactive = 0;   /* new changed() traversal for renderers */

	for(id = 1; 0 == GGI_kgi_getapi(vis, id, sugname, args); ++id){
		if(_ggiOpenDL(vis, sugname, args, NULL)){
			GGIDPRINT_LIBS("kgi: can't open %s\n", sugname);
			if (id < 4) return GGI_EFATAL;
			else continue;
		}
		GGIDPRINT_LIBS("kgi: loaded %s\n", sugname);
	}

	if(GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE){
		vis->palette = _ggi_malloc(sizeof(ggi_color) * 256);
		vis->opcolor->setpalvec = GGI_kgi_setpalvec;
	}

	/* Load generic frame/origin handling functions */

	vis->opdraw->setorigin = GGI_kgi_set_origin;
        vis->opdraw->setdisplayframe = GGI_kgi_set_display_frame;
        vis->opdraw->setreadframe = GGI_kgi_set_read_frame;
        vis->opdraw->setwriteframe = GGI_kgi_set_write_frame;

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}



/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_kgi_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	kgi_image_mode_t mode;
	int frames, virty;

	if (vis == NULL || tm == NULL) return GGI_EARGINVAL;

	frames = tm->frames;

	memset(&mode, 0, sizeof(kgi_image_mode_t));

	/* Take care of automatic graphtype selection */
	tm->graphtype = _GGIhandle_gtauto(tm->graphtype);

	switch (GT_SCHEME(tm->graphtype)) {
	
	case GT_TEXT:
		mode.flags |= KGI_IF_TEXT16;
		break;
		
	case GT_TRUECOLOR:
		mode.fam |= KGI_AM_COLORS;
		mode.bpfa[0]= GT_DEPTH(tm->graphtype) / 3;
		mode.bpfa[1]= GT_DEPTH(tm->graphtype) / 3 +
			      GT_DEPTH(tm->graphtype) % 3;
		mode.bpfa[2]= GT_DEPTH(tm->graphtype) / 3;
		mode.bpfa[3]= GT_SIZE(tm->graphtype) - GT_DEPTH(tm->graphtype);
		if (mode.bpfa[3])
			mode.fam |= KGI_AM_APPLICATION;
		break;
	
	case GT_PALETTE:
	case GT_STATIC_PALETTE:	
		mode.fam |= KGI_AM_COLOR_INDEX;
		mode.bpfa[0] = GT_SIZE(tm->graphtype);
		break;

	default:
		return -1;
	}

	mode.frames = 1;	
	mode.size.x = tm->visible.x;
	mode.size.y = tm->visible.y;
	mode.virt.x = tm->virt.x;

	if (frames > 1) {
		kgi_size_t pad;

		/* Hack... we essentially cannot autosize vertically
		 * with frames > 1.  Limitation of KGI.  Should be fixed
		 * in KGI (by adding GGI "frames" support, which will have
		 * to go under another name).
		 */
		if (mode.virt.y == GGI_AUTO)
			mode.virt.y = tm->visible.y;
		if (mode.virt.x == GGI_AUTO) 
			mode.virt.x = tm->visible.x;

		/* Most targets benefit from or require page-aligned frames. */
		virty = mode.virt.y;
		pad = mode.virt.y * mode.virt.x * GT_SIZE(tm->graphtype) / 8;
		if (pad & 4095) { pad /= 4096; pad++; pad *= 4096; }
		pad *= frames;
		mode.virt.y = pad / (mode.virt.x * GT_SIZE(tm->graphtype) / 8);
		mode.virt.y++;
	}
	else {
		virty = mode.virt.y;
		mode.virt.y = tm->virt.y;
	}

	GGIDPRINT("%d, %d, %d, %d\n", mode.size.x, mode.size.y, mode.virt.x, mode.virt.y);

	/* hack: kgi doesn't allow multiple mode checks */
	kgiUnsetMode(&KGI_CTX(vis));

	if(kgiSetImages(&KGI_CTX(vis), 1) != KGI_EOK){
		GGIDPRINT_LIBS("Unable to set images\n");
		return -1;
	}
	if(kgiSetImageMode(&KGI_CTX(vis), 0, &mode) != KGI_EOK){
		GGIDPRINT_LIBS("Unable to set image mode\n");
		return -1;
	}
	if(kgiCheckMode(&KGI_CTX(vis)) != KGI_EOK){
		GGIDPRINT_LIBS("Unable to check mode\n");
		return -1;
	}
	memset(&mode, 0, sizeof(kgi_image_mode_t));
	if (kgiGetImageMode(&KGI_CTX(vis), 0, &mode)) {
		GGIDPRINT_LIBS("Unable to get mode\n");
		return GGI_EFATAL;
	}
	kgiPrintImageMode(&mode);
	GGI_kgi_mode2ggi(tm, &mode);
	tm->frames = frames;
	tm->virt.y = virty;
	if (frames < 2) tm->virt.y = mode.virt.y;

	return 0;
}


/*************************/
/* set the current flags */
/*************************/
int GGI_kgi_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	/* Only raise supported flags */
	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC;
	return 0;
}
