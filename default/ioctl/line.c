/* $Id: line.c,v 1.3 2006/03/12 23:15:06 soyt Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1995 Andreas Beck   [becka@ggi-project.org]
   Copyright (C) 1997 Jason McMullan [jmcc@ggi-project.org]

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

#include "config.h"
#include <ggi/internal/ggi-dl.h>

#include <kgi/kgi_commands.h>

#include "ioctllib.h"

#define SPEED_TRESHOLD 15


int GGIdrawline(struct ggi_visual *vis,int x1,int y1,int x2,int y2)
{
	int RC;
	int result;

	if ( ((abs(x2-x1) & ~SPEED_TRESHOLD) == 0) &&
	     ((abs(y2-y1) & ~SPEED_TRESHOLD) == 0) ) 
		return fallback_opdraw.drawline(vis,x1,y1,x2,y2);

	if ((RC=ioctl(vis->fd,(int)ACCEL_DRAWLINE,&x1))<0) 
	{ 
		result = (RC == -1) ? errno : -RC;
		if ((result&NOSUP_MASK)==NOSUP)
		{	switch(result&AVE_MASK)
			{ case AVE_NOW:
				return fallback_opdraw.drawline(vis,x1,y1,x2,y2);
			  default:
				return (vis->opdraw->drawline=fallback_opdraw.drawline)(vis,x1,y1,x2,y2);
			}
		}
	}

	return RC;
}
