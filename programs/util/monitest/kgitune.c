/* $Id: kgitune.c,v 1.3 2007/05/05 08:34:49 cegger Exp $
******************************************************************************

   Tuning of KGIcon monitor timings

   Written in 1998 by Andreas Beck	[becka@ggi-project.org
           in 1999 by Marcus Sundberg	[marcus@ggi-project.org

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************
*/

/* We need to get vis->fd here, so <ggi/ggi.h> won't do. */
#include <ggi/internal/ggi.h>

#include <kgimon.h>

#include "menu.h"

enum {
	MM_MOVE = 0x0,
	MM_RESIZE,
	MM_POLARITY,
	MM_SYNCWIDTH,
	MM_BMOVE,
	MM_BRESIZE
};



void testpattern(ggi_visual_t vis);

static struct kgi_preset curtiming;

static int
show_timing(ggi_visual *vis, struct kgi_preset *pres)
{
	printf("DCLK: %d\n", pres->dclk);
	printf("X: %d, %d, %d, %d, %d, %d, %d\n", 
	       pres->x.width, pres->x.blankstart, pres->x.syncstart,
	       pres->x.syncend, pres->x.blankend, pres->x.total,
	       pres->x.polarity);
	printf("Y: %d, %d, %d, %d, %d, %d, %d\n", 
	       pres->y.width, pres->y.blankstart, pres->y.syncstart,
	       pres->y.syncend, pres->y.blankend, pres->y.total,
	       pres->y.polarity);

	return 0;
}

static void
beep(void)
{
}


static int
calc_change(struct kgi_preset *trymode, int mode, int c)
{
	switch (mode) {
	case MM_MOVE:
		switch (c) { 
		case GIIK_Left:
			if (trymode->x.syncend+8 < trymode->x.total) {
				trymode->x.syncstart += 8;
				trymode->x.syncend += 8;
				if (trymode->x.blankend < trymode->x.syncend) {
					trymode->x.blankend += 8;
				}
			} else beep();
			break;
		case GIIK_Right:
			if (trymode->x.syncstart-8 > trymode->x.width) {
				trymode->x.syncstart -= 8;
				trymode->x.syncend -= 8; 
				if (trymode->x.blankstart>trymode->x.syncstart) {
					trymode->x.blankstart -= 8;
				}
			} else beep();
			break;
		case GIIK_Up:
			if (trymode->y.syncend+1<trymode->y.total) 
				{ trymode->y.syncstart++;trymode->y.syncend++; 
				if (trymode->y.blankend<trymode->y.syncend)
					trymode->y.blankend++;
				}
			else beep();
			break;
		case GIIK_Down:
			if (trymode->y.syncstart-1>trymode->y.width) 
				{ trymode->y.syncstart--;trymode->y.syncend--; 
				if (trymode->y.blankstart>trymode->y.syncstart)
					trymode->y.blankstart--;
				}
			else beep();
			break;
		default:
			return 0;
		}
		break;
	case MM_BMOVE:
		switch (c) {
		case GIIK_Right:
			if (trymode->x.blankend  +8<=trymode->x.total &&
			    trymode->x.blankstart+8<=trymode->x.syncstart) {
				trymode->x.blankstart += 8;
				trymode->x.blankend += 8;
			}
			else beep();
			break;
		case GIIK_Left:
			if (trymode->x.blankstart-8>=trymode->x.width &&
			    trymode->x.blankend  -8>=trymode->x.syncend) {
				trymode->x.blankstart-=8;
				trymode->x.blankend-=8;
			}
			else beep();
			break;
		case GIIK_Down:
			if (trymode->y.blankend  +1<=trymode->y.total &&
			    trymode->y.blankstart+1<=trymode->y.syncstart) {
				trymode->y.blankstart++;
				trymode->y.blankend++;
			}
			else beep();
			break;
		case GIIK_Up:
			if (trymode->y.blankstart-1>=trymode->y.width &&
			    trymode->y.blankend  -1>=trymode->y.syncend) {
				trymode->y.blankstart--;
				trymode->y.blankend--;
			}
			else beep();
			break;
		default:
			return 0;
		}
		break;
	case MM_BRESIZE:
		switch (c) {
		case GIIK_Right:
			if (trymode->x.blankend  +8<=trymode->x.total)
				trymode->x.blankend  +=8;
			if (trymode->x.blankstart-8>=trymode->x.width)
				trymode->x.blankstart-=8;
			break;
		case GIIK_Left:
			if (trymode->x.blankstart+8<=trymode->x.syncstart) 
				trymode->x.blankstart += 8;
			if (trymode->x.blankend  -8>=trymode->x.syncend) 
				trymode->x.blankend  -=8;
			break;
		case GIIK_Down:
			if (trymode->y.blankend  +1<=trymode->y.total)
				trymode->y.blankend  +=1;
			if (trymode->y.blankstart-1>=trymode->y.width)
				trymode->y.blankstart-=1;
			break;
		case GIIK_Up:
			if (trymode->y.blankstart+1<=trymode->y.syncstart) 
				trymode->y.blankstart += 1;
			if (trymode->y.blankend  -1>=trymode->y.syncend) 
				trymode->y.blankend  -=1;
			break;
		default:
			return 0;
		}
		break;
	case MM_RESIZE:
		switch (c) {
		case GIIK_Up:
			if (trymode->y.total-1>trymode->y.blankend)
				trymode->y.total--;
			else beep();
			break;
		case GIIK_Down:
			trymode->y.total++;
			break;
		case GIIK_Left:
			if (trymode->x.total-8>trymode->x.blankend)
				trymode->x.total-=8;
			else beep();
			break;
		case GIIK_Right:
			trymode->x.total += 8;
			break;
		default:
			return 0;
		}
		break;
	case MM_POLARITY:
		switch (c) {
		case GIIK_Left:
			trymode->x.polarity=0;
			break;
		case GIIK_Right:
			trymode->x.polarity=1;
			break;
		case GIIK_Up:
			trymode->y.polarity=0;
			break;
		case GIIK_Down:
			trymode->y.polarity=1;
			break;
		default:
			return 0;
		}
		break;
	case MM_SYNCWIDTH:
		switch (c) {
		case GIIK_Left:
			if (trymode->x.syncend-8>trymode->x.syncstart)
				trymode->x.syncend-=8;
			else beep();
			break;
		case GIIK_Right:
			if (trymode->x.syncend+8<trymode->x.total)
				{  trymode->x.syncend += 8;
				if (trymode->x.blankend<trymode->x.syncend)
					trymode->x.blankend += 8;
				}
			else beep();
			break;
		case GIIK_Up:
			if (trymode->y.syncend-1>trymode->y.syncstart)
				trymode->y.syncend--;
			else beep();
			break;
		case GIIK_Down:
			if (trymode->y.syncend+1<trymode->y.total)
				{  trymode->y.syncend++;
				if (trymode->y.blankend<trymode->y.syncend)
					trymode->y.blankend++;
				}
			else beep();
			break;
		default:
			return 0;
		}
		break;
	}

	return 1;
}


