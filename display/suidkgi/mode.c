/* $Id: mode.c,v 1.2 2004/09/13 08:50:19 cegger Exp $
******************************************************************************

   Display-SUID

   Copyright (C) 1998 Andreas Beck ??? FIXME   [becka@ggi-project.org]

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

#include "suidhook.h"

#include <signal.h>


int GGI_suidkgi_kgicommand(ggi_visual *vis,int cmd,void *args)
{
	return graph_ioctl(cmd,args);
}

int GGI_suidkgi_setorigin(ggi_visual *vis,int x,int y)
{
	ggi_coord where;
	int err;

	CHECKXY(vis,x,y);

	where.x=x;
	where.y=y;

	GGIDPRINT("GGIsetorigin:\n");

	if ((err=GGI_suidkgi_kgicommand(vis,CHIP_SETVISFRAME,&where)) != 0) {
		return err;
	}

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}

#if 0
int GGI_suidkgi_setsplitline(ggi_visual *vis,int y)
{
	if (y<0 || y > LIBGGI_Y(vis)) return -1;

	GGIDPRINT("GGIsetsplitline:\n");
	return GGI_suidkgi_kgicommand(vis,CHIP_SETSPLITLINE,y);
}
#endif

void get_primary_fb(long *sz,long *psz,long *pstart);

/*
 * _Attempt_ to get the default framebuffer.. 
 */
