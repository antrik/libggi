/* $Id: visual.c,v 1.2 2002/09/08 21:37:46 soyt Exp $
******************************************************************************

   Display-SUID: initialization

   Copyright (C) 1995,1998 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan      [jmcc@ggi-project.org]

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
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/linvtsw.h>

#include <fcntl.h>
#include <signal.h>

#include <kgi/kgi_commands.h>

#include "suidhook.h"

int _suidtarget_dev_mem=-1;

extern struct kgi_device  mydevice;

static ggi_visual *_vis;  /* unix signal handlers suck ass */

static void get_killed(int signum)
{
	fprintf(stderr, "SIGNAL %d OCCURRED\n", signum);
	if (_vis) {
		GGIdlcleanup(_vis);
		_vis = NULL;
	}
	raise(9);
	raise(signum);
}


static int
GGI_suidkgi_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	/* Mainly here to stop those annoying '_default_error' messages
	 * in the debugging output.
	 */
	return 0;
}

extern ggi_pixel law_base;

int suidkgi_init_module(void);

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32 *dlret)
{
	ggi_mode mode;
	void *memptr=NULL;
  	suid_hook *priv;
	int x, err = GGI_ENOMEM;

	LIBGGI_GC(vis) = malloc(sizeof(ggi_gc));
	if (LIBGGI_GC(vis) == NULL) {	
		return GGI_ENOMEM;
	}

	priv = LIBGGI_PRIVATE(vis) = malloc(sizeof(suid_hook));
	if (priv == NULL) {
		goto out_freegc;
	}
  	
	priv->is_up=0;
	priv->dev_mem=_suidtarget_dev_mem=-1;
	priv->mmap_length=0;

	/* Open GII for input */
	vis->input = giiOpen("input-linux-kbd", NULL);
	if (vis->input == NULL) {
		fprintf(stderr, "display-suidkgi: Couldn't open kbd.\n");
		goto out_freepriv;
	}

	/* Has mode management */
	vis->opdisplay->getmode=GGI_suidkgi_getmode;
	vis->opdisplay->setmode=GGI_suidkgi_setmode;
	vis->opdisplay->checkmode=GGI_suidkgi_checkmode;
	vis->opdisplay->flush=GGI_suidkgi_flush;
	vis->opdisplay->kgicommand=GGI_suidkgi_kgicommand;
	vis->opdisplay->setflags=GGI_suidkgi_setflags;
	vis->opdraw->setorigin=GGI_suidkgi_setorigin;

	vis->w_frame = malloc(sizeof(ggi_directbuffer)); /* FIXME ! */
	vis->r_frame = malloc(sizeof(ggi_directbuffer)); /* FIXME ! */
	LIBGGI_CURWRITE(vis)=NULL;
	LIBGGI_CURREAD(vis)=NULL;

	if (iopl(3)) {perror("iopl");exit(2); }
	GGIDPRINT("IOPL is here.\n");
  
	if (-1 == (priv->dev_mem = _suidtarget_dev_mem = open("/dev/mem",O_RDWR))) { perror("opening /dev/mem");exit(3); }
	memptr=mmap(NULL,64*1024,PROT_READ|PROT_WRITE,MAP_SHARED,priv->dev_mem,0xa0000);
	GGIDPRINT("Have mmap at %p.\n",memptr);

/*	law_base=0xf3000000; */
	if (suidkgi_init_module()) {GGIDPRINT("Init has failed. Tough luck.\n");exit(1); }

	GGIDPRINT("Init was o.k.\n");
	signal(SIGSEGV,get_killed);
	signal(SIGINT,get_killed);
	signal(SIGTERM,get_killed);
	priv->is_up=1;

#if 1
	/* Not sure, if this is needed, but ... */
	mode.frames=1;
	mode.graphtype=GT_TEXT16;
	mode.visible.x=mode.virt.x=80;
	mode.visible.y=mode.virt.y=25;
	mode.dpp.x = 8;mode.dpp.y =16;
	x=GGI_suidkgi_checkmode(vis,&mode);
	GGIDPRINT("TESTMODE1 says %d.\n",x);
	x=GGI_suidkgi_setmode(vis,&mode);
	GGIDPRINT("SETMODE1 says %d.\n",x);
#endif

#undef TESTING_THE_SUID_TARGET
#ifdef TESTING_THE_SUID_TARGET

#ifdef GoneByeBye
	for(x=0;x<480;x++)
	{
		GGI_suidkgi_setsplitline(vis, x);
		ggUSleep(100000);
	}
#endif

	mode.frames=1;
	mode.graphtype=GT_8BIT;
	mode.visible.x=mode.virt.x=320;
	mode.visible.y=mode.virt.y=200;
	mode.dpp.x    =mode.dpp.y =1;
	x=ggiCheckMode(vis,&mode);
	GGIDPRINT("TESTMODE3 says %d.\n",x);
	x=ggiSetMode(vis,&mode);
	GGIDPRINT("SETMODE3 says %d.\n",x);

	sleep(1);

	{ int y;
		for(y=0;y<200;y++)
		{ 
			for(x=0;x<320;x++)
			{ 
				*((unsigned char *)memptr+x+y*320)=x+y; 
			}
			ggUSleep(1);
		}
	}
	sleep(1);

	mode.frames=1;
	mode.graphtype=GT_TEXT16;
	mode.visible.x=mode.virt.x=80;
	mode.visible.y=mode.virt.y=25;
	mode.dpp.x = 8;mode.dpp.y =16;
	x=GGI_suidkgi_checkmode(vis,&mode);
	GGIDPRINT("TESTMODE4 says %d.\n",x);
	x=GGI_suidkgi_setmode(vis,&mode);
	GGIDPRINT("SETMODE4 says %d.\n",x);

	sleep(1);

	suidkgi_cleanup_module();

	GGIDPRINT("Cleanup went well.\n");
	close(priv->dev_mem);
	exit(1);
#endif

	*dlret = GGI_DL_OPDISPLAY|GGI_DL_OPDRAW;
	return 0;

  out_freepriv:
	free(priv);
  out_freegc:
	free(LIBGGI_GC(vis);

	return err;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
  	suid_hook *priv = SUIDHOOK(vis);
	ggi_mode mode;
	int x;
	
	if (! priv) {
		return -1;
	}

	if (priv->is_up) {
#if 1
		/* Not sure, if this is needed, but ... */
		mode.frames=1;
		mode.graphtype=GT_TEXT16;
		mode.visible.x=mode.virt.x=80;
		mode.visible.y=mode.virt.y=25;
		mode.dpp.x = 9;mode.dpp.y =14;
		x=GGI_suidkgi_checkmode(vis,&mode);
		GGIDPRINT("TESTMODE1 says %d.\n",x);
		x=GGI_suidkgi_setmode(vis,&mode);
		GGIDPRINT("SETMODE1 says %d.\n",x);
#endif
		
/*		suidkgi_cleanup_module(); */

		if (priv->vt_fd >= 0) {
			vtswitch_close(vis);
			priv->vt_fd = -1;
		}

		GGIDPRINT("Cleanup went well.\n");
		close(priv->dev_mem);
		priv->is_up=0;
	}

	if (vis->input) {
		giiClose(vis->input);
		vis->input = NULL;
	}
	
	free(priv);

	return 0;
}
		

int GGIdl_suidkgi(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
