/* $Id: pack.c,v 1.9 2008/01/20 22:14:55 pekberg Exp $
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
int GGI_color_L1_packcolors(struct ggi_visual *vis, void *buf, const ggi_color *cols, int len)
{
        uint8_t *dest = (uint8_t *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint8_t) _ggiMapColor(vis, cols);
 	}

	return 0;
}	

int GGI_color_L2_packcolors(struct ggi_visual *vis, void *buf, const ggi_color *cols, int len)
{
        uint16_t *dest = (uint16_t *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint16_t) _ggiMapColor(vis, cols);
 	}

	return 0;
}	

int GGI_color_L3_packcolors(struct ggi_visual *vis, void *buf, const ggi_color *cols, int len)
{
        uint8_t *dest = (uint8_t *) buf;
		
        for (; len > 0; len--, cols++) {
		ggi_pixel pix = _ggiMapColor(vis, cols);

		*dest++ = (uint8_t) pix;  pix >>= 8;
		*dest++ = (uint8_t) pix;  pix >>= 8;
		*dest++ = (uint8_t) pix;
 	}

	return 0;
}	

int GGI_color_L4_packcolors(struct ggi_visual *vis, void *buf, const ggi_color *cols, int len)
{
        uint32_t *dest = (uint32_t *) buf;
		
        for (; len > 0; len--, cols++) {
		*dest++ = (uint32_t) _ggiMapColor(vis, cols);
 	}

	return 0;
}	


/* ---------------------------------------------------------------------- */


/* Unpack into the ggi_color array the values of the pixels
 */
int GGI_color_L1_unpackpixels(struct ggi_visual *vis, const void *buf, ggi_color *cols, int len)
{
	const uint8_t *src = (const uint8_t *)buf;
		
        for (; len > 0; len--, src++, cols++) {
		_ggiUnmapPixel(vis, (const ggi_pixel) *src, cols);
 	}

	return 0;
}	

int GGI_color_L2_unpackpixels(struct ggi_visual *vis, const void *buf, ggi_color *cols, int len)
{
	const uint16_t *src = (const uint16_t *)buf;
		
        for (; len > 0; len--, src++, cols++) {
		_ggiUnmapPixel(vis, (const ggi_pixel) *src, cols);
 	}

	return 0;
}	

int GGI_color_L3_unpackpixels(struct ggi_visual *vis, const void *buf, ggi_color *cols, int len)
{
	const uint8_t *src = (const uint8_t *) buf;
		
        for (; len > 0; len--, src+=3, cols++) {

		ggi_pixel pix = src[0] | (src[1] << 8) | (src[2] << 16);

		_ggiUnmapPixel(vis, pix, cols);
 	}

	return 0;
}	

int GGI_color_L4_unpackpixels(struct ggi_visual *vis, const void *buf, ggi_color *cols, int len)
{
	const uint32_t *src = (const uint32_t *) buf;
		
        for (; len > 0; len--, src++, cols++) {
		_ggiUnmapPixel(vis, (const ggi_pixel) *src, cols);
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
	
	switch (LIBGGI_GT(vis)) {
	case GT_1BIT:
	{
		uint8_t tmp=0,*obuf=(uint8_t *)outbuf;
		int mask;
		
		mask=7;
		for (i=0;i<len;i++) {
			tmp |= _ggiMapColor(vis,(cols++)) << mask--;
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
        uint8_t *obuf=(uint8_t *)outbuf;
		
        for (i=0;i<len/2;i++)
			*(obuf++)=(uint8_t)(_ggiMapColor(vis,(cols++)) << 4)
				| (uint8_t)(_ggiMapColor(vis,(cols++)));
		
        if (len & 1) {
			*obuf = _ggiMapColor(vis, cols) << 4;
        }
 	}
	break;
	case GT_8BIT:
	{
        uint8_t *obuf=(uint8_t *)outbuf;
		
        for (i=0;i<len;i++)
			*(obuf++)=(uint8_t)(_ggiMapColor(vis,(cols++)));
 	}
	break;
	case GT_15BIT:
	case GT_16BIT:
	{
		uint16_t *obuf=(uint16_t *)outbuf;
		
		for (i=0;i<len;i++) 
			*(obuf++)=(uint16_t)(_ggiMapColor(vis,(cols++)));
	}
	break;
	case GT_24BIT:
	{
        uint8_t *obuf=(uint8_t *)outbuf;
        uint32_t color;
		
        for (i=0;i<len;i++) {
			color=_ggiMapColor(vis,(cols++));
			obuf[0]=color      &0xff;
			obuf[1]=(color>>8) &0xff;
			obuf[2]=(color>>16)&0xff;
			obuf+=3;
        }
	}
	break;
	case GT_32BIT:
	{
        uint32_t *obuf=(uint32_t *)outbuf;
		
        for (i=0;i<len;i++)
			*(obuf++)=(uint32_t)(_ggiMapColor(vis,(cols++)));
	}
	break;
	default:
		break;
	}

GGIunpackpixels(...)
{
	int i;
	
	switch (GT_SIZE(LIBGGI_GT(vis))) {
	case 1:
	{
		uint8_t *ibuf=(uint8_t *)outbuf;
		int mask;
		uint32_t tmp;
		
		mask=7;
		for (i=0;i<len;i++) {
			tmp=((*ibuf) >> mask--)&1;
			_ggiUnmapPixel(vis,tmp,(cols++));
			if (mask<0) {
				ibuf++;
				mask=7;
			}
		}
	}
	break;
	case 4:
	{
        uint8_t *obuf=(uint8_t *)outbuf;
        unsigned int tmp;
		
        for (i=0;i<len/2;i++) {
			tmp = *obuf >> 4;
			_ggiUnmapPixel(vis,tmp,cols++);
			tmp = *(obuf++) & 0x0F;
			_ggiUnmapPixel(vis,tmp,cols++);
        }
		
        if (len & 1) {
			tmp = *obuf >> 4;
			_ggiUnmapPixel(vis,tmp,cols++);
        }
  	}
	break;
	case 8:
	{
        uint8_t *obuf=(uint8_t *)outbuf;
		
        for (i=0;i<len;i++)
			_ggiUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	case 16:
	{
		uint16_t *obuf=(uint16_t *)outbuf;
		
		for (i=0;i<len;i++) 
			_ggiUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	case 24:
	{
        uint8_t *obuf=(uint8 *)outbuf;
        uint32_t color;
		
        for (i=0;i<len;i++) {
			color=obuf[0]+(obuf[1]<<8)+(obuf[2]<<16);
			obuf+=3;
			_ggiUnmapPixel(vis,color,(cols++));
		}
	}
	break;
	case 32:
	{
        uint32_t *obuf=(uint32_t *)outbuf;
		
        for (i=0;i<len;i++)
			_ggiUnmapPixel(vis,*(obuf++),(cols++));
	}
	break;
	default:
		return GGI_ENOMATCH;
	}
	
	return 0;
}	
#endif
