/* $Id: flying_ggis.c,v 1.12 2005/07/30 11:58:39 cegger Exp $
******************************************************************************

   Flying-GGIs - Another neat GGI demo...

   Authors:	1998 	  Andrew Apted		[andrew@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************

   This is a demonstration of LibGGI's functions and can be used as a 
   reference programming example.

******************************************************************************
*/

/* This is needed for the HAVE_* macros */
#include "config.h"
#ifndef HAVE_RANDOM
# define random		rand
#endif

#include <ggi/ggi.h>
#include <ggi/gg.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <time.h>
#include <math.h>
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#include "banner.inc.c"


#define fixed  int32_t   /* 16.16 */

#define DEFAULT_WIDTH   320
#define DEFAULT_HEIGHT  200

#define DEFAULT_GENTIME      200   /* msec */
#define DEFAULT_MAXSIZE      100   /* percent */
#define DEFAULT_CLUSTERSIZE  100   /* percent */
#define DEFAULT_SPEED        30 

/* Global Info */

static int screen_width;
static int screen_height;
static int screen_diag;

static int banner_width;
static int banner_height;
static int banner_diag;

static int gen_time;	  /* msec */
static fixed max_size;    /* default 1.0 = full screen */
static int cluster_size;  /* pixels */
static int fixed_speed=0;
static int speed;
static int async = 1;

static char *target_str = NULL;
static ggi_visual_t vis;
static ggi_mode vis_mode;

static int use_putbox;

static ggi_pixel lookup[256];

static void *image_buf;
static size_t image_size;


typedef struct texture
{
        struct texture *succ;
        struct texture *tail;

        fixed mid_x, mid_y;
        fixed size;
        fixed millis;
        fixed speed;

        uint8_t color;
} Texture;

static Texture *texture_list;


static void banner_size(int *width, int *height)
{
        *width=0;

        for (*height=0; banner[*height] != NULL; (*height)++) {

                int len = strlen(banner[*height]);

                if (len > *width) {
                        *width = len;
		}
        }
}

static int random_in_range(int low, int high)
{
	return low + random() % (high-low+1);
}

static void setup_palette(void)
{
	int i;

	use_putbox = 0;

	if (GT_SCHEME(vis_mode.graphtype) == GT_PALETTE) {

		ggiSetColorfulPalette(vis);

		if (GT_DEPTH(vis_mode.graphtype) == 8) {
			use_putbox = 1;
		}
	}

	for (i=0; i < 256; i++) {

		ggi_color col;

		col.r = ((i >> 5) & 7) * 0xffff / 7;
		col.g = ((i >> 2) & 7) * 0xffff / 7;
		col.b = ((i)      & 3) * 0xffff / 3;

		lookup[i] = ggiMapColor(vis, &col);
	}
}

static uint8_t trans_buffer[8192];

static void translate_hline(int x, int y, int w, uint8_t *data)
{
	int ww = w;

	uint8_t  *buf1 = (uint8_t  *) trans_buffer;
	uint16_t *buf2 = (uint16_t *) trans_buffer;
	uint32_t *buf4 = (uint32_t *) trans_buffer;

	switch (GT_ByPP(vis_mode.graphtype)) {
	case 1:
		for (; ww > 0; ww--) {
			*buf1++ = lookup[*data++];
		}
		break;

	case 2:
		for (; ww > 0; ww--) {
			*buf2++ = lookup[*data++];
		}
		break;

	case 3:
		for (; ww > 0; ww--) {
			ggi_pixel pix = lookup[*data++];

			*buf1++ = pix; pix >>= 8;
			*buf1++ = pix; pix >>= 8;
			*buf1++ = pix;
		}
		break;

	case 4:
		for (; ww > 0; ww--) {
			*buf4++ = lookup[*data++];
		}
		break;
	}
	
	ggiPutHLine(vis, x, y, w, trans_buffer);
}

static void update_frame(void)
{
	if (use_putbox) {
		ggiPutBox(vis, 0, 0, screen_width, screen_height, image_buf);
	} else {
		int y;
		uint8_t *src = (uint8_t *) image_buf;

		for (y=0; y < screen_height; y++) {
			translate_hline(0, y, screen_width, src);
			src += screen_width;
		}
	}

	if(async)
		ggiFlush(vis);
}

