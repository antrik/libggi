/* $Id: structs.c,v 1.2 2003/02/14 15:37:51 fries Exp $
******************************************************************************

   This is a GGI test application.

   Copyright (C) 1998 Andreas Beck	[becka@ggi-project.org]
  
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ggi/ggi.h>


static ggi_visual_t visual;

/**********************************************************************/

/* Leftover. Up to now, we have only one struct test, but we might want
 * to revive that ...
 */
struct test 
{	char *name;
	void (*func)(void);
	int active;
} tests[]=
{	
	{ NULL,NULL,0 }
};

/* Display usage info.
 */
void
usage(const char *prog)
{
	fprintf(stderr,"Usage:\n\n"
		       "%s [-flags] [--tests]\n\n"
		       "Supported flags are :\n",prog);

	fprintf(stderr,	"-? list available tests\n");

	exit(1);
}

/* Display usage info. List available tests.
 */
void
list_tests(void)
{
	int testnum;
	
	fprintf(stderr,"Available tests are :\n");
	
	for (testnum=0;tests[testnum].name;testnum++)
	{
		fprintf(stderr,"--%s\n",tests[testnum].name);
	}
}

/* Check arguments.
 */
int
parse_args(int argc,char **argv)
{
	int x,testnum;

	for (x=1;x<argc;x++) {
		if (*argv[x]=='-') {
			switch(argv[x][1]) {

				case '?': list_tests();return 1;
				case '-':
					for (testnum=0;tests[testnum].name;
					     testnum++) {
						if (strcmp(tests[testnum].name,
							   argv[x]+2)==0 ||
						    strcmp("all", argv[x]+2)
						    ==0) {
							tests[testnum].active=1;
						}
					}
					break;
				default:
					fprintf(stderr,
						"%s: Unknown switch '%s' !\n\n",
						argv[0],argv[x]);
					usage(argv[0]);
					return 1;
			}
		}
		else {
			fprintf(stderr,"%s: Can't parse '%s'.\n\n",argv[0],argv[x]);
			usage(argv[0]);
			return 1;
		}
	}	

	return 0;
}

void
print_pixfmt(const ggi_pixelformat *pixfmt)
{
	int i;
	if (pixfmt == NULL) {
		fprintf(stderr, "  pixfmt is a NULL pointer!\n");
		fflush(stderr);
		return;
	}

	printf("  depth = %2d, size = %2d, flags = 0x%08x\n"
	       "  stdformat = 0x%08x\n"
	       "  red_mask     = 0x%08x, red_shift     = %2d\n"
	       "  green_mask   = 0x%08x, green_shift   = %2d\n"
	       "  blue_mask    = 0x%08x, blue_shift    = %2d\n"
	       "  alpha_mask   = 0x%08x, alpha_shift   = %2d\n"
	       "  clut_mask    = 0x%08x, clut_shift    = %2d\n"
	       "  fg_mask      = 0x%08x, fg_shift      = %2d\n"
	       "  bg_mask      = 0x%08x, bg_shift      = %2d\n"
	       "  texture_mask = 0x%08x, texture_shift = %2d\n",
	       pixfmt->depth, pixfmt->size, pixfmt->flags,
	       pixfmt->stdformat,
	       pixfmt->red_mask, pixfmt->red_shift,
	       pixfmt->green_mask, pixfmt->green_shift,
	       pixfmt->blue_mask, pixfmt->blue_shift,
	       pixfmt->alpha_mask, pixfmt->alpha_shift,
	       pixfmt->clut_mask, pixfmt->clut_shift,
	       pixfmt->fg_mask, pixfmt->fg_shift,
	       pixfmt->bg_mask, pixfmt->bg_shift,
	       pixfmt->texture_mask, pixfmt->texture_shift);
	for (i = 0; i < pixfmt->size; i++) {
		printf("Bit %d: 0x%08x\n", i, pixfmt->bitmeaning[i]);
	}
}


int db_doacquire(const ggi_directbuffer *dbuf)
{
	if (ggiResourceAcquire(dbuf->resource,
			       GGI_ACTYPE_READ | GGI_ACTYPE_WRITE) == 0) {
		return 2;
	}
	if (ggiResourceAcquire(dbuf->resource, GGI_ACTYPE_WRITE) == 0) {
		return 1;
	}
	if (ggiResourceAcquire(dbuf->resource, GGI_ACTYPE_READ) == 0) {
		return 0;
	}
	return -1;
}


/* Set the default mode and do all the work. We might want to split 
 * that later ...
 */
int
setup_mode(void)
{
	int i;
	ggi_color col;
	const ggi_directbuffer *buf;
	ggi_mode gmode;

	if ((visual=ggiOpen(NULL)) == NULL) {
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		return -1;
	}

	ggiCheckSimpleMode(visual, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO,
			   &gmode);
	if (ggiSetMode(visual, &gmode) == 0) {
		printf("Graph mode %dx%d (%dx%d virt)\n",
		       gmode.visible.x, gmode.visible.y,
		       gmode.virt.x, gmode.virt.y);
	} else {
		fprintf(stderr, "Unable to set any mode at all!\n");
		ggiClose(visual);
		return -1;
	}

	col.r = 0xFFFF;
	col.g = 0xFFFF;
	col.b = 0xFFFF;
	printf("white = 0x%x\n", ggiMapColor(visual, &col));

	col.r = 0x0000;
	col.g = 0x0000;
	col.b = 0x0000;
	printf("black = 0x%x\n", ggiMapColor(visual, &col));

	printf("Pixelformat for Get/Put buffers:\n");
	print_pixfmt(ggiGetPixelFormat(visual));

	for (i = 0; ; i++) {
		int acquired = 0;

		buf = ggiDBGetBuffer(visual, i);
		if (buf == NULL) break;

		printf("DirectBuffer (frame #%d):\n", buf->frame);
		if (ggiResourceMustAcquire(buf->resource)) {
			switch (db_doacquire(buf)) {
			case 2:
				printf("Acquired DirectBuffer read/write\n");
				acquired = 1;
				break;
			case 1:
				printf("Acquired DirectBuffer for writing\n");
				acquired = 1;
				break;
			case 0:
				printf("Acquired DirectBuffer for read-only\n");
				acquired = 1;
				break;
			default:
				printf("DirectBuffer can not be Acquired!\n");
				break;
			}
		} else {
			printf("Does not need to be acquired\n");
		}

		printf("Mapped at read:%p, write:%p (paged %d)\n",
		       buf->read, buf->write, buf->page_size);
		switch (buf->layout) {
			case blPixelLinearBuffer: 
				printf("Layout: Linear Pixel Buffer\n");
				printf("Stride=%d\n", buf->buffer.plb.stride);
				printf("Pixelformat:\n");
				print_pixfmt(buf->buffer.plb.pixelformat);
				break;
			default: 
				printf("Layout: Unknown\n");
				break;	/* Skip it. Don't know it. */
		}
		if (acquired) ggiResourceRelease(buf->resource);
	}

	return 0;
}

/* Main function. Check parameters. Set up mode. Run tests.
 */
int
main(int argc,char **argv)
{
	int testnum;

	if (parse_args(argc,argv)) return 1;

	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(1);
	}

	if (setup_mode()) {
		ggiExit();
		return 2;
	}
	
	for (testnum=0;tests[testnum].name;testnum++) {
		if (!tests[testnum].active) continue;
		tests[testnum].func();
	}

	ggiClose(visual);
	ggiExit();	

	return 0;
}
