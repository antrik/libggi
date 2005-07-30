/* $Id: window.c,v 1.3 2005/07/30 08:43:03 soyt Exp $
******************************************************************************

   Universal window for LibGGI

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************
*/

#include "config.h"
#include "window.h"
#include "ggicolors.h"

static struct window _defaultwindow = {
	NULL,		/* no reasonable default visual possible */
	10,10,200,150,	/* small window should fit everywhere */
	GGI_BLACK,	/* background */
	2, GGI_WHITE,	/* border */
	"title", GGI_RED
};

int default_window(struct window *w, ggi_visual_t vis)
{
	*w = _defaultwindow;
	w->vis = vis;
	return 0;
}

int draw_window(struct window * w)
{
	int i;
	/* clear background */
	ggiSetGCForeground(w->vis, ggiMapColor(w->vis,&w->backgroundcolor));
	ggiDrawBox(w->vis, w->xorigin, w->yorigin, w->xsize, w->ysize);

	/* draw border */
	ggiSetGCForeground(w->vis, ggiMapColor(w->vis,&w->bordercolor));
	for (i = w->borderwidth; i > 0; i--){
		/*printf("border %d\n", i);*/
		ggiDrawHLine(w->vis, w->xorigin, w->yorigin+i-1, w->xsize);
		ggiDrawHLine(w->vis, w->xorigin, w->yorigin+w->ysize-i,
			     w->xsize);
		ggiDrawVLine(w->vis, w->xorigin+i-1, w->yorigin, w->ysize);
		ggiDrawVLine(w->vis, w->xorigin+w->xsize-i, w->yorigin,
			     w->ysize);
	}

	ggiSetGCForeground(w->vis, ggiMapColor(w->vis,&w->titlecolor));
	ggiSetGCBackground(w->vis, ggiMapColor(w->vis,&w->backgroundcolor));
	
	ggiPuts(w->vis, w->xorigin+10, w->yorigin+/*2*/0, w->title);
	return 0;
}



