/* $Id: readtga.c,v 1.2 2003/07/05 11:35:58 cegger Exp $
******************************************************************************
  
   Warp-GGI
 
   General Graphics Interface demo: realtime picture 'gooing'
   Written by Emmanuel Marty <core@ggi-project.org>

   readtga.c: TGA files reader
   TGA code adapted from Abuse, game by Crack Dot Com
   Contributed by Jaromir Koutek <miri@punknet.cz>
  
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
#include <string.h>
#include <ggi/ggi.h>
#include "rawpict.h"

#include "config.h"

struct TargaHeader {
	unsigned char id;
	unsigned char color_map;
	unsigned char im_type;
	char gap[9];
	unsigned short W;
	unsigned short H;
	unsigned char bpp;
	unsigned char im_des;
};

static inline int get16(int x)
{
	uint8 *p = (uint8 *) & x;
	return (p[1] << 8) | p[0];
}

static inline void StoreBPP(uint8 * data, int value, unsigned int bpp)
{
	memcpy(data, &value, bpp);
}

static inline int RGBconv(int r, int g, int b, unsigned int bpp)
{
	switch (bpp) {
	case 4:
	case 3:
		return (r << 16) | (g << 8) | b;
	case 2:
		return ((r & 0xf8) << 8) | ((g & 0xfc) << 3) | (b >> 3);
	default:
		return (r & 0xe0) | ((g >> 3) & 0x1e) | (b >> 6);
	}
}

int readTGA(char *name, struct raw_pict *rp, uint32 udepth)
{
	FILE *is;
	struct TargaHeader th;
	int W, H;
	uint8 *data;
	unsigned char ctrl;
	unsigned char bgra[4];
	uint8 *sl, *end_p;
	uint32 bpp;
	int c, x, y;

	if (!(is = fopen(name, "rb")))
		return RPREAD_NOFILE;

	fread(&th, sizeof(th), 1, is);

	if (th.color_map)
		return RPREAD_BADFMT;

	if (!(th.im_type == 2 || th.im_type == 10))
		return RPREAD_BADFMT;

	if ((th.bpp != 32) && (th.bpp != 24))
		return RPREAD_BADFMT;

	fseek(is, (long) th.id, SEEK_CUR);	/* skip image ID - emmanuel */
	W = get16(th.W);
	H = get16(th.H);

	rp->width = W;
	rp->height = H;
	rp->clut = NULL;

	switch (udepth) {
	case 32:
		bpp = 4;
		break;
	case 24:
		bpp = 3;
		break;
	case 8:
		bpp = 1;
		break;

	default:
		bpp = 2;
		break;
	}
	rp->depth = bpp << 3;

	if (!(data = (uint8 *) malloc(W * H * bpp)))
		return RPREAD_NOMEM;

	end_p = data + W * H * bpp;
	rp->framebuf = (void *) data;

	sl = data;

	for (y = 0; y < H && sl != end_p; y++) {
		for (x = 0; x < W && sl != end_p;) {

			if (th.im_type == 2) {
				fread(&bgra, (th.bpp == 32 ? 4U : 3U), 1,
				      is);
				c = RGBconv(bgra[2], bgra[1], bgra[0],
					    bpp);
				StoreBPP(sl, c, bpp);
				sl += bpp;
				x++;

			} else {
				fread(&ctrl, sizeof(ctrl), 1, is);

				/* RLE decompression */

				if (ctrl & 0x80) {
					fread(&bgra,
					      (th.bpp == 32 ? 4U : 3U), 1,
					      is);
					ctrl &= (~0x80);
					ctrl++;
					c = RGBconv(bgra[2], bgra[1],
						    bgra[0], bpp);

					while (ctrl-- && sl != end_p) {
						StoreBPP(sl, c, bpp);
						sl += bpp;
						x++;
					}

				} else {

					/* not compressed */

					ctrl++;
					while (ctrl-- && sl != end_p) {
						fread(&bgra,
						      (th.bpp ==
						       32 ? 4U : 3U), 1, is);
						c = RGBconv(bgra[2],
							    bgra[1],
							    bgra[0], bpp);
						StoreBPP(sl, c, bpp);
						sl += bpp;
						x++;
					}
				}

			}
			if (x >= W) {
				y++;
				x -= W;
			}
		}
	}

	fclose(is);
	return 0;
}
