/* $Id: pattern.c,v 1.2 2003/07/05 14:04:25 cegger Exp $
******************************************************************************

   Monitest core pattern routine

   Written in 1998 by Hartmut Niemann

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.
 
******************************************************************************
*/

#include <ggi/ggi.h>

#include "monitest.h"
#include "drawlib.h"

void testpattern(ggi_visual_t vis)
{
#define PARTSHOR 16
#define PARTSVERT 12
#define PARTX(x) ((x)*(xmax-1)/PARTSHOR)
#define PARTWIDTH (xmax/PARTSHOR)
#define PARTY(y) ((y)*(ymax-1)/PARTSVERT)
#define PARTHEIGHT (ymax/PARTSVERT)
#define MIDX PARTX(PARTSHOR/2)
#define MIDY PARTY(PARTSVERT/2)
#define FONTX fontwidth
#define FONTY fontheight
#define        MIN(_a,_b)      ((_a)<(_b)?(_a):(_b))
	
	int xmax, ymax;
	ggi_color col;
	int i;
	ggi_mode currmode;
	int radius;

	int xleft;
	int xright;
	ggi_pixel reddish, bluish, greenish, grey;
	int x, c;

	char s[255];

        int fontwidth, fontheight;

	ggiGetMode(vis,&currmode);
	xmax = currmode.visible.x;
	ymax = currmode.visible.y;

	ggiGetCharSize(vis,&fontwidth,&fontheight);

	/* clear background */
	ggiSetGCForeground(vis, black);
	ggiDrawBox(vis,0,0, xmax, ymax);
	
       	ggiSetGCForeground(vis, white);

	/* Grid */
	for (i = 0;i<=PARTSHOR;i++) 
		ggiDrawVLine(vis, PARTX(i),0, ymax);
	for (i = 0;i<=PARTSVERT;i++) {
		ggiDrawHLine(vis,0, PARTY(i), xmax);
	}

	radius = MIN(PARTHEIGHT , PARTWIDTH );
	circle(vis, PARTX(1), PARTY(1), radius);
	circle(vis, PARTX(PARTSHOR-1), PARTY(1), radius);
	circle(vis, PARTX(1), PARTY(PARTSVERT-1), radius);
	circle(vis, PARTX(PARTSHOR-1), PARTY(PARTSVERT-1), radius);
	circle(vis, PARTX(PARTSHOR/2), PARTY(PARTSVERT/2),/*ymax/2*/
	       MIN( (PARTY(PARTSVERT-1)-PARTY(1))/2, 
		    (PARTX(PARTSHOR-1) -PARTX(1))/2  ));

	ggiSetGCForeground(vis, white);
	ggiDrawBox(vis, PARTX(PARTSHOR/2-4), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, yellow);
	ggiDrawBox(vis, PARTX(PARTSHOR/2-3), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, red);
	ggiDrawBox(vis, PARTX(PARTSHOR/2-2), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, green);
	ggiDrawBox(vis, PARTX(PARTSHOR/2-1), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, cyan);
	ggiDrawBox(vis, PARTX(PARTSHOR/2+0), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, blue);
	ggiDrawBox(vis, PARTX(PARTSHOR/2+1), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, magenta);
	ggiDrawBox(vis, PARTX(PARTSHOR/2+2), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);
	ggiSetGCForeground(vis, black);
	ggiDrawBox(vis, PARTX(PARTSHOR/2+3), PARTY(3)+1, PARTWIDTH,
		   PARTY(5)-PARTY(3)-1);


	/* increasing intensity from left to right */
	/* colour bars red, green, blue and white -> black */
	xleft = PARTX(PARTSHOR/2-4)+1;
	xright = PARTX(PARTSHOR/2+4)-1;
	for (x = xleft;x<=xright;x++){
		c=((xright-x)*0xFFFF)/(xright-xleft);
		
		col.r = c; col.g = c; col.b = c;
		grey = ggiMapColor(vis,&col);
		col.r = c; col.g = 0; col.b = 0;
		reddish = ggiMapColor(vis,&col);
		col.r = 0; col.g = c; col.b = 0;
		greenish = ggiMapColor(vis,&col);
		col.r = 0; col.g = 0; col.b = c;
		bluish = ggiMapColor(vis,&col);

		ggiSetGCForeground(vis, reddish);		
		ggiDrawVLine(vis, x,PARTY(PARTSVERT-5)+1, PARTHEIGHT/2);

		ggiSetGCForeground(vis, bluish);
		ggiDrawVLine(vis, x,PARTY(PARTSVERT-5)+1+PARTHEIGHT/2 ,
			     PARTHEIGHT/2);

		ggiSetGCForeground(vis, greenish);
		ggiDrawVLine(vis, x,PARTY(PARTSVERT-4)+1, PARTHEIGHT/2);

		ggiSetGCForeground(vis, grey);
		ggiDrawVLine(vis, x,PARTY(PARTSVERT-4)+1+PARTHEIGHT/2 ,
			     PARTHEIGHT/2);
	}

#if 0
	/* colour bar red -> blue on top, yellow -> cyan on bottom */
	xleft = PARTX(PARTSHOR/2-5)+1;
	xright = PARTX(PARTSHOR/2+5)-1;
		
	for (x = xleft;x<=xright;x++){
		c=((xright-x)*0xFFFF)/(xright-xleft);
		
		col.r = c; col.g = 0; col.b = 0xffff-c;
		ggiSetGCForeground(vis, ggiMapColor(vis,&col));
		ggiDrawVLine(vis, x,PARTY(0)+1, PARTHEIGHT/2);

		col.g = 0xffff;
		ggiSetGCForeground(vis, ggiMapColor(vis,&col));
		ggiDrawVLine(vis, x,PARTY(PARTSVERT-1)+PARTHEIGHT/2 ,
			     PARTHEIGHT/2);
	}

	ytop =PARTY(PARTSVERT/2-4)+1;
	ybott =PARTY(PARTSVERT/2+4)-1;
		
	
	for (y = ytop;y<=ybott;y++){
		c=((ybott-y)*0xFFFF)/(ybott-ytop);
		
		col.r = c; col.g = 0xffff-c; col.b = 0;
		ggiSetGCForeground(vis, ggiMapColor(vis,&col));
		ggiDrawHLine(vis,1, y,PARTWIDTH/2);

		col.r = 0; col.b = c; col.g = 0xffff-c;
		ggiSetGCForeground(vis, ggiMapColor(vis,&col));
		ggiDrawHLine(vis, PARTX(PARTSHOR-1)+PARTWIDTH/2, y,
			     PARTWIDTH/2);
	}
#endif

	stripevert(vis, PARTX(0)+1, PARTY(0)+1,
		   PARTX(1)-1, PARTY(1)-1, black, white,1);
	stripevert(vis, PARTX(PARTSHOR-1)+1, PARTY(0)+1,
		   PARTX(PARTSHOR)-1, PARTY(1)-1, black, white,1);
	stripevert(vis, PARTX(0)+1, PARTY(PARTSVERT-1)+1,
		   PARTX(1)-1, PARTY(PARTSVERT)-1, black, white,1);
	stripevert(vis, PARTX(PARTSHOR-1)+1, PARTY(PARTSVERT-1)+1,
		   PARTX(PARTSHOR)-1, PARTY(PARTSVERT)-1, black, white,1);

	stripevert(vis, PARTX(PARTSHOR/2-4)+1, PARTY(PARTSVERT/2-1)+1,
		   PARTX(PARTSHOR/2-3)-1, PARTY(PARTSVERT/2)-1,
		   black, white,1);
	stripevert(vis, PARTX(PARTSHOR/2-4)+1, PARTY(PARTSVERT/2)+1,
		   PARTX(PARTSHOR/2-3)-1, PARTY(PARTSVERT/2+1)-1,
		   black, red,1);
	stripevert(vis, PARTX(PARTSHOR/2+3)+1, PARTY(PARTSVERT/2-1)+1,
		   PARTX(PARTSHOR/2+4)-1, PARTY(PARTSVERT/2)-1,
		   black, green,1);
	stripevert(vis, PARTX(PARTSHOR/2+3)+1, PARTY(PARTSVERT/2)+1,
		   PARTX(PARTSHOR/2+4)-1, PARTY(PARTSVERT/2+1)-1,
		   black, blue,1);

	/* text field in the middle */
	
	/* text output: */
	ggiSetGCForeground(vis, white);
	ggiSetGCBackground(vis, black);
	ggiPuts(vis, MIDX-(15*FONTX)/2, MIDY-2*FONTY,"GGI screen test");
	
	/* -visible resolution */
	sprintf(s,"%4dx%3d %d/%d", currmode.visible.x, currmode.visible.y,
		GT_DEPTH(currmode.graphtype), GT_SIZE(currmode.graphtype));
	ggiPuts(vis, MIDX-(11*FONTX)/2, MIDY-1*FONTY, s);
	
	/* -virtual resolution */
	sprintf(s,"virtual %4dx%3d", currmode.virt.x, currmode.virt.y);
	ggiPuts(vis, MIDX-(16*FONTX)/2, MIDY+0*FONTY, s);
	
	ggiPuts(vis, MIDX-(13*FONTX)/2, MIDY+1*FONTY,"Press <Space>");

	ggiFlush(vis);
}








