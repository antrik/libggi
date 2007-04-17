/* $Id: mode.c,v 1.26 2007/04/17 00:27:49 ggibecka Exp $
******************************************************************************

   Display-file: mode management

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg	[marcus@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "config.h"
#include <ggi/display/file.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */

#include "../common/pixfmt-setup.inc"
#include "../common/ggi-auto.inc"
#include "../common/gt-auto.inc"

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif

#define write_byte	_ggi_file_write_byte
#define write_word	_ggi_file_write_word
#define write_string	_ggi_file_write_string
#define write_zeros	_ggi_file_write_zeros
#define write_flush	_ggi_file_flush

static void dowritefile(struct ggi_visual *vis)
{
	ggi_file_priv *priv = FILE_PRIV(vis);
	char cmdbuf[1024];

       	if (!(priv->flags & FILEFLAG_RAW)) {
		(* priv->writer)(vis);
	}

	if (priv->flushcmd) {
		snprintf(cmdbuf, 1024, priv->flushcmd,
			priv->flushcnt,priv->flushcnt,priv->flushcnt,
			priv->flushcnt,priv->flushcnt,priv->flushcnt,
			priv->flushcnt,priv->flushcnt,priv->flushcnt,
			priv->flushcnt,priv->flushcnt,priv->flushcnt);

		system(cmdbuf);
	}
	priv->flushcnt++;
}

static int GGI_file_flush(struct ggi_visual *vis, 
			int x, int y, int w, int h, int tryflag)
{
	ggi_file_priv *priv = FILE_PRIV(vis);
	struct timeval now;

	if ( priv->flushevery && (priv->flushtotal%priv->flushevery) == 0) {
		dowritefile(vis);
	}
	if (priv->flushstep.tv_sec || priv->flushstep.tv_usec) {
		gettimeofday(&now,NULL);
		if (   now.tv_sec >  priv->flushlast.tv_sec || 
		     ( now.tv_sec == priv->flushlast.tv_sec &&
		       now.tv_usec > priv->flushlast.tv_usec) ) {
			priv->flushlast.tv_sec +=priv->flushstep.tv_sec;
			priv->flushlast.tv_usec+=priv->flushstep.tv_usec;
			if (priv->flushlast.tv_usec    >=1000000) {
				priv->flushlast.tv_usec-=1000000;
				priv->flushlast.tv_sec++;
			}
			dowritefile(vis);
		}
	}
	priv->flushtotal++;
	return 0;
}

static int _ggi_rawstuff(struct ggi_visual *vis)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	ggi_graphtype gt = LIBGGI_GT(vis);

	int padding;


	/* Calculate stuff
	 */
	priv->offset_pal = 8+2+2+4+2+2;

	priv->offset_image = priv->offset_pal + priv->num_cols * 3;
	priv->offset_image += priv->fb_stride-1;
	priv->offset_image -= (priv->offset_image % priv->fb_stride);

	priv->file_size = priv->offset_image + priv->fb_size;
	priv->file_size += ROUND_UP_SIZE-1;
	priv->file_size -= (priv->file_size % ROUND_UP_SIZE);

	padding = priv->offset_image - priv->offset_pal - priv->num_cols*3;
	
	DPRINT("stride=0x%x padding=0x%x "
		"offset_image=0x%x file_size=0x%x", priv->fb_stride, 
		padding, priv->offset_image, priv->file_size);
		

	/* Write initial file contents 
	 */
	write_string(vis, (const unsigned char*)"\020GGIFILE");	/* magic */
	write_word(vis, (unsigned)LIBGGI_VIRTX(vis));	/* width */
	write_word(vis, (unsigned)LIBGGI_VIRTY(vis));	/* height */

	/* graphtype */
	write_byte(vis, (gt & GT_SCHEME_MASK) >> GT_SCHEME_SHIFT);
	write_byte(vis, (gt & GT_SUBSCHEME_MASK) >> GT_SUBSCHEME_SHIFT);
	write_byte(vis, (gt & GT_SIZE_MASK)  >> GT_SIZE_SHIFT);
	write_byte(vis, (gt & GT_DEPTH_MASK) >> GT_DEPTH_SHIFT);

	write_word(vis, (unsigned)priv->fb_stride);		/* stride */
	write_word(vis, (unsigned)padding);		/* padsize */

	write_zeros(vis, priv->num_cols * 3);	/* palette */
	write_zeros(vis, padding);		/* padding */

	/* image */
	write_zeros(vis, priv->file_size - priv->offset_image);
	write_flush(vis);


	/* Map the file into memory
	 */
	priv->file_mmap = (uint8_t *) mmap(0, (size_t)priv->file_size, 
		PROT_READ | PROT_WRITE, MAP_SHARED, LIBGGI_FD(vis), 0);

	DPRINT("File mmap'd at 0x%x.\n", priv->file_mmap);

	if (priv->file_mmap == MAP_FAILED) {
		perror("display-file: mmap failed");
		close(LIBGGI_FD(vis));
		return GGI_ENODEVICE;
	}

	priv->fb_ptr = priv->file_mmap + priv->offset_image;

	return 0;
}

