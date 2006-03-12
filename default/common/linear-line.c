/* $Id: linear-line.c,v 1.4 2006/03/12 23:15:04 soyt Exp $
******************************************************************************

   Graphics library for GGI.

   Copyright (C) 1998 Alexander Larsson   [alla@lysator.liu.se]
   
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

#include "../common/clip.c"

/* This is a general clipping linedrawer using a sliced run-length
   bresenham algorithm. It is modified to calculate the first and last
   run-lengths when they are clipped to get pixel-perfect rasterization.

   If you wan't to use this you have to define FBTYPE to the right frame-
   buffer type and then include this file.
   */

int GGIdrawline(struct ggi_visual *vis,int orig_x1,int orig_y1,int orig_x2,int orig_y2)
{
	int orig_dx,orig_dy,sx,sy;
	int dx,dy;
	int i;
	int w;
	int x1,y1,x2,y2;
	int clip_first,clip_last;
	FBTYPE *fb,color;

	color = LIBGGI_GC_FGCOLOR(vis);
  
	x1 = orig_x1;
	y1 = orig_y1;
	x2 = orig_x2;
	y2 = orig_y2;

	/* clip x1,y1 and x2,y2. Set clip_first and clip_last if clipped */
	if (!_ggi_clip2d(vis, &x1,&y1,&x2,&y2,&clip_first,&clip_last))
		return 0; /* Clipped */
  
	PREPARE_FB(vis);

	dy = y2 - y1;
	orig_dy = orig_y2 - orig_y1;
	sy=1;
	if (orig_dy<0) {
		orig_dy = -orig_dy;
		dy = -dy;
		sy = -1;
	}

	dx = x2-x1;
	orig_dx = orig_x2 - orig_x1;
	sx=1;
	if (orig_dx<0) {
		sx=-1;
		orig_dx = -orig_dx;
		dx = -dx;
	}

	w = LIBGGI_FB_W_STRIDE(vis);
  
	fb = (FBTYPE *) (((uint8_t *)LIBGGI_CURWRITE(vis))
			 + y1*w+x1*sizeof(FBTYPE));
  
	if (dx==0) {
		if (sy<0)
			w = -w;
   
		for (i=dy;i>=0; i--) {
			*fb = color;
			fb = (FBTYPE *) (((uint8_t *)fb) + w);
		}
		return 0;
	}

	if (dy==0) {
		for (i=dx;i>=0; i--) {
			*fb = color;
			fb += sx;
		}
		return 0;
	}

	if (orig_dx==orig_dy) {
		if (sy<0)
			w = -w;
		w += sx*sizeof(FBTYPE);
		for (i=dx;i>=0; i--) {
			*fb = color;
			fb = (FBTYPE *) (((uint8_t *)fb) + w);
		}
		return 0;
	}

	if (orig_dx >= orig_dy) { /* x major */
		int runlen,adjup,adjdown,e,len;
		int firstlen,lastlen;

		runlen = orig_dx/orig_dy;
		adjup = orig_dx%orig_dy;
		lastlen = firstlen = (runlen>>1) + 1;
		if (clip_first) { /* clipped, Adjust firstlen */
			int clip_dx = abs(x1 - orig_x1);
			int clip_dy = abs(y1 - orig_y1);
			int d = (2*clip_dy+1)*orig_dx; 
			firstlen = d/(2*orig_dy) - clip_dx + 1;
			e = d%(2*orig_dy);
			if ((e==0) && (sy>0)) { /* Special case, arbitrary choise. Select lower pixel.(?) */
				firstlen--;
				e += 2*orig_dy;
			}
			e -= (orig_dy*2);
		} else { /* Not clipped, calculate start error term */
			e = adjup - (orig_dy<<1); /* initial errorterm == half a step */
			if ((runlen&1) != 0) {
				e += orig_dy;
			}
		}
		if (clip_last) { /* Last endpoint clipped */
			int clip_dx = abs(x2 - orig_x2);
			int clip_dy = abs(y2 - orig_y2);
			int d = (1+2*clip_dy)*orig_dx; 
			lastlen = d/(2*orig_dy) - clip_dx + 1;
			if ((sy<0)  && ((d%(2*orig_dy))==0) ) /* special arbitrary case */
				lastlen--; 
		}
		adjup <<= 1;
		adjdown = orig_dy<<1;

		if (sy>0) {  /* line goes down */
			if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
				firstlen--;
			}
			if (sx>0) { /* line goes right */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb++;
				}
				fb = (FBTYPE *) (((uint8_t *)fb) + w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb++;
					}
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb++;
				}
				return 0;
			} else { /* line goes left */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb--;
				}
				fb = (FBTYPE *) (((uint8_t *)fb) + w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb--;
					}
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				} 
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb--;
				}
				return 0;
			}
		} else { /* line goes up */
			if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) { 
				lastlen--;
			}
			if (sx>0) { /* line goes right */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb++;
				}
				fb = (FBTYPE *) (((uint8_t *)fb) - w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb++;
					}
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb++;
				}
				return 0;
			} else { /* line goes left */
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb--;
				}
				fb = (FBTYPE *) (((uint8_t *)fb) - w);
				for (i=dy-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb--;
					}
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				} 
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb--;
				}
				return 0;
			}
		}
	} else { /* y major */
		int runlen,adjup,adjdown,e,len;
		int firstlen,lastlen;

		runlen = orig_dy/orig_dx;
		adjup = orig_dy%orig_dx;
    
		lastlen = firstlen = (runlen>>1) + 1;
		if (clip_first) { /* clipped, Adjust firstlen */
			int clip_dx = abs(x1 - orig_x1);
			int clip_dy = abs(y1 - orig_y1);
			int d = (2*clip_dx+1)*orig_dy;
			firstlen = d/(2*orig_dx) - clip_dy + 1;
			e = d%(2*orig_dx);
			if ((e==0) && (sx>0)) { /* Special case, arbitrary choise. Select lower pixel.(?) */
				firstlen--;
				e += 2*orig_dx;
			}
			e -= (orig_dx*2);
		} else { /* Not clipped, calculate start error term */
			e = adjup - (orig_dx<<1); /* initial errorterm == half a step */
			if ((runlen&1) != 0) {
				e += orig_dx;
			}
		}
		if (clip_last) { /* Last endpoint clipped */
			int clip_dx = abs(x2 - orig_x2);
			int clip_dy = abs(y2 - orig_y2);
			int d = (1+2*clip_dx)*orig_dy; 
			lastlen = d/(2*orig_dx) - clip_dy + 1;
			if ((sx<0)  && ((d%(2*orig_dx))==0) ) /* special arbitrary case */
				lastlen--;
		}
		adjup <<= 1;
		adjdown = orig_dx<<1;
		if (sy>0) { /* Line goes DOWN */
			if (sx>0) { /* line goes RIGHT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
					firstlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				}
				fb++;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (FBTYPE *) (((uint8_t *)fb) + w);
					}
					fb ++;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				}
				return 0;
			} else { /* line goes LEFT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) {
					lastlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				}
				fb--;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (FBTYPE *) (((uint8_t *)fb) + w);
					}
					fb --;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) + w);
				}
				return 0;
			}
		} else { /* Line goes UP */
			if (sx>0) { /* line goes RIGHT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_first)) {
					firstlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				}
				fb++;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (FBTYPE *) (((uint8_t *)fb) - w);
					}
					fb ++;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				}
				return 0;
			} else { /* line goes LEFT */
				if ((adjup==0) && ((runlen&1)==0) && (!clip_last)) {
					lastlen--;
				}
				for (;firstlen>0; firstlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				}
				fb--;
				for (i=dx-1; i>0; i--) {
					len = runlen;
					e += adjup;
					if (e>=0) {
						len++;
						e -= adjdown;
					}
					for (;len>0; len--) {
						*fb = color;
						fb = (FBTYPE *) (((uint8_t *)fb) - w);
					}
					fb --;
				}
				for (;lastlen>0; lastlen--) {
					*fb = color;
					fb = (FBTYPE *) (((uint8_t *)fb) - w);
				}
				return 0;
			}
		}
	}
}
