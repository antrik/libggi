/* $Id: mousedemo.c,v 1.2 2004/02/02 19:22:02 cegger Exp $
******************************************************************************

   Written in 1998 by Andreas Beck	[becka@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************
*/

/* Include the LibGGI declarations.
 */
#include <ggi/ggi.h>

/* Include the necessary headers used for e.g. error-reporting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ggi_visual_t vis;

enum basecolors {
	BC_BLACK,
	BC_RED,
	BC_GREEN,
	BC_BLUE,
	BC_YELLOW,
	BC_CYAN,
	BC_MAGENTA,
	BC_WHITE,
	BC_LAST
};

ggi_color colorstructs[BC_LAST]={
	{0x0000,0x0000,0x0000,0x0000},	/* BC_BLACK   */
	{0xffff,0x0000,0x0000,0x0000},	/* BC_RED     */
	{0x0000,0xffff,0x0000,0x0000},	/* BC_GREEN   */
	{0x0000,0x0000,0xffff,0x0000},	/* BC_BLUE    */
	{0xffff,0xffff,0x0000,0x0000},	/* BC_YELLOW  */
	{0x0000,0xffff,0xffff,0x0000},	/* BC_CYAN    */
	{0xffff,0x0000,0xffff,0x0000},	/* BC_MAGENTA */
	{0xffff,0xffff,0xffff,0x0000} 	/* BC_WHITE   */
};

ggi_pixel pixelvalues[BC_LAST];

static void setup_pixelvalues(void)
{
	int x;
	for(x=0;x<BC_LAST;x++)
		pixelvalues[x]=ggiMapColor(vis,&colorstructs[x]);
}

/* Palette helper function
 */
static void Setup_Palette(void)
{
        ggiSetPalette(vis, 0, BC_LAST, colorstructs);
}


/* The main routine.
 * It will set up a graphics mode as requested on the commandline and 
 * do an extensive test using about all graphics primitives LibGGI
 * knows.
 */
int main(int argc, char **argv)
{
	/* This is an enum holding the requested type of graphics mode.
	 * See the mode setting code below for details.
	 */
	ggi_graphtype type;

	/* The depth is used for chosing a graphics type.  it is mapped
	 * to the right GT_* by a switch-statement.
	 */
	int depth=0;

	/* These holde the visible screen size (sx,sy) and
	 * the virtual size (vx,vy). On most targets you can 
	 * request a certain larger area on which the visible
	 * area can be placed.
	 */
	int sx,sy,vx,vy;
	
	/* This is a struct containing visual and virtual screen size as
	 * well as the bpp/textmode information 
	 */

	ggi_mode mode = { /* This will cause the default mode to be set */
		1,                      /* 1 frame [???] */
		{GGI_AUTO,GGI_AUTO},    /* Default size */
		{GGI_AUTO,GGI_AUTO},    /* Virtual */
		{0,0},                  /* size in mm don't care */
		GT_AUTO,               /* Mode */
		{GGI_AUTO,GGI_AUTO}     /* Font size */
	};

	int x,err;
	
	int mousex,mousey;
	int boxsizex,boxsizey;
	int fgcol,bgcol;
	int sizing,keeprunning;

	/* Initialize the GGI library. This must be called before any other 
	 * GGI function. Otherwise behaviour is undefined.
	 */
	if (ggiInit() != 0) {
		fprintf(stderr, "Unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	vis=ggiOpen(NULL);

	if (vis == NULL) {
		fprintf(stderr,
			"Unable to open default visual, exiting.\n");
		ggiExit();
		exit(1);
	}

	ggiSetFlags(vis,GGIFLAG_ASYNC);
	/* that's what we try. See what we get ... */
	printf("Trying mode ");
	ggiPrintMode(&mode);
	printf("\n");

	/* Is the mode possible ? If not, a better one will be
	 * suggested. 
	 */
	ggiCheckMode(vis,&mode);
	printf("Suggested mode ");
	ggiPrintMode(&mode);
	printf("\n");

	err=ggiSetMode(vis,&mode);   /* now try it. it *should* work! */
	
	if (err) {
		fprintf(stderr,"Can't set mode\n");
		ggiClose(vis);
		ggiExit();
		return 2;
	}

	type=mode.graphtype;
	vx=mode.virt.x;    vy=mode.virt.y;
	sx=mode.visible.x; sy=mode.visible.y;
	depth=GT_DEPTH(mode.graphtype);

	if (GT_SCHEME(mode.graphtype) == GT_PALETTE) {
		Setup_Palette();
	}
	setup_pixelvalues();
	
	mousex=mousey=0;
	boxsizex=boxsizey=100;
	fgcol=BC_WHITE;
	bgcol=BC_BLACK;
	sizing=0;keeprunning=1;
	
	while(keeprunning)
	{
		ggi_event event;
		
		ggiSetGCForeground(vis, pixelvalues[fgcol]);
		ggiDrawBox(vis,mousex,mousey,boxsizex,boxsizey);
		ggiFlush(vis);
		ggiEventRead(vis,&event,emKey|emPointer);
		ggiSetGCForeground(vis, pixelvalues[bgcol]);
		ggiDrawBox(vis,mousex,mousey,boxsizex,boxsizey);
		
		switch(event.any.type)
		{
			case evPtrButtonPress:
				switch(event.pbutton.button)
				{
					case GII_PBUTTON_FIRST:
						fgcol++;if (fgcol>=BC_LAST) fgcol=0;
						break;
					case GII_PBUTTON_SECOND:
						sizing=1;
						break;
					case GII_PBUTTON_THIRD:
						x=fgcol;fgcol=bgcol;bgcol=x;
						ggiSetGCForeground(vis, pixelvalues[bgcol]);
						ggiFillscreen(vis);
						break;
				}
				break;
			case evPtrButtonRelease:
				switch(event.pbutton.button)
				{
					case GII_PBUTTON_SECOND:
						sizing=0;
						break;
				}
				break;
			case evPtrAbsolute:
				if (sizing) {
					boxsizex=event.pmove.x-mousex;
					boxsizey=event.pmove.y-mousey;
					if (boxsizex<1) boxsizex=1;
					if (boxsizey<1) boxsizey=1;
				} else {
					mousex=event.pmove.x;
					mousey=event.pmove.y;
				}
				break;
			case evPtrRelative:
				if (sizing) {
					boxsizex+=event.pmove.x;
					boxsizey+=event.pmove.y;
					if (boxsizex<1) boxsizex=1;
					if (boxsizey<1) boxsizey=1;
				} else {
					mousex+=event.pmove.x;
					mousey+=event.pmove.y;
				}
				break;
			case evKeyPress:
				keeprunning=0;
				break;
		}
	}
	ggiClose(vis);
	ggiExit();	

	return 0;
}
