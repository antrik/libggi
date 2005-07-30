/* $Id: readpcx.c,v 1.6 2005/07/30 11:58:39 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   readpcx.c : PCX files reader	
   PCX original code by some cow-orker I forgot :)
  
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

#include "config.h"
#include <ggi/ggi.h>
#include <stdio.h>
#include <stdlib.h>

#include "rawpict.h"

struct pcx_header {
	uint8_t manufacturer;
	uint8_t version;
	uint8_t encoding;
	uint8_t bits_per_pixel;
	uint8_t xmin[2], ymin[2];
	uint8_t xmax[2], ymax[2];
	uint8_t hres[2];
	uint8_t vres[2];

	uint8_t palette16[48];
	uint8_t reserved;
	uint8_t color_planes;
	uint8_t bytes_per_line[2];
	uint8_t palette_type[2];
	uint8_t filler[58];
};

#define get16(x) ((x[1]<<8)|x[0])

int readPCX(const char *name, struct raw_pict *rp, uint32_t udepth)
{
	FILE *f;
	struct pcx_header header;
	uint32_t bpp;
	int32_t p, c, err;
	uint32_t i, j;
	uint32_t width, height, cpl;
	ggi_color *palette;
	uint8_t *fbuf, *lptr, *pptr, *nextlptr, palbuf[4];

	err = 0;

	if ((f = fopen(name, "rb"))) {
		fread((char *) &header, 1, sizeof(struct pcx_header), f);

		width = get16(header.xmax) - get16(header.xmin) + 1;
		height = get16(header.ymax) - get16(header.ymin) + 1;

		rp->width = width;
		rp->height = height;

		switch (udepth) {
		case 32:
			bpp = 4;
			break;
		case 24:
			bpp = 3;
			break;
		case 16:
			bpp = 2;
			break;
		case 15:
			bpp = 2;
			break;
		default:
			bpp = 1;
			break;
		}

		cpl = header.color_planes;

		if ((header.bits_per_pixel != 8)
		    || ((cpl != 1) && (cpl != 3)))
			err = RPREAD_BADFMT;

		if (!err)
			if (bpp < cpl)
				bpp = cpl;

		if ((!err)
		    && (fbuf = (uint8_t *) malloc(width * height * bpp))) {
			rp->framebuf = (void *) fbuf;
			lptr = fbuf;

			if (cpl == 1) {
				rp->depth = 8;

				for (i = 0; i < height; ++i) {
					nextlptr = lptr + width;

					do {
						c = fgetc(f) & 0xFF;
						if ((c & 0xC0) == 0xC0) {
							j = c & 0x3F;
							c = fgetc(f);
							while (j--)
								*lptr++ =
								    c;
						} else
							*lptr++ = c;
					} while (lptr < nextlptr);

					lptr = nextlptr;
				}
			} else {
				rp->depth = 24;

				for (i = 0; i < height; ++i) {
					nextlptr = lptr + width * 3;
					pptr = lptr;
					p = 0;

					do {
						c = fgetc(f) & 0xFF;
						if ((c & 0xC0) == 0xC0) {
							j = c & 0x3F;
							c = fgetc(f);

							while (j--) {
								*pptr = c;
								pptr += 3;
								if (pptr >=
								    nextlptr)
								{
									p++;
									pptr = lptr + p;
								}
							}
						} else {
							*pptr = c;
							pptr += 3;
							if (pptr >=
							    nextlptr) {
								p++;
								pptr =
								    lptr +
								    p;
							}
						}
					} while (p < 3);

					lptr = nextlptr;
				}
			}

			if (fgetc(f) == 12) {
				if ((palette =
				     (ggi_color *) malloc(256 *
							  sizeof
							  (ggi_color)))) {
					for (i = 0; i < 256; i++) {
						fread(palbuf, 1, 3, f);
						palette[i].r =
						    palbuf[0] << 8;
						palette[i].g =
						    palbuf[1] << 8;
						palette[i].b =
						    palbuf[2] << 8;
					}
					rp->clut = palette;
				} else
					err = RPREAD_NOMEM;
			} else
				rp->clut = NULL;

			if (err)
				free(fbuf);
		} else if (!err)
			err = RPREAD_NOMEM;

		fclose(f);
	} else
		err = RPREAD_NOFILE;

	return (err);
}