static int _ggi_getmmap(struct ggi_visual *vis)
{
	int rc = GGI_OK;
	ggi_file_priv *priv = FILE_PRIV(vis);

	ggi_graphtype gt = LIBGGI_GT(vis);


	priv->fb_stride = GT_ByPPP(LIBGGI_VIRTX(vis), gt);
	priv->fb_size = LIBGGI_FB_SIZE(LIBGGI_MODE(vis));

	if (GT_SCHEME(gt) == GT_PALETTE) {
		priv->num_cols = 1 << GT_DEPTH(gt);
	} else {
		priv->num_cols = 0;
	}


	/* create file */
	
	rc = _ggi_file_create_file(vis, priv->filename);
	if (rc < 0) {
		return rc;
	}


	/* map the file into memory */

	if (priv->flags & FILEFLAG_RAW) {
		rc = _ggi_rawstuff(vis);
		if (rc < 0) {
			return rc;
		}
	} else {
		priv->fb_ptr = malloc((size_t)priv->fb_size);

		if (priv->fb_ptr == NULL) {
			DPRINT_MODE("Out of memory!");
			return GGI_ENOMEM;
		}
	}

	/* set up pixel format */
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), gt);
	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	/* set up DirectBuffer */
	_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());

	LIBGGI_APPBUFS(vis)[0]->frame = 0;
	LIBGGI_APPBUFS(vis)[0]->type  = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
	LIBGGI_APPBUFS(vis)[0]->read  = priv->fb_ptr;
	LIBGGI_APPBUFS(vis)[0]->write = priv->fb_ptr;
	LIBGGI_APPBUFS(vis)[0]->layout = blPixelLinearBuffer;
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.stride = priv->fb_stride;
	LIBGGI_APPBUFS(vis)[0]->buffer.plb.pixelformat = LIBGGI_PIXFMT(vis);

	/* Set up palette */
	if (LIBGGI_PAL(vis)->clut.data) {
 		free(LIBGGI_PAL(vis)->clut.data);
 		LIBGGI_PAL(vis)->clut.data = NULL;
	}
	
	if (GT_SCHEME(gt) == GT_PALETTE) {
		LIBGGI_PAL(vis)->clut.data = _ggi_malloc(sizeof(ggi_color) * priv->num_cols);
		LIBGGI_PAL(vis)->clut.size = priv->num_cols;
	}
	
	return 0;
}

int GGI_file_getapi(struct ggi_visual *vis,int num, char *apiname ,char *arguments)
{
	ggi_graphtype gt = LIBGGI_GT(vis);

	*arguments = '\0';

	switch(num) { 

	case 0: strcpy(apiname, "display-file");
		return 0;

	case 1: strcpy(apiname, "generic-stubs");
		return 0;
		
	case 2: if (GT_SCHEME(gt) == GT_TEXT) {
			sprintf(apiname, "generic-text-%u", GT_SIZE(gt));
			return 0;
		}

		sprintf(apiname, "generic-linear-%u%s", GT_SIZE(gt),
			(gt & GT_SUB_HIGHBIT_RIGHT) ? "-r" : "");
		return 0;

	case 3: if (GT_SCHEME(gt) == GT_TEXT) {
			return GGI_ENOMATCH;
		}

		strcpy(apiname, "generic-color");
		return 0;
	}

	return GGI_ENOMATCH;
}

static void _ggi_freedbs(struct ggi_visual *vis) 
{
	int i;

	for (i=LIBGGI_APPLIST(vis)->num-1; i >= 0; i--) {
		_ggi_db_free(LIBGGI_APPBUFS(vis)[i]);
		_ggi_db_del_buffer(LIBGGI_APPLIST(vis), i);
	}
}

static int _ggi_domode(struct ggi_visual *vis)
{
	int err, i;
	char name[GGI_MAX_APILEN];
	char args[GGI_MAX_APILEN];

	_ggiZapMode(vis, 0);
	_ggi_freedbs(vis);

	DPRINT("_ggi_domode: zapped\n");

	if ((err = _ggi_getmmap(vis)) != 0) {
		return err;
	}

	DPRINT("_ggi_domode: got mmap\n");

	for(i=1; GGI_file_getapi(vis, i, name, args) == 0; i++) {
		err = _ggiOpenDL(vis, _ggiGetConfigHandle(),
				name, args, NULL);
		if (err) {
			fprintf(stderr,"display-file: Can't open the "
				"%s (%s) library.\n", name, args);
			return GGI_EFATAL;
		} else {
			DPRINT_LIBS("Success in loading "
				       "%s (%s)\n", name, args);
		}
	}

	if (GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE) {
		LIBGGI_PAL(vis)->setPalette = GGI_file_setPalette;
	}
	vis->opdisplay->flush = GGI_file_flush;
	
	return 0;
}

