/* $Id: ggitext.c,v 1.5 2005/07/30 08:43:03 soyt Exp $
******************************************************************************

   Implementation of ggitext: routines for formatted text output

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
#include <ggi/ggi.h>
#include <stdlib.h>     /* malloc */
#include <stdio.h>      /* printf, only debugging */

#include "ggitext.h"


int ggiGraphTextCharwidth(ggi_visual_t vis, const char c)     
{
	int w, h;
	ggiGetCharSize(vis,&w,&h);  /* until we have some font management. */
	
	return w;
}


int ggiGraphTextCharheight(ggi_visual_t vis, const char c)
{
	int w, h;
	ggiGetCharSize(vis,&w,&h);  /* until we have some font management. */
	
	return h;
}

int ggiGraphTextStringwidth(ggi_visual_t vis, const char * c)
{
	int w;
	w = 0;
	while ( *c != '\000'){
		w += ggiGraphTextCharwidth(vis,*c);
		c++;
	}
	return w;
}
int ggiGraphTextStringheight(ggi_visual_t vis, const char * c)
{
	return ggiGraphTextCharheight(vis, *c );
}

int ggiGraphTextPuts(ggi_visual_t vis,
		     int x, int y, int width, int height, 
		     int flags,
		     char * text )
{
	char * tmp;

	int txwidth;   /* width of the formatted text in pixels */
	int txheight;  /* height               "                */

	int posx;      /* reference position for the final puts */
	int posy;

	int h;

	if (flags & GGI_TEXT_FRAME){
		ggiDrawHLine(vis, x,y, width);
		ggiDrawHLine(vis, x,y+height, width);
		ggiDrawVLine(vis, x,y, height);
		ggiDrawVLine(vis, x+width, y,height);
	}

	txwidth = 0 ;	
	txheight= 0 ;
	tmp = text;

	while ( *tmp != '\000') {
		/*printf("%c",*tmp);*/
		txwidth += ggiGraphTextCharwidth(vis, * tmp);
		h = ggiGraphTextCharheight(vis,* tmp);
		if ( h > txheight){
			txheight = h;
		}
		tmp++;
	}

	posx = x;
	if (flags & GGI_TEXT_FRAME) {
		posx +=2;
		width -=4;
	}
	switch (flags & (GGI_TEXT_CENTER | GGI_TEXT_LEFT | GGI_TEXT_RIGHT
			  | GGI_TEXT_JUSTIFY)){
	case GGI_TEXT_CENTER: 
		posx += (width-txwidth)/2; break;
	case GGI_TEXT_LEFT:   
		break;
	case GGI_TEXT_RIGHT:
		posx += (width-txwidth);break;
	case GGI_TEXT_JUSTIFY:
		return -1 ; /* not implemented */
		break; /* never get here */
	default:
		return -1 ; /* not implemented */
		break; /* never get here */
	}

	posy = y;
	if (flags & GGI_TEXT_FRAME) {
		posy +=2;
		height -=4;
	}
	switch (flags & (GGI_TEXT_CENTER | GGI_TEXT_TOP | GGI_TEXT_BOTTOM)){
	case GGI_TEXT_CENTER: 
		posy += (height-txheight)/2; break;
	case GGI_TEXT_TOP:   
		break;
	case GGI_TEXT_BOTTOM:
		posy += (height-txheight);break;
	case GGI_TEXT_TOP | GGI_TEXT_BOTTOM:
		return -1 ; /* not implemented */
		break; /* never get here */
	default:
		return -1 ; /* not implemented */
		break; /* never get here */
	}

/*	printf("x=%d y=%d w=%d h=%d flags=%d posx=%d posy=%d\n",
**	x, y,width, height, flags, posx, posy);
*/

	ggiPuts(vis, posx, posy, text);
	return 0; 
}


int ggiGraphTextLongPuts(ggi_visual_t vis,
			 int x, int y, int width, int height, 
			 int flags,
			 char * text )
