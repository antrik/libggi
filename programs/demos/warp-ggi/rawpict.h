/* $Id: rawpict.h,v 1.3 2005/06/09 18:59:06 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   rawpict.h: definitions for picture files management
  
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

#ifndef  RAWPICT_H
#define  RAWPICT_H

#define RPREAD_NOMEM    -1    /* not enough memory */
#define RPREAD_NOFILE   -2    /* couldn't open file */
#define RPREAD_READERR  -3    /* error while reading data in file */
#define RPREAD_BADFMT   -4    /* picture type not supported by loader */

struct raw_pict {
   int width, height;
   int depth;
   ggi_color *clut;
   void *framebuf;
};

/* Defined in readpcx.c */

extern int readPCX (const char *name, struct raw_pict *rp, uint32 udepth);

/* Defined in readtga.c */

extern int readTGA (const char *name, struct raw_pict *rp, uint32 udepth);

/* Defined in color.c */

extern int convertbpp (struct raw_pict *rp, uint32 udepth);


#endif   /* RAWPICT_H */
