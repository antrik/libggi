/* $Id: flatpanel.c,v 1.3 2006/03/27 19:46:31 pekberg Exp $
******************************************************************************

   Monitest flat panel check routine

   Authors:	1998 Hartmut Niemann
   		1999 Marcus Sundberg	[marcus@ggi-project.org]

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

#include "monitest.h"

#define COL(x) (x==0?black:(x==1?red:(x==2?green:(x==3?blue:white))))
void flatpanel(ggi_visual_t vis)
{  
	ggi_mode currmode;
	int mousex, mousey, boxsizex, boxsizey;
	int oldmx, oldmy, oldbx, oldby, oldbg;
	int flushx, flushy, flushw, flushh;
	int fgcol, bgcol;	/* 0: black, 1: red, 2: green, 3: blue,
				   4: white */
	int sizing, keeprunning;
	int xmax, ymax;
	int tmp;

	ggiGetMode(vis,&currmode);
        flushw = xmax = currmode.visible.x;
        flushh = ymax = currmode.visible.y;

	flushx = flushy = oldmx = oldmy = mousex = mousey = 0;
	oldbx = oldby = boxsizex = boxsizey = 100;
	fgcol = 4;
	bgcol = 0;
	sizing = 0;
	keeprunning = 1;

	ggiSetGCForeground(vis, COL(bgcol));
	ggiFillscreen(vis);
	
	while (keeprunning) {
		int tmpold, tmpnew;
		int events;
		int fillcol;
		gii_event event;
		
		ggiSetGCForeground(vis, COL(fgcol));
		ggiDrawBox(vis, mousex, mousey, boxsizex, boxsizey);
		ggiFlushRegion(vis, flushx, flushy, flushw, flushh);

		/* Wait for event(s) to come in */
		giiEventPoll(vis, emKey|emPointer, NULL);
		events = giiEventsQueued(vis, emKey|emPointer);

		oldmx = mousex;
		oldmy = mousey;
		oldbx = boxsizex;
		oldby = boxsizey;
		oldbg = bgcol;
		fillcol = -1;

		while (events--) {
			giiEventRead(vis, &event, emKey|emPointer);

			switch(event.any.type) {
			case evPtrButtonPress:
				switch(event.pbutton.button) {
				case GII_PBUTTON_FIRST:
					fgcol++;if (fgcol>4) fgcol = 0;
					break;
				case GII_PBUTTON_SECOND:
					sizing = 1;
					break;
				case GII_PBUTTON_THIRD:
					tmp = fgcol;
					fgcol = bgcol;
					bgcol = tmp;
					fillcol = bgcol;
					break;
				}
				break;
			case evPtrButtonRelease:
				switch (event.pbutton.button) {
				case GII_PBUTTON_SECOND:
					sizing = 0;
					break;
				}
				break;
			case evPtrAbsolute:
				if (sizing) {
					boxsizex = event.pmove.x-mousex;
					boxsizey = event.pmove.y-mousey;
					if (boxsizex < 1) boxsizex = 1;
					if (boxsizey < 1) boxsizey = 1;
				} else {
					mousex = event.pmove.x;
					mousey = event.pmove.y;
				}
				break;
			case evPtrRelative:
				if (sizing) {
					boxsizex += event.pmove.x;
					boxsizey += event.pmove.y;
					if (boxsizex < 1) boxsizex = 1;
					if (boxsizey < 1) boxsizey = 1;
					if (boxsizex > xmax) boxsizex = xmax;
					if (boxsizey > ymax) boxsizey = ymax;
				} else {
					mousex += event.pmove.x;
					mousey += event.pmove.y;
				}
				break;
			case evKeyPress:
			case evKeyRepeat:
				switch (event.key.sym){
				case ' ':
					fgcol++;if (fgcol>4) fgcol = 0;
					break;
				case '0': fgcol = 0; break;
				case '1': 
				case 'r': fgcol = 1; break;
				case '2': 
				case 'g': fgcol = 2; break;
				case '3': 
				case 'b': fgcol = 3; break;
				case '4': 
				case 'w': fgcol = 4; break;
				case 'i':
					tmp = fgcol;
					fgcol = bgcol;
					bgcol = tmp;
					fillcol = bgcol;
					break;
				case GIIK_Up:
					mousey -= (boxsizey >> 1);
					break;
				case GIIK_Down:
					mousey += (boxsizey >> 1);
					break;
				case GIIK_Left:
					mousex -= (boxsizex >> 1);
					break;
				case GIIK_Right:
					mousex += (boxsizex >> 1);
					break;
					
				case 'q': case 'Q':
				case GIIUC_Escape:
					keeprunning = 0;
					break;
				}
			}
			if (mousex < 0) mousex = 0;
			if (mousey < 0) mousey = 0;
			if (mousex > xmax - boxsizex) mousex = xmax - boxsizex;
			if (mousey > ymax - boxsizey) mousey = ymax - boxsizey;
		}
		if (fillcol != -1) {
			ggiSetGCForeground(vis, COL(fillcol));
			ggiFillscreen(vis);
			flushx = flushy = 0;
			flushw = xmax;
			flushh = ymax;
			continue;
		}
		/* Clear old box */
		ggiSetGCForeground(vis, COL(oldbg));
		if (mousex > oldmx) {
			ggiDrawBox(vis, oldmx, oldmy, mousex - oldmx, oldby);
			flushx = oldmx;
		} else {
			flushx = mousex;
		}
		if (mousey > oldmy) {
			ggiDrawBox(vis, oldmx, oldmy, oldbx, mousey - oldmy);
			flushy = oldmy;
		} else {
			flushy = mousey;
		}
		tmpold = oldmx + oldbx;
		tmpnew = mousex + boxsizex;
		if (tmpnew < tmpold) {
			ggiDrawBox(vis, tmpnew, oldmy, tmpold - tmpnew, oldby);
			flushw = tmpold - flushx;
		} else {
			flushw = tmpnew - flushx;
		}
		tmpold = oldmy + oldby;
		tmpnew = mousey + boxsizey;
		if (tmpnew < tmpold) {
			ggiDrawBox(vis, oldmx, tmpnew, oldbx, tmpold - tmpnew);
			flushh = tmpold - flushy;
		} else {
			flushh = tmpnew - flushy;
		}
	}
}