static void init_textures(void)
{
        texture_list = NULL;
}

static void free_textures(void)
{
	Texture *t;

        while (texture_list != NULL) {
		
		t = texture_list;
		texture_list = t->succ;
		
		free(t);
	}
	
}

static void add_texture(int x, int y, uint8_t color)
{
        Texture *t;

        t = (Texture *) malloc(sizeof(Texture));

        t->mid_x = x << 16;
        t->mid_y = y << 16;

        t->millis = 0;
        t->color  = color;
        t->speed  = speed + (fixed_speed ? 0 :
			random_in_range(-(speed/2), +(speed/2)));
        t->succ = texture_list;
        texture_list = t;
}

static void render_texture(int width, int height, Texture *t)
{
        int x, y;
        int sx, sy, bx;
        int dx, dy;

        uint8_t *dest;

        height <<= 16;
        width  <<= 16;

        dx = dy = t->size * screen_diag / banner_diag;

        bx = t->mid_x - (banner_width  * dx / 2);
        sy = t->mid_y - (banner_height * dy / 2);

        for (y=0; (banner[y] != NULL) && (sy < height); y++, sy += dy) {

		const char *pos = banner[y];

                if (sy >= 0) {

                        dest = image_buf;
                        dest += ((sy>>16) * screen_width);

                        for (x=0, sx=bx; (*pos != 0) && (sx < width); 
			     x++, pos++, sx += dx) {
			
                                if ((sx >= 0) && (*pos == '#'))
                                {
                                        dest[sx>>16] = t->color;
                                }
                        }
                }
        }
}

static void update_texture(Texture *t, Texture ***prev_ptr, int millis)
{
        t->millis += millis;

        t->size = t->millis * t->speed;

        if (t->size > max_size) {
                
                /* remove texture */

                **prev_ptr = t->succ;

		free(t);

                return;
        }

        *prev_ptr = &t->succ;

        render_texture(screen_width, screen_height, t);
}

static void update_all_textures(int millis)
{
        Texture *cur;
        Texture **prev_ptr;

        cur = texture_list;
        prev_ptr = (Texture **) &texture_list;

        while (cur != NULL) {

                update_texture(cur, &prev_ptr, millis);

                cur = *prev_ptr;
        }
}

static void show_usage(char *progname)
{
	/* remove leading paths */

	if (strrchr(progname, '/') != NULL) {
		progname = strrchr(progname, '/') + 1;
	}

	printf("\nUSAGE: "
		"%s [OPTION]...\n\n"
		"Options:\n"
		"    -h, --help\n" 
		"    -m, --mode      <mode spec>\n" 
		"    -t, --target    <target spec>\n"
		"    -g, --gentime   <generation time>\n"
		"    -s, --speed     <speed>\n"
		"    -z, --size      <percentage>\n"
		"    -c, --cluster   <percentage>\n"
		"    -f, --fixed\n\n"
		"Press 's' to toggle (a)synchronous drawing.\n\n",
		progname);
}

int main(int argc, char **argv)
{ 
        int gen_millis;

        struct timeval prev_time, cur_time;

	int x, y, i;


        /* initialize */

        srand((unsigned)time(NULL));

        banner_size(&banner_width, &banner_height);

        banner_diag = sqrt((double)(banner_width * banner_width +
                              banner_height * banner_height));

        screen_width  = DEFAULT_WIDTH;
        screen_height = DEFAULT_HEIGHT;

        gen_time = DEFAULT_GENTIME;

        speed        = -1;
        max_size     = -1;
	cluster_size = -1;

        init_textures();

	ggiParseMode("", &vis_mode);

        
	/* handle arguments */

	for (i=1; i < argc; i++) {

#define CMPOPT(x,s,l,n)  (((strcmp(x,s)==0) || \
			   (strcmp(x,l)==0)) && ((i+n) < argc))

		if (CMPOPT(argv[i], "-h", "--help", 0)) {
		    
			show_usage(argv[0]);
			exit(EXIT_SUCCESS);
		}

		if (CMPOPT(argv[i], "-m", "--mode", 1)) {
		    
			i++; ggiParseMode(argv[i], &vis_mode);
			continue;
		}

		if (CMPOPT(argv[i], "-t", "--target", 1)) {

			i++; target_str = argv[i];
			continue;
		}

		if (CMPOPT(argv[i], "-g", "--gentime", 1)) {

			i++; gen_time = atoi(argv[i]);
			continue;
		}

		if (CMPOPT(argv[i], "-s", "--speed", 1)) {

			i++; speed = atoi(argv[i]);
			continue;
		}

		if (CMPOPT(argv[i], "-z", "--size", 1)) {

			i++; max_size = atoi(argv[i]);
			continue;
		}

		if (CMPOPT(argv[i], "-c", "--cluster", 1)) {

			i++; cluster_size = atoi(argv[i]);
			continue;
		}

		if (CMPOPT(argv[i], "-f", "--fixed", 0)) {

			fixed_speed = 1;
			continue;
		}
