/* $Id: mode.c,v 1.2 2002/07/29 15:45:31 fspacek Exp $
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

#include <ggi/display/kgi.h>

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


/*
int GGI_kgi_set_display_frame(ggi_visual *vis, int num)
{
	GGIDPRINT("setting display frame %d\n", num);

	kgiSetDisplayOrigin(&KGI_CTX(vis), 0, num*LIBGGI_MODE(vis)->visible.y);

	vis->d_frame_num = num;
	
	return 0;
}
*/

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


int GGI_kgi_setmode(ggi_visual *vis, ggi_mode *tm)
{
	const kgic_mapper_resource_info_result_t *fb;
	int id, i;
	char sugname[256], args[256];
	int err;
	kgi_u8_t *fb_ptr;

	if (vis == NULL) {
		GGIDPRINT("Visual==NULL\n");
		return -1;
	}

	err = GGI_kgi_checkmode(vis, tm);
	if(err)
		return err;

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
	
	if(KGI_PRIV(vis)->fb)
		munmap(KGI_PRIV(vis)->fb, KGI_PRIV(vis)->fb_size);

	fb = kgiGetResource(&KGI_CTX(vis), 0, KGI_RT_MMIO);
	if (! fb) {
		GGIDPRINT_LIBS("No framebuffer resource available");
		return -1;
	}

	GGIDPRINT("Found fb as resource %d", fb->resource);

	kgiSetupMmapFB(&KGI_CTX(vis), fb->resource);

	KGI_PRIV(vis)->fb_size = fb->info.mmio.size;
	KGI_PRIV(vis)->fb = mmap(NULL, KGI_PRIV(vis)->fb_size,
				 PROT_READ | PROT_WRITE, MAP_SHARED,
				 KGI_CTX(vis).mapper.fd, 0);

	if (KGI_PRIV(vis)->fb == MAP_FAILED){
		GGIDPRINT_LIBS("Unable to map the frambuffer\n");
		return -1;
	}

	*LIBGGI_MODE(vis) = *tm;
	
	memset(LIBGGI_PIXFMT(vis), 0, sizeof(ggi_pixelformat));
	setup_pixfmt(LIBGGI_PIXFMT(vis), tm->graphtype);

	GGIDPRINT_LIBS("Pixelformat set\n");

	fb_ptr = KGI_PRIV(vis)->fb;
	for (i = 0; i < tm->frames; ++i) {
	
		_ggi_db_add_buffer(LIBGGI_APPLIST(vis), _ggi_db_get_new());
		LIBGGI_APPBUFS(vis)[i]->frame = i;
		LIBGGI_APPBUFS(vis)[i]->type = GGI_DB_NORMAL | GGI_DB_SIMPLE_PLB;
		LIBGGI_APPBUFS(vis)[i]->read = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->write = fb_ptr;
		LIBGGI_APPBUFS(vis)[i]->layout = blPixelLinearBuffer;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride = 
			(GT_SIZE(tm->graphtype)*tm->virt.x + 7) / 8;
		LIBGGI_APPBUFS(vis)[i]->buffer.plb.pixelformat = 
			LIBGGI_PIXFMT(vis);
		
		fb_ptr += LIBGGI_APPBUFS(vis)[i]->buffer.plb.stride*tm->virt.y;
	}

	_ggi_build_pixfmt(LIBGGI_PIXFMT(vis));

	_ggiZapMode(vis, 0);

	for(id = 1; 0 == GGI_kgi_getapi(vis, id, sugname, args); ++id){
		if(_ggiOpenDL(vis, sugname, args, NULL)){
			GGIDPRINT_LIBS("kgi: can't open %s\n", sugname);
			
			if (id < 4)
				return GGI_EFATAL;
			else
				continue;
		}
		GGIDPRINT_LIBS("kgi: loaded %s\n", sugname);
	}

	if(GT_SCHEME(LIBGGI_GT(vis)) == GT_PALETTE){
		vis->palette = _ggi_malloc(sizeof(ggi_color) * 256);
		vis->opcolor->setpalvec = GGI_kgi_setpalvec;
	}

	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_kgi_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	kgi_image_mode_t mode;

	if (vis == NULL)
		return -1;

	memset(&mode, 0, sizeof(kgi_image_mode_t));

	/* Take care of automatic graphtype selection */
	tm->graphtype = _GGIhandle_gtauto(tm->graphtype);

	switch (GT_SCHEME(tm->graphtype)) {
	
	case GT_TEXT:
		mode.flags |= KGI_IF_TEXT16;
		break;
		
	case GT_TRUECOLOR:
		mode.fam |= KGI_AM_COLORS;
		mode.bpfa[0] = GT_DEPTH(tm->graphtype) / 3;
		mode.bpfa[1] = GT_DEPTH(tm->graphtype) / 3 +
			       GT_DEPTH(tm->graphtype) % 3;
		mode.bpfa[2] = GT_DEPTH(tm->graphtype) / 3;
		mode.bpfa[3] = GT_SIZE(tm->graphtype) - GT_DEPTH(tm->graphtype);
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
	mode.virt.y = tm->virt.y;

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
	GGI_kgi_getmode(vis, tm);
	
	return 0;
}

/************************/
/* get the current mode */
/************************/
int GGI_kgi_getmode(ggi_visual *vis, ggi_mode *tm)
{
	kgi_image_mode_t mode;
	
	GGIDPRINT("In GGI_kgi_getmode(%p,%p)\n",vis,tm);
	if (vis == NULL)
		return -1;

	memset(&mode, 0, sizeof(kgi_image_mode_t));
	kgiGetImageMode(&KGI_CTX(vis), 0, &mode);
	kgiPrintImageMode(&mode);

	tm->graphtype = image_mode_to_graphtype(&mode);
	tm->frames = 1;
	tm->visible.x = mode.size.x;
	tm->visible.y = mode.size.y;
	tm->virt.x = mode.virt.x;
	tm->virt.y = mode.virt.y;
	tm->size.x = GGI_AUTO;
	tm->size.y = GGI_AUTO;
	tm->dpp.x = 1;
	tm->dpp.y = 1;
	
	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_kgi_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;
	return 0;
}
