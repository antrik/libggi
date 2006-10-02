/* $Id: menu.c,v 1.6 2006/10/02 09:40:51 cegger Exp $
******************************************************************************

   Universal menu for ggi

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
#include <ggi/gii.h>
#include <ggi/gii-keyboard.h>
#include <ggi/ggi.h>
#include <stdio.h>

#include "menu.h"
#include "ggitext.h"

static struct menu _defaultmenu = {
#ifdef setwindow
	{NULL,
	10,10,100,100,           /* size */
	{0,0,0},   /* background black */
	6,{0xffff,0xffff,0xffff}, /* white border */
	"notitle",{0xffff,0,0}},
#else
	{NULL },   /* Window */
#endif
	1,
	{{"firstentry"}},
	{0xffff,0xffff,0x0000}, /* entry */

	{0x0000,0x0000,0xffff}, /* selected fg */
        {0xffff,0xffff,0x0000},  /* selected bg */
	5,15,15,15,
	10,4,10,
	NULL,0,0,{0xffff,0xffff,0x0000},
	NULL,0,0,{0xffff,0xffff,0x0000},
};


int default_menu(struct menu * m, ggi_visual_t vis)
{
	*m = _defaultmenu;
	default_window(& m->w, vis);
	return 0;
}

int calculate_menu(struct menu * m)
{
/* Calculate size and positions */

	int i;

	int maxwidth, w;
	int h;

	m -> titlex = m->w.borderwidth +m->titleleftskip ; 
        /* reference point is upper left corner of window! */
	m -> titley = 0;


	m -> toptextx = m->w.borderwidth + m->entryleftskip;
	m -> toptexty = ggiGraphTextStringheight(m->w.vis, m->w.title) +
				m-> firstlineskip;
	
	/*printf("Title at x=%d, y=%d\n", m->titlex, m->titley);*/

	maxwidth = 0;

	for (i = 0;i<=m->lastentry;i++){
		/*printf("Entry %d .. \n", i);*/
		w = ggiGraphTextStringwidth(m->w.vis, m->entry[i].text);
		if ( w > maxwidth ){
			maxwidth = w;
		}


		m->entry[i].x= m->w.borderwidth+
			m->entryleftskip;

		if (i==0){
			if (m->toptext == NULL){
				m->entry[i].y=
					ggiGraphTextStringheight(m->w.vis, m->w.title) +
					m-> firstlineskip;
			} else {
				m->entry[i].y=
					ggiGraphTextStringheight(m->w.vis, m->toptext) +
					m-> firstlineskip+
					m-> toptexty;
			}  
		} else {
			m->entry[i].y = m->entry[i-1].y + 
				ggiGraphTextStringheight(m->w.vis, m->entry[i-1].text) + 
				m->entrylineskip;
		}

	}
	/*printf("done\n");*/
	m -> bottomtextx = m->w.borderwidth + m->entryleftskip;
	m -> bottomtexty = m->entry[m->lastentry].y +
		ggiGraphTextStringheight(m->w.vis, m->entry[m->lastentry].text) +
		m-> firstlineskip;
	
	/* does the header fit? */
	w = 2*m->w.borderwidth + ggiGraphTextStringwidth(m->w.vis, m->w.title) +
		m->titleleftskip + m-> titlerightskipmin;
	if (w > m-> w.xsize) {
		m-> w.xsize = w;
	}
	
	/* do the entries fit? */
	w = 2*m->w.borderwidth + maxwidth +
		m->entryleftskip + m-> entryrightskipmin;
	if (w > m-> w.xsize) {
		m-> w.xsize = w;
	}
	/* top- and bottomtext */
	if (m->toptext != NULL) {
		w = 2 * m->w.borderwidth + 
			ggiGraphTextStringwidth(m->w.vis, m->toptext) +
			m->entryleftskip + 
			m-> entryrightskipmin;
		if (w > m-> w.xsize) {
			m-> w.xsize = w;
		}
	}
	if (m->bottomtext != NULL) {
		w = 2 * m->w.borderwidth + 
			ggiGraphTextStringwidth(m->w.vis, m->bottomtext) +
			m->entryleftskip + 
			m-> entryrightskipmin;
		if (w > m-> w.xsize) {
			m-> w.xsize = w;
		}
	}
		
	if (m->bottomtext == NULL) {
		h = m->entry[m->lastentry].y + 
			ggiGraphTextStringheight(m->w.vis, m->entry[m->lastentry].text) +
			m->lastlineskip	+ m->w.borderwidth;
	} else {
		h = m->bottomtexty +
			ggiGraphTextStringheight(m->w.vis, m->bottomtext) +
			m->lastlineskip + m->w.borderwidth;
	}
	if (m-> w.ysize < h){
		m->w.ysize = h;
	}


	return 0;
}