static int
do_change(ggi_visual *vis, struct kgi_preset *pres)
{
	ggi_mode mode;
	ggiGetMode(vis, &mode);
	set_preset(vis->fd, pres);
	ggiSetMode(vis, &mode);
	testpattern(vis);

	return 0;
}

static int
interactive(ggi_visual *vis)
{
	int c;
	int mode = MM_MOVE;
	int quit = 0;
	struct kgi_preset trymode, origmode;
	trymode = origmode = curtiming;

	do {
		c = giiGetc(vis);
		if (calc_change(&trymode, mode, c)) {
			do_change(vis, &trymode);
		} else {
			switch (c) {
			case GIIK_Enter:
				origmode = trymode;
				quit = 1;
				break;
			case GIIUC_Escape: case 'q': case 'Q':
				quit = 1;
				break;
			case 'h': case 'H':
				/* Display help ... */
				break;
			case 'm': case 'M':
				mode = MM_MOVE;
				break;
			case 'r': case 'R':
				mode = MM_RESIZE;
				break;
			case 'p': case 'P':
				mode = MM_POLARITY;
				break;
			case 's': case 'S':
				mode = MM_SYNCWIDTH;
				break;
			case 'b': case 'B':
				mode = MM_BMOVE;
				break;
			case 'e': case 'E':
				mode = MM_BRESIZE;
				break;
			}
		}
	} while (!quit);
	do_change(vis, &origmode);
	testpattern(vis);
	get_curtiming(vis->fd, &curtiming);

	return 0;
}

int
is_kgicondev(void *vis)
{
	ggi_visual *visptr = vis;

	if (get_curtiming(visptr->fd, &curtiming) == 0) {
		return 1;
	} else {
		return 0;
	}
}


int
kgitune(void *vis)
{
	int select = 0;
	struct menu kgimenu;
	ggi_visual *visptr = vis;

	if (!is_kgicondev(vis)) {
		return -1;
	}
	
	default_menu(&kgimenu, vis);

	kgimenu.lastentry = 1;
	kgimenu.entry[0].text = "1 Interactively tune timings";
	kgimenu.entry[1].text = "2 Exit";

	kgimenu.toptext = "Please choose with arrows";
	kgimenu.bottomtext = "and press <return>";

	calculate_menu(&kgimenu);
	center_menu(&kgimenu);

	while (1) {
		switch (select = do_menu(&kgimenu, select)) {
		case 0: 
			interactive(visptr);
			break;
		case 1:
			return 0;
		default:
			return 0;
		}
	}

	return 0;
}