/* in this case text is allowed to contain \n. */
/* the text itself is shown left-justified.    */
{
	char * tmp;
	char * tmpdest;

	struct line {
		char * text;
		struct line * next;
		int cpl ; /* chars per line */
		int x;  /* reference, to be calculated! */
		int y;
		int width;
		int height;
	} firstline;

	struct line * currline;
 
	int txwidth;   /* width of the formatted text in pixels */
	int txheight;  /* height               "                */

	int posx;      /* reference position for the final puts */
	int posy;

	int h;

	/* process lines */
	currline = &firstline;

	/* chars per line , and line headers */
	tmp = text;
	firstline.cpl = 0;
	while ( (* tmp) && (* tmp != '\n') ) {
		tmp ++;
		firstline.cpl++;
	}
	/*printf ("first line length %d\n", currline->cpl);*/
	while (* tmp){      /* until end of string */
		/*printf ("allocating line header");*/
		currline->next = (struct line *) malloc(sizeof(struct line));
		if (currline->next == NULL){
			ggiPanic("Out of memory");
		}
		currline = currline -> next;
		currline -> next = NULL;
		currline -> cpl =0;
		tmp++; /* now at start of next line */

		while ( (* tmp) && (* tmp != '\n') ) {
			tmp ++;
			currline ->cpl++;
		}
		/*printf ("..line length %d\n", currline->cpl);*/
	}

	/* copy strings and add \0 */

	txwidth = 0;
	txheight = 0;

	currline = &firstline;
	tmp = text;
	while (currline != NULL) {
		tmpdest = (char *) malloc(sizeof(char)*
					  (currline->cpl+1));
		if ( (currline->text = tmpdest) == NULL){
			ggiPanic("Out of memory");
		}
		currline -> width  = 0;
		currline -> height = 0;
		while ( (* tmp) && (* tmp != '\n') ) {
			*tmpdest = *tmp;
			/*printf("%c",*tmp);*/
			currline ->width += ggiGraphTextCharwidth(vis,* tmp);
			h = ggiGraphTextCharheight(vis, * tmp);
			if (h>currline -> height){
				currline -> height = h;
			}

			tmp ++;
			tmpdest ++;
			
		}
		*tmpdest= '\000';    /* endofstring */
		tmp++;
		/*printf(" h=%d, w=%d \n", currline->height, currline->width);*/
		if (currline->width > txwidth) {
			txwidth = currline ->width;
		}
		txheight += currline ->height;
		currline = currline->next;
	}
/*	printf("sum: h=%d w=%d\n", txheight, txwidth);*/


	if (flags & GGI_TEXT_FRAME){
		ggiDrawHLine(vis, x,y, width);
		ggiDrawHLine(vis, x,y+height, width);
		ggiDrawVLine(vis, x,y, height);
		ggiDrawVLine(vis, x+width, y,height);
	}


	posx = x;
	if (flags & GGI_TEXT_FRAME) {
		posx +=2;
		width -=4;
	}
	switch (flags & (GGI_TEXT_CENTER | GGI_TEXT_LEFT | GGI_TEXT_RIGHT
			  | GGI_TEXT_JUSTIFY)){
	case GGI_TEXT_CENTER: 
		posx += (width-txwidth)/2; break;
	case GGI_TEXT_LEFT:   
		break;
	case GGI_TEXT_RIGHT:
		posx += (width-txwidth);break;
	case GGI_TEXT_JUSTIFY:
		return -1 ; /* not implemented */
		break; /* never get here */
	default:
		return -1 ; /* not implemented */
		break; /* never get here */
	}

	posy = y;
	if (flags & GGI_TEXT_FRAME) {
		posy +=2;
		height -=4;
	}
	switch (flags & (GGI_TEXT_CENTER | GGI_TEXT_TOP | GGI_TEXT_BOTTOM)){
	case GGI_TEXT_CENTER: 
		posy += (height-txheight)/2; break;
	case GGI_TEXT_TOP:   
		break;
	case GGI_TEXT_BOTTOM:
		posy += (height-txheight);break;
	case GGI_TEXT_TOP | GGI_TEXT_BOTTOM:
		return -1 ; /* not implemented */
		break; /* never get here */
	default:
		return -1 ; /* not implemented */
		break; /* never get here */
	}


	currline = &firstline;
	while (currline != NULL) {
		/*
		**  currline->x = posx;
		**  currline->y = posy;
		*/
		/*printf("printing at %d,%d: %s\n", posx, posy, currline->text);*/
		ggiPuts(vis, posx, posy, currline->text);
		posy+=currline->height;
		currline = currline -> next;
	}

	currline = firstline.next;
	{
		struct line * t;
		while (currline != NULL){
			t = currline->next;
			free(currline);
			/*printf("free-");*/
			currline = t;
		}
	}

	return 0; 
}






