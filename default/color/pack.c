/* $Id: pack.c,v 1.2 2002/09/08 21:37:42 soyt Exp $
******************************************************************************

   Generic color packing

   Copyright (C) 1998 Andrew Apted  [andrew@ggi-project.org]

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
#include "color.h"


/* Pack the colors into an array
 */
int GGI_color_L1_packcolors(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint8 *dest = (uint8 *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint8) LIBGGIMapColor(vis, cols);
 	}

	return 0;
}	

int GGI_color_L2_packcolors(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint16 *dest = (uint16 *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint16) LIBGGIMapColor(vis, cols);
 	}

	return 0;
}	

int GGI_color_L3_packcolors(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint8 *dest = (uint8 *) buf;
		
        for (; len > 0; len--, cols++) {
		ggi_pixel pix = LIBGGIMapColor(vis, cols);

		*dest++ = (uint8) pix;  pix >>= 8;
		*dest++ = (uint8) pix;  pix >>= 8;
		*dest++ = (uint8) pix;
 	}

	return 0;
}	

int GGI_color_L4_packcolors(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint32 *dest = (uint32 *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint32) LIBGGIMapColor(vis, cols);
 	}

	return 0;
}	


/* ---------------------------------------------------------------------- */


/* Unpack into the ggi_color array the values of the pixels
 */
int GGI_color_L1_unpackpixels(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint8 *src = (uint8 *) buf;
		
        for (; len > 0; len--, src++, cols++) {
		LIBGGIUnmapPixel(vis, (ggi_pixel) *src, cols);
 	}

	return 0;
}	

int GGI_color_L2_unpackpixels(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint16 *src = (uint16 *) buf;
		
        for (; len > 0; len--, src++, cols++) {
		LIBGGIUnmapPixel(vis, (ggi_pixel) *src, cols);
 	}

	return 0;
}	

int GGI_color_L3_unpackpixels(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint8 *src = (uint8 *) buf;
		
        for (; len > 0; len--, src+=3, cols++) {

		ggi_pixel pix = src[0] | (src[1] << 8) | (src[2] << 16);

		LIBGGIUnmapPixel(vis, pix, cols);
 	}

	return 0;
}	

int GGI_color_L4_unpackpixels(ggi_visual *vis, void *buf, ggi_color *cols, int len)
{
        uint32 *src = (uint32 *) buf;
		
        for (; len > 0; len--, src++, cols++) {
		LIBGGIUnmapPixel(vis, (ggi_pixel) *src, cols);
 	}

	return 0;
}	



/* ---------------------------------------------------------------------- */


#if 0    /* OLDSTUFF.  Will come in handy when implementing the
	  * GT_SUB_PACKED_GETPUT versions of packcolors() &
	  * unpackpixels() in the linear_{1,4} and bitplanar_{1,2,4}
	  * and iplanar_2p{2,4,8} libraries.
	  */
	  
GGIpackcolors(...)
{
	int i;
	
	switch (LIBGGI_MODE(vis)->graphtype) {
	case GT_1BIT:
	{
		uint8 tmp=0,*obuf=(uint8 *)outbuf;
		int mask;
		
		mask=7;
		for (i=0;i<len;i++) {
			tmp |= LIBGGIMapColor(vis,(cols++)) << mask--;
			if (mask<0) {
				*(obuf++)=tmp;
				tmp=0;
				mask=7;
			}
        }
 	}
	break;
	case GT_4BIT:
	{
        uint8 *obuf=(uint8 *)outbuf;
		
        for (i=0;i<len/2;i++)
			*(obuf++)=(uint8)(LIBGGIMapColor(vis,(cols++)) << 4)
				| (uint8)(LIBGGIMapColor(vis,(cols++)));
		
        if (len & 1) {
			*obuf = LIBGGIMapColor(vis, cols) << 4;
        }
 	}
	break;
	case GT_8BIT:
	{
        uint8 *obuf=(uint8 *)outbuf;
		
        for (i=0;i<len;i++)
			*(obuf++)=(uint8)(LIBGGIMapColor(vis,(cols++)));
 	}
	break;
	case GT_15BIT:
	case GT_16BIT:
	{
		uint16 *obuf=(uint16 *)outbuf;
		
		for (i=0;i<len;i++) 
			*(obuf++)=(uint16)(LIBGGIMapColor(vis,(cols++)));
	}
	break;
	case GT_24BIT:
	{
        uint8 *obuf=(uint8 *)outbuf;
        uint32 color;
		
        for (i=0;i<len;i++) {
			color=LIBGGIMapColor(vis,(cols++));
			obuf[0]=color      &0xff;
			obuf[1]=(color>>8) &0xff;
			obuf[2]=(color>>16)&0xff;
			obuf+=3;
        }
	}
	break;
	case GT_32BIT:
	{
        uint32 *obuf=(uint32 *)outbuf;
		
        for (i=0;i<len;i++)
			*(obuf++)=(uint32)(LIBGGIMapColor(vis,(cols++)));
	}
	break;
	default:
		break;
	}

GGIunpackpixels(...)
{
	int i;
	
	switch (GT_SIZE(LIBGGI_MODE(vis)->graphtype)) {
	case 1:
	{
		uint8 *ibuf=(uint8 *)outbuf;
		int mask;
		uint32 tmp;
		
		mask=7;
		for (i=0;i<len;i++) {
			tmp=((*ibuf) >> mask--)&1;
			LIBGGIUnmapPixel(vis,tmp,(cols++));
			if (mask<0) {
				ibuf++;
				mask=7;
			}
		}
	}
	break;
	case 4:
	{
        uint8 *obuf=(uint8 *)outbuf;
        unsigned int tmp;
		
        for (i=0;i<len/2;i++) {
			tmp = *obuf >> 4;
			LIBGGIUnmapPixel(vis,tmp,cols++);
			tmp = *(obuf++) & 0x0F;
			LIBGGIUnmapPixel(vis,tmp,cols++);
        }
		
        if (len & 1) {
			tmp = *obuf >> 4;
			LIBGGIUnmapPixel(vis,tmp,cols++);
        }
  	}
	break;
	case 8:
	{
        uint8 *obuf=(uint8 *)outbuf;
		
        for (i=0;i<len;i++)
			LIBGGIUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	case 16:
	{
		uint16 *obuf=(uint16 *)outbuf;
		
		for (i=0;i<len;i++) 
			LIBGGIUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	case 24:
	{
        uint8 *obuf=(uint8 *)outbuf;
        uint32 color;
		
        for (i=0;i<len;i++) {
			color=obuf[0]+(obuf[1]<<8)+(obuf[2]<<16);
			obuf+=3;
			LIBGGIUnmapPixel(vis,color,(cols++));
		}
	}
	break;
	case 32:
	{
        uint32 *obuf=(uint32 *)outbuf;
		
        for (i=0;i<len;i++)
			LIBGGIUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	default:
		return -1;
	}
	
	return 0;
}	
#endif
