/* $Id: mode.c,v 1.1 2003/02/13 19:44:25 fries Exp $
******************************************************************************

   LibGGI wsfb(3) target

   Copyright (C) 2003 Todd T. Fries <todd@openbsd.org>

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

#include <sys/ioccom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/wsfb.h>

int GGI_wsfb_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	//struct wsfb_priv *priv = WSFB_PRIV(vis);
	GGIDPRINT("GGI_wsfb_getapi called\n");

	switch(num) {
		case 0:
			strcpy(apiname, "display-wsfb");
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
			sprintf(apiname, "generic-linear-%d%s", 
				GT_SIZE(LIBGGI_GT(vis)),
				(LIBGGI_GT(vis) & GT_SUB_HIGHBIT_RIGHT) 
				? "-r" : "");
			strcpy(arguments, "");
			return 0;
		default:
			break;
	}
			
	return -1;
}

int GGI_wsfb_setmode(ggi_visual *vis, ggi_mode *tm)
{ 
	//struct wsfb_priv *priv = WSFB_PRIV(vis);
	ggi_graphtype gt = tm->graphtype;
	//unsigned long modenum = 0;
	//char sugname[256];
	//char args[256];
	int err = 0;
	//int id, i;
	int pixelBytes;

	GGIDPRINT_MODE("display-wsfb: setmode %dx%d#V%dx%d.F%d[0x%02x]\n",
		tm->visible.x, tm->visible.y,
		tm->virt.x, tm->virt.y,
		tm->frames, tm->graphtype);

	err = GGI_wsfb_checkmode(vis, tm);
	if (err)
		return err;

	switch(gt) {
		case GT_8BIT : pixelBytes = 1; break;
		/* Unsupported mode depths */
		default:
			return -1;
	}

//	ggiIndicateChange(vis, GGI_CHG_APILIST);

	return 0;
}

#define WANT_MODELIST
#include "../common/modelist.inc"

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_wsfb_checkmode(ggi_visual *vis, ggi_mode *tm)
{
	wsfb_priv *priv = WSFB_PRIV(vis);
	int err = 0;

	GGIDPRINT_MODE("display-wsfb: setmode %dx%d#V%dx%d.F%d[0x%02x]\n",
		tm->visible.x, tm->visible.y,
		tm->virt.x, tm->virt.y,
		tm->frames, tm->graphtype);

	if (vis==NULL || tm==NULL)
		return -1;

	tm->visible.x = tm->virt.x = priv->info.width;
	tm->visible.y = tm->virt.y = priv->info.height;

	tm->graphtype = GT_8BIT;
	
	if(tm->virt.x==GGI_AUTO) tm->virt.x = tm->visible.x;
	if(tm->virt.y==GGI_AUTO) tm->virt.y = tm->visible.y;

	/* Force virtual to equal visible */
	if(tm->virt.x != tm->visible.x) {
		tm->virt.x = tm->visible.x;
		err = -1;
	}
	if (tm->virt.y != tm->visible.y) {
		tm->virt.y = tm->visible.y;
		err = -1;
	}
 
	tm->frames = 0;

	return err;
}

/************************/
/* get the current mode */
/************************/
int GGI_wsfb_getmode(ggi_visual *vis,ggi_mode *tm)
{
	LIBGGI_APPASSERT(vis != NULL, "GGIgetmode(wsfb): Visual == NULL");

	GGIDPRINT("In GGIgetmode(%p,%p)\n",vis,tm);

	memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));
	return 0;
}