int center_menu(struct menu * m)
{
	ggi_mode currmode;

	ggiGetMode(m->w.vis,&currmode);
	m->w.xorigin = (currmode.visible.x - m->w.xsize ) /2;
	m->w.yorigin = (currmode.visible.y - m->w.ysize ) /2;
	if ((m->w.xorigin < 0) || (m->w.yorigin < 0)){
		return -1;
	}
	return 0;
}

int do_menu(struct menu * m , int selected)
{

/* select a menu item by either a number or with arrow keys */

	int i;

	int evmask;
	gii_event ev;
	struct timeval t={0,0};
	int oldselection = selected;

	draw_window(&m->w);

	if (m->toptext != NULL) {
		ggiSetGCForeground(m->w.vis,
			   ggiMapColor(m->w.vis,&m->toptextcolor));
		/* FIXME*/
		ggiPuts(m->w.vis, m->w.xorigin+m->toptextx,
			m->w.yorigin+m->toptexty, m->toptext);
	}

	if (m->bottomtext != NULL){
		ggiSetGCForeground(m->w.vis,
				   ggiMapColor(m->w.vis,&m->bottomtextcolor));
		/* FIXME*/
		ggiPuts(m->w.vis, m->w.xorigin+m->bottomtextx,
			m->w.yorigin+m->bottomtexty, m->bottomtext);
	}

	for (i = 0;i<=m->lastentry;i++){
		if (i!=selected){
			ggiSetGCForeground(m->w.vis,
					   ggiMapColor(m->w.vis,&m->entrycolor));
			ggiSetGCBackground(m->w.vis,
					   ggiMapColor(m->w.vis,&m->w.backgroundcolor));
		} else {
			ggiSetGCForeground(m->w.vis,
					   ggiMapColor(m->w.vis,&m->selectedcolor));
			ggiSetGCBackground(m->w.vis,
					   ggiMapColor(m->w.vis,
						       &m->selectedbackgroundcolor));
		}
		ggiPuts( m->w.vis,
			 m->w.xorigin + m->entry[i].x,
			 m->w.yorigin + m->entry[i].y,
			 m->entry[i].text);
	}
	for (;;){
		/* if we are in asynchronous mode, we must guarantee */
		/* the user sees he's next.                          */
		ggiFlush(m->w.vis);   

		/* get a keypress */
		evmask = emKey;
		giiEventPoll(m->w.vis, evmask, NULL);
		while (giiEventPoll(m->w.vis, evmask,&t)){
			do {
				giiEventRead(m->w.vis,&ev, evmask);
			} while (!((1<<ev.any.type)&evmask));
			switch(ev.any.type){
			case evKeyPress:
			case evKeyRepeat:
				switch(ev.key.sym){
				case '1':selected = 0;
					break;
				case '2':selected = 1;
					break;
				case '3':selected = 2;
					break;
				case '4':selected = 3;
					break;
				case '5':selected = 4;
					break;
				case '6':selected = 5;
					break;
				case '7':selected = 6;
					break;
				case '8':selected = 7;
					break;
				case '9':selected = 8;
					break;
				case GIIK_Up:
					selected--;
					break;
				case GIIK_Down:
					selected++;
					break;
				case GIIK_Enter:
					ggiFlush(m->w.vis); 
					/* just to make sure */
					return (selected);
					break; /* never get here */
				case GIIUC_Escape:
					ggiFlush(m->w.vis); 
					/* just to make sure */
					return (-1);
				default: 
/*printf("unknown sym=%4x code=%4x\n", ev.key.sym, ev.key.code);*/
					break;
				}
			default: /*printf("can't handle this event yet.\n");*/
				break;
			}
		}

		ggiSetGCForeground(m->w.vis,
				   ggiMapColor(m->w.vis,&m->entrycolor));
		ggiSetGCBackground(m->w.vis,
				   ggiMapColor(m->w.vis,&m->w.backgroundcolor));
		ggiPuts( m->w.vis,
			 m->w.xorigin+m->entry[oldselection].x,
			 m->w.yorigin+m->entry[oldselection].y,
			 m->entry[oldselection].text);
		if (selected<0) selected = 0;
		if (selected > m->lastentry) selected = m->lastentry;
		
		ggiSetGCForeground(m->w.vis,
				   ggiMapColor(m->w.vis,&m->selectedcolor));
		ggiSetGCBackground(m->w.vis,
				   ggiMapColor(m->w.vis,
					       &m->selectedbackgroundcolor));
		ggiPuts( m->w.vis,
			 m->w.xorigin + m->entry[selected].x,
			 m->w.yorigin + m->entry[selected].y,
			 m->entry[selected].text);
		oldselection = selected;
	}
}
