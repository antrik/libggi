/* $Id: warp.h,v 1.2 2003/07/05 11:35:58 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   warp.h: definitions for warping
  
   This is a demonstration of LibGGI's functions and can be used as a
   reference programming example.
  
   This software is placed in the public domain and can be used
   freely for any purpose. It comes with absolutely NO WARRANTY,
   either expressed or implied, including, but not limited to the
   implied warranties of merchantability or fitness for a particular
   purpose.  USE IT AT YOUR OWN RISK. The author is not responsible
   for any damage or consequences raised by use or inability to use
   this program.

******************************************************************************
*/

#ifndef  WARP_H
#define  WARP_H

#include <math.h>
#ifndef M_PI
# define M_PI	3.141592654
#endif

struct warp {
   sint32 width, height, pixsize;
   sint32 srclinelen, destlinelen;

   void *offstable;
   sint32 *disttable;
   void *source;
   void *framebuf;

   sint32 ctable [1024];
   sint32 sintable [1024+256];
};


/* Defined in dowarp.c */

extern struct warp *initWarp (uint32 width, uint32 height, uint32 pixsize,
                              void *source, uint32 srclinelen);
extern void disposeWarp (struct warp *w);
extern void doWarp8bpp  (struct warp *w, sint32 xw, sint32 yw, sint32 cw);
extern void doWarp16bpp (struct warp *w, sint32 xw, sint32 yw, sint32 cw);
extern void doWarp24bpp (struct warp *w, sint32 xw, sint32 yw, sint32 cw);
extern void doWarp32bpp (struct warp *w, sint32 xw, sint32 yw, sint32 cw);



#endif   /* WARP_H */
