/* $Id: menu.h,v 1.1 2001/05/12 23:03:51 cegger Exp $
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

#ifndef _MENU_H
#define _MENU_H

#include <ggi/ggi.h>
#include "window.h"

#define MAXENTRIES 9
/* indexes 0 .. 8 but keys 1 ..9.
** any good solution for this??  
*/

struct menuentry {
	char * text ;        /* menu text to be printed */
	char * shortcut ;    /* Shortcut key(s) */
	int x;               /* printing position */
	int y;
};

struct menu {
	struct window w;
	 
	int lastentry ;                      /* number of last used entry */
	struct menuentry entry[MAXENTRIES] ;

	ggi_color entrycolor;

	ggi_color selectedcolor;
	ggi_color selectedbackgroundcolor;

	/* menu formatting. you should leave the defaults untouched. */
	int titleleftskip;       /* space left inner border <-> title */
	int titlerightskipmin;   /*       title <-> right inner border */
	int entryleftskip;       /**/
	int entryrightskipmin;

	int firstlineskip;
	int entrylineskip;
	int lastlineskip;

	char * toptext;
	int toptextx;
	int toptexty;
	ggi_color toptextcolor;

	char * bottomtext;
	int bottomtextx;
	int bottomtexty;
	ggi_color bottomtextcolor;

      

	int titlex, titley;    /* automatically calculated ! */

};

int default_menu(struct menu *m, ggi_visual_t vis);

int calculate_menu(struct menu *m);
/* calculates size and positions of elements */

int center_menu(struct menu *m);
/* moves menu to center of screen */

int do_menu(struct menu *m , int selected);
/* returns -1 for `quit' */

#endif /* _MENU_H */
