/* $Id: mode.c,v 1.1 2001/05/12 23:02:17 cegger Exp $
******************************************************************************

   Display-sub: mode management

   Copyright (C) 1998 Andreas Beck    [becka@ggi-project.org]

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

#include <string.h>

#include <ggi/display/sub.h>

int GGI_sub_getapi(ggi_visual *vis, int num, char *apiname, char *arguments)
{
	if(num==0) {
		strcpy(apiname, "display-sub");
		strcpy(arguments, "");
		return 0;
	}

	return -1;
}

int GGI_sub_setmode(ggi_visual *vis,ggi_mode *tm)
{ 
	ggi_sub_priv *subinfo=LIBGGI_PRIVATE(vis);
	subinfo->position.x=tm->visible.x;
	subinfo->position.y=tm->visible.y;
	subinfo->botright.x=tm->virt.x+tm->visible.x;
	subinfo->botright.y=tm->virt.y+tm->visible.y;

	tm->visible=tm->virt;
        memcpy(LIBGGI_MODE(vis),tm,sizeof(ggi_mode));
                                                                                                
	return 0;
}

/**********************************/
/* check any mode (text/graphics) */
/**********************************/
int GGI_sub_checkmode(ggi_visual *vis,ggi_mode *tm)
{
	return -1;
}

/************************/
/* get the current mode */
/************************/
int GGI_sub_getmode(ggi_visual *vis,ggi_mode *tm)
{
        memcpy(tm,LIBGGI_MODE(vis),sizeof(ggi_mode));
	tm->visible.x=tm->virt.x;
	tm->visible.y=tm->virt.y;
	tm->frames=1;
	return 0;
}

/*************************/
/* set the current flags */
/*************************/
int GGI_sub_setflags(ggi_visual *vis,ggi_flags flags)
{
	ggi_sub_priv *priv=LIBGGI_PRIVATE(vis);

	if (ggiSetFlags(priv->parent, flags)) return -1;

	LIBGGI_FLAGS(vis) = flags;

	return 0;	
}

int GGI_sub_flush(ggi_visual *vis, int x, int y, int w, int h, int tryflag)
{
	ggi_sub_priv *priv = LIBGGI_PRIVATE(vis);
	return _ggiInternFlush(priv->parent, x+priv->position.x, 
				y+priv->position.y, w, h, tryflag);
}
