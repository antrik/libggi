/* $Id: box.c,v 1.1 2001/05/12 23:01:37 cegger Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck     [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan   [jmcc@ggi-project.org]

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

#include <ggi/internal/ggi-dl.h>

#include <kgi/kgi_commands.h>

#include "ioctllib.h"

#define SPEED_TRESHOLD 7


int GGIdrawbox(ggi_visual *vis,int x,int y,int width,int length)
{
	int RC;
	int result;

	if ( ((width  & ~SPEED_TRESHOLD) == 0) &&
	     ((length & ~SPEED_TRESHOLD) == 0) ) 
		return fallback_opdraw.drawbox(vis,x,y,width,length);
	    
	if ((RC=ioctl(vis->fd,(int)ACCEL_DRAWBOX,&x))<0) 
	{ 
		/* linux glibc2: errors > 4095 are returned in RC and not in errno */
		result = (RC == -1) ? errno : -RC;
		if ((result&NOSUP_MASK)==NOSUP)
		{	switch(result&AVE_MASK)
			{ case AVE_NOW:
				return fallback_opdraw.drawbox(vis,x,y,width,length);
			  default:
				return (vis->opdraw->drawbox=fallback_opdraw.drawbox)(vis,x,y,width,length);
			}
		}
	}
	return RC;
}

static int lower_fillscreen(ggi_visual *vis)
{
	return GGIdrawbox(vis,0,0,LIBGGI_VIRTX(vis),LIBGGI_VIRTY(vis));
}

int GGIfillscreen(ggi_visual *vis)
{
	int RC;
	int result;

	if ( (RC=ioctl(vis->fd,(int)ACCEL_FILLSCREEN,NULL)) < 0 ) 
	{ 
		result = (RC == -1) ? errno : -RC;
		if ((result&NOSUP_MASK)==NOSUP)
		{	switch(result&AVE_MASK)
			{ case AVE_NOW:
				switch(result&AVM_MASK)
				{ case AVM_LOWER:
				  	return lower_fillscreen(vis);
				  default:
					return fallback_opdraw.fillscreen(vis);
				}
				break;
			  default:
				switch(result&AVM_MASK)
				{ case AVM_LOWER:
					return (vis->opdraw->fillscreen=lower_fillscreen)(vis);
				  default:
					return (vis->opdraw->fillscreen=fallback_opdraw.fillscreen)(vis);
				}
			}
		}
	}
	return RC;
}
