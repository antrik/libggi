/* $Id: mode.c,v 1.1 2001/05/12 23:02:08 cegger Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#include <ggi/internal/ggi-dl.h>

#include <kgi/ioctl.h>

#ifndef MAP_FAILED
#define MAP_FAILED ((void*)-1)
#endif


int GGIkgicommand(ggi_visual *vis,int cmd,void *args)
{
	return ioctl(LIBGGI_FD(vis),cmd,args);
}


int GGIsetorigin(ggi_visual *vis,int x,int y)
{
	ggi_coord where;

	int err;

	CHECKXY(vis,x,y);

	where.x=x;
	where.y=y;

	if ((err=GGIkgicommand(vis,CHIP_SETVISFRAME,&where) != 0)
		return err;

	vis->origin_x=x;
	vis->origin_y=y;
	
	return 0;
}

int GGIsetsplitline(ggi_visual *vis,int y)
{
	if (y<0 || y>LIBGGI_MODE(vis)->visible.y) return -1;

	return GGIkgicommand(vis,CHIP_SETSPLITLINE,(void *)y);
}

/*
 * _Attempt_ to get the default framebuffer.. 
 */
static void _GGIgetmmio(ggi_visual *vis)
{
	int size=LIBGGI_MODE(vis)->virt.x*LIBGGI_MODE(vis)->virt.y;

	if (LIBGGI_FB_LINEAR(vis)!=NULL) {
		munmap(LIBGGI_FB_LINEAR(vis),LIBGGI_FB_LINEAR_SIZE(vis));
		LIBGGI_FB_LINEAR(vis)=NULL;
		LIBGGI_FB_LINEAR_SIZE(vis)=0;
	}

	size=_ggiSetupMode(vis);
	GGIDPRINT("Calculated size=%d bytes\n",size);

	if (size <= 0) 
		return;

	LIBGGI_FB_LINEAR_SIZE(vis)=size;	
	LIBGGI_FB_LINEAR(vis)=mmap(NULL,
                     size,
                     PROT_READ|PROT_WRITE,
                     MAP_SHARED,
                     LIBGGI_FD(vis),
	             MMAP_TYPE_MMIO|MMAP_PER_REGION_TYPE|MMAP_FRAMEBUFFER);

	GGIDPRINT("Linear FB=%p\n",LIBGGI_FB_LINEAR(vis));
	if (LIBGGI_FB_LINEAR(vis) == MAP_FAILED) {
		LIBGGI_FB_LINEAR(vis)=NULL;
		LIBGGI_FB_LINEAR_SIZE(vis)=0;
	}
}

static int _GGIdomode(ggi_visual *vis)
{
	int err;
	ggi_suggest sug;

	_GGIgetmmio(vis);

	err=(_ggiOpenDL(vis,"generic-stubs","")==NULL);
	if (err) {
		fprintf(stderr,"display-KGI: Can't load the \"generic-stubs\" "
			       "library\n");
	}
		
	vis->opdraw->setorigin=GGIsetorigin;

	sug.id=0;
	do {
		err=GGIkgicommand(vis,GRAPHICS_GETSUGGEST,&sug);
		if (err!=0) {
			fprintf(stderr,"display-KGI: Failed getting suggestion %d\n",
				       sug.id);
			perror("display-KGI");
			return -1;	/* Error */
		}

		GGIDPRINT("display-KGI - attempting %s (%s)\n",sug.name,sug.args);
		err=(_ggiOpenDL(vis,sug.name,sug.args)==NULL);
		if (err) {
			fprintf(stderr,"display-KGI: Can't find an appropriate "
				       "library for %s (%s)\n",
					sug.name,sug.args);
			return err;
		} else {
			GGIDPRINT("Success in loading %s (%s)\n",sug.name,sug.args);
		}
	} while (sug.id!=0);

	return 0;
}

int GGIsetmode(ggi_visual *vis,ggi_mode *tm)
{ 
	int err;

	if (vis==NULL) {
		GGIDPRINT("Visual==NULL\n");
		return -1;
	}
	
	/* Temporary */
	if (LIBGGI_FB_LINEAR(vis)!=NULL) {	/* Unmap mem - it might be invalid after mode-change */
		munmap(LIBGGI_FB_LINEAR(vis),LIBGGI_FB_LINEAR_SIZE(vis));
		LIBGGI_FB_LINEAR(vis)=NULL;
	}
	_ggiZapMode(vis, 0);

	GGIkgicommand(vis,DRIVER_RELEASEMODE,NULL);
	err=GGIkgicommand(vis,DRIVER_SETMODE,tm);
	if (err) {
		GGIDPRINT("%d=GGIkgicommand(%d,DRIVER_SETMODE,%p)\n",err,LIBGGI_FD(vis),tm);
		return err;
	}
	memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));

	return _GGIdomode(vis);
}

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGIcheckmode(ggi_visual *vis,ggi_mode *tm)
{
	if (vis==NULL)
		return -1;
	
	return GGIkgicommand(vis,DRIVER_TESTMODE,tm);
}

/************************/
/* get the current mode */
/************************/
int GGIgetmode(ggi_visual *vis,ggi_mode *tm)
{
	GGIDPRINT("In GGIgetmode(%p,%p)\n",vis,tm);
	if (vis==NULL)
		return -1;
	
	return GGIkgicommand(vis,DRIVER_GETMODE,tm);
}

/*************************/
/* set the current flags */
/*************************/
int GGIsetflags(ggi_visual *vis,ggi_flags flags)
{
	LIBGGI_FLAGS(vis)=flags;
	return 0;
}