#undef CMPOPT

		fprintf(stderr, "Unknown option '%s'\n", argv[i]);
		exit(EXIT_FAILURE);
	}


	/* sort out parameters */

	if (speed < 0) {
		speed = DEFAULT_SPEED;
	}

	if (max_size < 0) {
		max_size = DEFAULT_MAXSIZE;
	}

	max_size = (max_size << 16) / 100;

	if (cluster_size < 0) {
		cluster_size = DEFAULT_CLUSTERSIZE;
	}


        /* setup graphics mode */

	if (ggiInit() != 0) {
		fprintf(stderr, "%s: unable to initialize LibGGI, exiting.\n",
			argv[0]);
		exit(EXIT_FAILURE);
	}

        vis = ggiOpen(target_str, NULL);

        if (vis == NULL) {
                ggiPanic("Failed to open visual.\n");
        }

	if(async)
		ggiSetFlags(vis, GGIFLAG_ASYNC);
	
	ggiCheckMode(vis, &vis_mode);

        if (ggiSetMode(vis, &vis_mode) < 0) {
		ggiPanic("%s: mode refused.\n", argv[0]);
	}
	
        setup_palette();

	screen_width  = vis_mode.visible.x;
	screen_height = vis_mode.visible.y;

        screen_diag = sqrt((double)(screen_width  * screen_width +
			   screen_height * screen_height));

        image_size = screen_width * screen_height * 1;

        image_buf = malloc(image_size);

	x = screen_width/2;
	y = screen_width/2;


        /* main loop */

        ggCurTime(&prev_time);

        gen_millis=0;

        for(;;) {
                int millis;

                struct timeval tv = { 0, 10000 };

                if (ggiEventPoll(vis, emKeyPress|emPtrRelative, &tv)) {
		
			ggi_event ev;
			int done=0;

			ggiEventRead(vis, &ev, emKeyPress|emPtrRelative);

			if (ev.any.type == evKeyPress) {
				switch (ev.key.sym) {
				case GIIUC_Escape:
				case 'q': case 'Q':
				case 'x': case 'X':
					done=1;
					break;
				case 's': case 'S':
					async = !async;
					ggiSetFlags(vis,
						async ? GGIFLAG_ASYNC : 0);
					break;
				}
			}

			if (ev.any.type == evPtrRelative) {
				x += ev.pmove.x;
				y += ev.pmove.y;
			}

			if (done) {
				break;
			}
                }

                /* determine time lapse */

                ggCurTime(&cur_time);

                millis = (cur_time.tv_sec  - prev_time.tv_sec)  * 1000
                       + (cur_time.tv_usec - prev_time.tv_usec) / 1000;

                prev_time = cur_time;


		if (use_putbox) {
			memset(image_buf, (signed)lookup[0], image_size);
		} else {
			memset(image_buf, 0,         image_size);
		}

                update_all_textures(millis);

                update_frame();

                
                for (gen_millis += millis; 
		     gen_millis >  gen_time;
                     gen_millis -= gen_time) {

			int disp_x = screen_width  * cluster_size / 200;
			int disp_y = screen_height * cluster_size / 141;

			x += random_in_range(-disp_x, +disp_x);
			y += random_in_range(-disp_y, +disp_y);

			if (x < 0) {
				x += screen_width;
			}
			if (y < 0) {
				y += screen_height;
			}
			if (x >= screen_width) {
				x -= screen_width;
			}
			if (y >= screen_height) {
				y -= screen_height;
			}

                        add_texture(x, y, random_in_range(0, 255));
                }
        }

        free_textures();

        ggiClose(vis);
        ggiExit();
	return 0;
}