int GGI_file_setmode(struct ggi_visual *vis, ggi_mode *mode)
{ 
	ggi_file_priv *priv = FILE_PRIV(vis);
	int err;

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}
	
	DPRINT_MODE("setmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	if ((err = ggiCheckMode(vis->instance.stem, mode)) != 0) {
		return err;
	}

	*LIBGGI_MODE(vis) = *mode;

	err = _ggi_domode(vis);

	if (err) {
		DPRINT("domode failed (%d)\n",err);
		return err;
	}

	ggiIndicateChange(vis->instance.stem, GGI_CHG_APILIST);
	DPRINT("change indicated\n",err);

	priv->flushtotal = 0;
	priv->mode_reset = 1;	/* signal the writer that the mode was reset. */

	gettimeofday(&priv->flushlast,NULL);
	return 0;
}

int GGI_file_resetmode(struct ggi_visual *vis)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	DPRINT("GGIresetmode(%p)\n", vis);

	if (priv->flags & FILEFLAG_RAW) {
		munmap((void *)priv->file_mmap, (unsigned)priv->file_size);
	} else {
		/* This is where we write the non-raw file */

       		_ggi_file_rewind(vis);
		(* priv->writer)(vis);

		free((void *)priv->fb_ptr);
	}

	priv->file_mmap = priv->fb_ptr = NULL;

	_ggi_freedbs(vis);
	_ggi_file_close_file(vis);

	return 0;
}

int GGI_file_checkmode(struct ggi_visual *vis, ggi_mode *mode)
{
	/* ggi_file_priv *priv = FILE_PRIV(vis); */
	int err = 0;

	DPRINT_MODE("checkmode %dx%d#%dx%dF%d[0x%02x]\n",
			mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	/* handle GT_AUTO and GGI_AUTO */
	_GGIhandle_ggiauto(mode, 320, 200);

	mode->graphtype = _GGIhandle_gtauto(mode->graphtype);

	/* do some checks */
	if (GT_SIZE(mode->graphtype) < 8) {
		int align = 8 / GT_SIZE(mode->graphtype);

		if (mode->visible.x % align != 0) {
			mode->visible.x += align-(mode->visible.x % align);
			err = -1;
		}
		
		if (mode->virt.x % align != 0) {
			mode->virt.x += align-(mode->virt.x % align);
			err = -1;
		}
	}
	
	if (mode->virt.x < mode->visible.x) {
		mode->virt.x = mode->visible.x;
		err = -1;
	}

	if (mode->virt.y < mode->visible.y) {
		mode->virt.y = mode->visible.y;
		err = -1;
	}

	if (mode->frames != 1 && mode->frames != GGI_AUTO) {
		err = -1;
	}
	mode->frames = 1;

	if ((mode->dpp.x != 1 && mode->dpp.x != GGI_AUTO) ||
	    (mode->dpp.y != 1 && mode->dpp.y != GGI_AUTO)) {
		err = -1;
	}
	mode->dpp.x = mode->dpp.y = 1;

	if (mode->size.x != GGI_AUTO || mode->size.y != GGI_AUTO) {
		err = -1;
	}
	mode->size.x = mode->size.y = GGI_AUTO;

	DPRINT_MODE("result %d %dx%d#%dx%dF%d[0x%02x]\n",
			err, mode->visible.x, mode->visible.y,
			mode->virt.x, mode->virt.y, 
			mode->frames, mode->graphtype);

	return err;	
}

int GGI_file_getmode(struct ggi_visual *vis, ggi_mode *mode)
{
	DPRINT("GGIgetmode(%p,%p)\n", vis, mode);

	if (vis==NULL || mode==NULL || LIBGGI_MODE(vis)==NULL) {
		return GGI_EARGINVAL;
	}

	memcpy(mode, LIBGGI_MODE(vis), sizeof(ggi_mode));

	return 0;
}

int GGI_file_setflags(struct ggi_visual *vis, uint32_t flags)
{
	LIBGGI_FLAGS(vis) = flags;

	LIBGGI_FLAGS(vis) &= GGIFLAG_ASYNC; /* Unkown flags don't take. */

	return 0;
}

int GGI_file_setPalette(struct ggi_visual *vis, size_t start, size_t size, const ggi_color *colormap)
{
 	ggi_file_priv   *priv     = FILE_PRIV(vis);
 	uint8_t         *file_pal = priv->file_mmap + priv->offset_pal;
 	ggi_color       *dest     = LIBGGI_PAL(vis)->clut.data + start;
 	const ggi_color *src      = colormap;	

	DPRINT("setpalette.\n");
    
	file_pal += start * 3;

 	for (; start<size; ++start, ++src, ++dest) {
		*dest = *src;
		
		if (priv->flags & FILEFLAG_RAW) {
			*(file_pal++) = dest->r >> 8;
 			*(file_pal++) = dest->g >> 8;
 			*(file_pal++) = dest->b >> 8;
		}
	}

	return 0;
}
