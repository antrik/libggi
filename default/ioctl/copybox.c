/* $Id: copybox.c,v 1.2 2002/09/08 21:37:42 soyt Exp $
******************************************************************************

   Graphics library for GGI. Copybox

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

#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>

#include <kgi/kgi_commands.h>

#include "ioctllib.h"

/**********/
/* BitBlt */
/**********/

int GGIcopybox(ggi_visual *vis,int x,int y,int w,int h,int nx,int ny)
{
	int RC;
	int result;

	CHECKXYWH(vis,x,y,w,h);
	CHECKXYWH(vis,nx,ny,w,h);
	
	if ((RC=ioctl(vis->fd,(int)ACCEL_COPYBOX,&x))<0) 
	{ 
		result = (RC == -1) ? errno : -RC;
		if ((result&NOSUP_MASK)==NOSUP)
		{	switch(result&AVE_MASK)
			{ case AVE_NOW:
				return fallback_opdraw.copybox(vis,x,y,w,h,nx,ny);
			  default:
				return (vis->opdraw->copybox=fallback_opdraw.copybox)(vis,x,y,w,h,nx,ny);
			}
		}
	}
	return RC;
}
