/* $Id: warp.h,v 1.3 2005/07/30 11:58:39 cegger Exp $
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
   int32_t width, height, pixsize;
   int32_t srclinelen, destlinelen;

   void *offstable;
   int32_t *disttable;
   void *source;
   void *framebuf;

   int32_t ctable [1024];
   int32_t sintable [1024+256];
};


/* Defined in dowarp.c */

extern struct warp *initWarp (uint32_t width, uint32_t height, uint32_t pixsize,
                              void *source, uint32_t srclinelen);
extern void disposeWarp (struct warp *w);
extern void doWarp8bpp  (struct warp *w, int32_t xw, int32_t yw, int32_t cw);
extern void doWarp16bpp (struct warp *w, int32_t xw, int32_t yw, int32_t cw);
extern void doWarp24bpp (struct warp *w, int32_t xw, int32_t yw, int32_t cw);
extern void doWarp32bpp (struct warp *w, int32_t xw, int32_t yw, int32_t cw);



#endif   /* WARP_H */