static void _GGIgetmmio(ggi_visual *vis)
{
	suid_hook *priv = SUIDHOOK(vis);

	long regsize,regpsize,regpstart;
	int size = LIBGGI_VIRTX(vis) * LIBGGI_VIRTY(vis);
	
	printf("Getmmio: %d %d\n",LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	if (LIBGGI_CURWRITE(vis) != NULL) {
		munmap(LIBGGI_CURWRITE(vis),priv->mmap_length);
		LIBGGI_CURWRITE(vis)=LIBGGI_CURREAD(vis)=NULL;
		priv->mmap_length=0;
	}

	size=64*1024;	/* FIXME ! Was _ggiSetupMode(vis); */
	GGIDPRINT("Calculated size=%d bytes\n",size);

	if (size <= 0) 
		return;

	priv->mmap_length=size;	

	/* Now we map the FB ... */
	get_primary_fb(&regsize,&regpsize,&regpstart);
	
	if (regpsize!=regsize)
		GGIDPRINT("Banked framebuffer (%x!=%x) - not implemented !\n",
		       regpsize,regsize);

	if (size>regsize)
		GGIDPRINT("Total framebuffer too small ??? (%x>%x)"
		       "Will probably fail.\n",
		       size,regsize);

	if (size>regpsize) {
		GGIDPRINT("Single bank too small. (%x>%x)"
		       "Cutting down - will probably fail.\n",
		       size,regpsize);
		size=regpsize;
	}
	
	/* FIXME !! */
	LIBGGI_CURWRITE(vis)=LIBGGI_CURREAD(vis)=mmap(NULL,
                     size,
                     PROT_READ|PROT_WRITE,
                     MAP_SHARED,
                     priv->dev_mem,
	             regpstart);
/*	             MMAP_TYPE_MMIO|MMAP_PER_REGION_TYPE|MMAP_FRAMEBUFFER); */

	LIBGGI_FB_R_STRIDE(vis) = LIBGGI_FB_W_STRIDE(vis) =
		GT_ByPPP(LIBGGI_VIRTX(vis), LIBGGI_GT(vis));

	GGIDPRINT("Linear FB=%p\n",LIBGGI_CURWRITE(vis));
	if (LIBGGI_CURWRITE(vis) == (void *)(-1)) {
		LIBGGI_CURWRITE(vis)=LIBGGI_CURREAD(vis)=NULL;
		priv->mmap_length=0;
	}
}

static int _GGIdomode(ggi_visual *vis)
{
	int err;
	kgi_suggest sug;

	_GGIgetmmio(vis);

	err = _ggiOpenDL(vis, "generic-stubs", "", NULL);
	if (err) {
		fprintf(stderr, "display-suidkgi: Can't load the \"generic-stubs\" library\n");
	}

	err = _ggiOpenDL(vis, "generic-color", "", NULL);
	if (err) {
		fprintf(stderr,"display-suidkgi: Can't load the \"generic-color\" library\n");
	}
		
/*	vis->opdraw->setsplitline=GGI_suidkgi_setsplitline;*/
	vis->opdraw->setorigin=GGI_suidkgi_setorigin;
	printf("domode: %d %d\n", LIBGGI_VIRTX(vis), LIBGGI_VIRTY(vis));

	sug.id=0;
	do {
		err=GGI_suidkgi_kgicommand(vis,ACCEL_GETSUGGEST,&sug);
		if (err!=0) {
			fprintf(stderr,"display-suidkgi: Failed getting suggestion %d err=%x\n",
				       sug.id,err);
			return -1;	/* Error */
		}

		GGIDPRINT("display-suid - attempting %s (%s)\n",sug.name,sug.args);
		err = _ggiOpenDL(vis, sug.name, sug.args, NULL);
		if (err) {
			fprintf(stderr,"display-suidkgi: Can't find an appropriate "
				       "library for %s (%s)\n",
					sug.name,sug.args);
			/* return err; */
		} else {
			GGIDPRINT("Success in loading %s (%s)\n",sug.name,sug.args);
		}
	} while (sug.id!=0);

	return 0;
}

static void mode_ggi2kgi(ggi_mode *gm, kgi_modereq *km)
{
	km->frames   =gm->frames;
	km->visible.x=gm->visible.x;
	km->visible.y=gm->visible.y;
	km->virt.x   =gm->virt.x;
	km->virt.y   =gm->virt.y;
	km->size.x   =gm->size.x;
	km->size.y   =gm->size.y;
	km->dpp.x    =gm->dpp.x;
	km->dpp.y    =gm->dpp.y;
	km->graphtype=0;
	switch(gm->graphtype)
	{
		case GT_TEXT16:
			km->graphtype=KGIGT_TEXT16;
			break;
		case GT_TEXT32:
			km->graphtype=KGIGT_TEXT32;
			break;
		case GT_1BIT:
			km->graphtype=KGIGT_1BIT;
			break;
		case GT_4BIT:
			km->graphtype=KGIGT_4BIT;
			break;
		case GT_8BIT:
			km->graphtype=KGIGT_8BIT;
			break;
		case GT_15BIT:
			km->graphtype=KGIGT_15BIT;
			break;
		case GT_16BIT:
			km->graphtype=KGIGT_16BIT;
			break;
		case GT_24BIT:
			km->graphtype=KGIGT_24BIT;
			break;
		case GT_32BIT:
			km->graphtype=KGIGT_32BIT;
			break;
		default:
			km->graphtype=KGIGT_8BIT;
			fprintf(stderr,"mode_ggi2kgi:Unknown graphtype\n");
			break;
	}
}

static void mode_kgi2ggi(kgi_modereq *km, ggi_mode *gm)
{
	gm->frames   =km->frames;
	gm->visible.x=km->visible.x;
	gm->visible.y=km->visible.y;
	gm->virt.x   =km->virt.x;
	gm->virt.y   =km->virt.y;
	gm->size.x   =km->size.x;
	gm->size.y   =km->size.y;
	gm->dpp.x    =km->dpp.x;
	gm->dpp.y    =km->dpp.y;
	gm->graphtype=0;
	switch(km->graphtype)
	{
		case KGIGT_TEXT16:
			gm->graphtype=GT_TEXT16;
			break;
		case KGIGT_TEXT32:
			gm->graphtype=GT_TEXT32;
			break;
		case KGIGT_1BIT:
			gm->graphtype=GT_1BIT;
			break;
		case KGIGT_4BIT:
			gm->graphtype=GT_4BIT;
			break;
		case KGIGT_8BIT:
			gm->graphtype=GT_8BIT;
			break;
		case KGIGT_15BIT:
			gm->graphtype=GT_15BIT;
			break;
		case KGIGT_16BIT:
			gm->graphtype=GT_16BIT;
			break;
		case KGIGT_24BIT:
			gm->graphtype=GT_24BIT;
			break;
		case KGIGT_32BIT:
			gm->graphtype=GT_32BIT;
			break;
		default:
			gm->graphtype=GT_8BIT;
			fprintf(stderr,"mode_kgi2ggi:Unknown graphtype\n");
			break;
	}
}

int GGI_suidkgi_setmode(ggi_visual *vis,ggi_mode *tm)
{ 
	int err;
	suid_hook *priv = SUIDHOOK(vis);
	
	kgi_modereq kgimode;
	
	mode_ggi2kgi(tm,&kgimode);

	if (vis==NULL) {
		GGIDPRINT("Visual==NULL\n");
		return -1;
	}
	
	/* Temporary */
	if (LIBGGI_CURWRITE(vis)!=NULL) {	/* Unmap mem - it might be invalid after mode-change */
		munmap(LIBGGI_CURWRITE(vis),priv->mmap_length);
		LIBGGI_CURWRITE(vis)=LIBGGI_CURREAD(vis)=NULL;
	}
	_ggiZapMode(vis, 0);


#if 0	/* What the hell was that ? */
	/* Setup new VT 
	*/
	priv->vt_fd = vtswitch_open(vis); if (priv->vt_fd < 0) {
		return -1;
	}
#endif

	GGIDPRINT_CORE("DRIVER_SETGRAPHMODE:\n");
	err=GGI_suidkgi_kgicommand(vis,DRIVER_SETGRAPHMODE,&kgimode);
	if (err) {
		GGIDPRINT("%d=GGI_suidkgi_kgicommand(vis,DRIVER_SETGRAPHMODE,%p)\n",err,tm);
		return err;
	}
	mode_kgi2ggi(&kgimode,tm);
	memcpy(LIBGGI_MODE(vis),tm,sizeof(*tm));

	return _GGIdomode(vis);
}

int GGI_suidkgi_checkmode(ggi_visual *vis,ggi_mode *tm)
{
	kgi_modereq km;
	int rc;
	
	if (vis==NULL)
		return -1;
	
	GGIDPRINT("DRIVER_TESTMODE:\n");
	mode_ggi2kgi(tm,&km);
	rc=GGI_suidkgi_kgicommand(vis,DRIVER_TESTMODE,&km);
	mode_kgi2ggi(&km,tm);
	return rc;
}

int GGI_suidkgi_getmode(ggi_visual *vis,ggi_mode *tm)
{
	kgi_modereq km;
	int rc;

	GGIDPRINT("In GGIgetmode(%p,%p)\n",vis,tm);
	if (vis==NULL)
		return -1;
	
	GGIDPRINT("DRIVER_GETGRAPHMODE:\n");
	rc=GGI_suidkgi_kgicommand(vis,DRIVER_GETGRAPHMODE,&km);
	mode_kgi2ggi(&km,tm);
	return rc;
}


int GGI_suidkgi_setflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis) = flags;

	return 0;
}


void handle_switched_away(ggi_visual *vis)
{
	/* Reset the graphic card into text mode, ready for console
	 * switching.  VT switching remains disabled during this
	 * routine.
	 */

#if 1
	raise(11 /*SIGSEGV*/);
#else
	!!! implement me
#endif
}


void handle_switched_back(ggi_visual *vis)
{
	/* Restore the graphic card to the mode it was in before
	 * switched away.  VT switching remains disabled during this
	 * routine.
	 */

#if 1
	raise(11 /*SIGSEGV*/);
#else
	!!! implement me
#endif
}
