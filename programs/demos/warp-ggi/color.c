/* $Id: color.c,v 1.1 2001/05/12 23:03:41 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   color.c: color depth conversion
  
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

#include <stdio.h>
#include <stdlib.h>
#include <ggi/ggi.h>
#include "rawpict.h"

int convertbpp (struct raw_pict *rp, uint32 udepth) {
   int i, pixels = rp->width * rp->height;
   uint8 *bp, *bp2, x;
   uint16 *wp, r,g,b;
   uint32 *lp;
   ggi_color *clut;

   switch (rp->depth) {
      case 8:
         if ( !(clut = rp->clut)) return (0);

         switch (udepth) {
            case 15:
               wp = ((uint16 *) rp->framebuf) + pixels;
               bp = ((uint8 *) rp->framebuf) + pixels;

               for (i=0; i<pixels; i++) {
                  x = *(--bp);
                  *(--wp) =
                     (clut[x].r & 0xF800) >> 1 |
                     (clut[x].g & 0xF800) >> 6 |
                     (clut[x].b & 0xF800) >> 11;
               }
               break;

            case 16:					
               wp = ((uint16 *) rp->framebuf) + pixels;
               bp = ((uint8 *) rp->framebuf) + pixels;

               for (i=0; i<pixels; i++) {
                  x = *(--bp);
                  *(--wp) =
                     (clut[x].r & 0xF800) |
                     (clut[x].g & 0xFC00) >> 5 |
                     (clut[x].b & 0xF800) >> 11;
               }
               break;

            case 24:
               bp2 = ((uint8 *) rp->framebuf) + 3*pixels;
               bp = ((uint8 *) rp->framebuf) + pixels;

               for (i=0; i<pixels; i++) {
                  x = *(--bp);

                  *(--bp2) = clut[x].r >> 8;
                  *(--bp2) = clut[x].g >> 8;
                  *(--bp2) = clut[x].b >> 8;
               }
               break;

            case 32:
               lp = ((uint32 *) rp->framebuf) + pixels;
               bp = ((uint8 *) rp->framebuf) + pixels;

               for (i=0; i<pixels; i++) {
                  x = *(--bp);

                  *(--lp) = clut[x].r << 8 |
                            clut[x].g |
                            clut[x].b >> 8;
               }
               break;
         }
         break;

   case 24:
      switch (udepth) {
         case 15:
            wp = ((uint16 *) rp->framebuf);
            bp = ((uint8 *) rp->framebuf);
                
            for (i=0; i<pixels; i++) {
               r = *bp++;
               g = *bp++;
               b = *bp++;

               *wp++ = (r & 0xF8) << 7 |
                       (g & 0xF8) << 2 |
                       (b & 0xF8) >> 3;
            }
	    break;

         case 16:
            wp = ((uint16 *) rp->framebuf);
            bp = ((uint8 *) rp->framebuf);
                                        
            for (i=0; i<pixels; i++) {
               r = *bp++;
               g = *bp++;
               b = *bp++;

               *wp++ = (r & 0xF8) << 8 |
                       (g & 0xFC) << 3 |
                       (b & 0xF8) >> 3;
            }
            break;

         case 32:
            lp = ((uint32 *) rp->framebuf) + pixels; 
            bp = ((uint8 *) rp->framebuf) + 3*pixels;
        
            for (i=0; i<pixels; i++) {
               b = *(--bp);
               g = *(--bp);
               r = *(--bp);
               
               *(--lp) = r << 16 |
                         g << 8 |
                         b;
            }
            break;
      }
      break;
   }

   return 1;
}
