/* $Id: teleserver.c,v 1.1 2004/09/29 13:49:54 cegger Exp $
******************************************************************************

   TELE SERVER.

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]
                 2002 Tobias Hunger   [tobias@fresco.org]

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

	Usage:  teleserver  [OPTIONS...]

	Options:
		-d --display   <display num>
		-t --target    <target spec>
		-h --help
		-V --version

******************************************************************************
*/


#include <ggi/ggi.h>
#include <ggi/gg.h>

#include "libtele.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>


#define VERSION_STRING  "teleserver 0.8 (c) 1998 Andrew Apted"

#define TSERVER_SLEEP_TIME   30    /* milliseconds */
#define TSERVER_FLUSH_TIME  100    /* milliseconds */


/***  Global variables  ***/

static int mode_up = 0;

static ggi_visual_t vis;
static ggi_mode vis_mode;

static char *target_name = NULL;

static TeleServer serv;
static TeleUser *user;

static int display_num = 0;

static int quit;
static int busy;


static void close_connection(int abnormal)
{
	if (abnormal) {
		fprintf(stderr, "Client vanished... connection closed.\n");
		/* Only run once for now. */
		quit = 1;
	}

	if (user) {
		tserver_close(user);
		user = NULL;

		ggiClose(vis);
		mode_up = 0;
	}
}

static void handle_connection(void)
{
	TeleUser *u;

	static int numcon=0;


	u = malloc(sizeof(TeleUser));

	numcon++;

	fprintf(stderr, "Connection %d received.\n", numcon);


	if (user != NULL) {
		fprintf(stderr, "Connection refused.\n");

		/* tserver_refuse() ??? */

		if (tserver_open(&serv, u) == 0) {
			tserver_close(u);
		}
		return;
	}

	if (NULL == target_name)
	  vis = ggiOpen(NULL);
	else
	  vis = ggiOpen(target_name, NULL);

	if (vis == NULL) {
		fprintf(stderr, "teleserver: Couldn't open GGI visual.\n");
		ggiExit();
		exit(2);
	}

	if (tserver_open(&serv, u) < 0) {
		fprintf(stderr, "tserver_open < 0\n");
		ggiClose(vis);
		return;
	}

	user = u;
}

/* pretty crap. oh well */
#define HASH_ORIG(o) ((((o) & 0x1f00) >> 6) | ((o) & 0x03))

static int translate_to_tele(TeleUser *u, TeleEvent *g, ggi_event *ev)
{
	switch (ev->any.type) {

		case evKeyPress:
		case evKeyRepeat:
		case evKeyRelease:
		{
			TeleInpKeyData *d;

			d = tserver_new_event(u, g,
				(ev->any.type == evKeyRelease) ?
				TELE_INP_KEYUP : TELE_INP_KEY,
				sizeof(TeleInpKeyData), 0);

			g->device = TELE_DEVICE_KEYBOARD |
				HASH_ORIG(ev->any.origin);

			d->key     = ev->key.sym;
			d->label   = ev->key.label;
			d->button  = ev->key.button;
			d->modifiers = ev->key.modifiers;

			return 0;
		}

		case evPtrButtonPress:
		case evPtrButtonRelease:
		{
			TeleInpButtonData *d;

			d = tserver_new_event(u, g,
				(ev->any.type == evPtrButtonPress) ?
				TELE_INP_BUTTON : TELE_INP_BUTTONUP,
				sizeof(TeleInpButtonData), 0);

			g->device = TELE_DEVICE_MOUSE |
				HASH_ORIG(ev->any.origin);
			d->button = ev->pbutton.button;

			return 0;
		}

		case evPtrRelative:
		{
			TeleInpAxisData *d;

			d = tserver_new_event(u, g, TELE_INP_MOUSE,
				sizeof(TeleInpAxisData), 4*4);

			g->device = TELE_DEVICE_MOUSE |
				HASH_ORIG(ev->any.origin);

			d->count = 4;

			d->axes[0] = ev->pmove.x;
			d->axes[1] = ev->pmove.y;
			d->axes[2] = ev->pmove.z;
			d->axes[3] = ev->pmove.wheel;

			return 0;
		}

		case evPtrAbsolute:
		{
			TeleInpAxisData *d;

			d = tserver_new_event(u, g, TELE_INP_TABLET,
				sizeof(TeleInpAxisData), 4*4);

			g->device = TELE_DEVICE_TABLET |
				HASH_ORIG(ev->any.origin);

			d->count = 4;

			d->axes[0] = ev->pmove.x;
			d->axes[1] = ev->pmove.y;
			d->axes[2] = ev->pmove.z;
			d->axes[3] = ev->pmove.wheel;

			return 0;
		}

		case evValAbsolute:
		{
			TeleInpAxisData *d;
			int i;

			if ((ev->val.first != 0) ||
			    (ev->val.count < 1) ||
			    (ev->val.count > 8)) {
				return -1;
			}

			d = tserver_new_event(u, g,
				TELE_INP_JOYSTICK,
				sizeof(TeleInpAxisData),
				(signed)(4 * ev->val.count));

			g->device = TELE_DEVICE_JOYSTICK |
				HASH_ORIG(ev->any.origin);

			d->count = ev->val.count;

			for (i=0; i < d->count; i++) {
				d->axes[i] = ev->val.value[i];
			}

			return 0;
		}

		case evExpose:
		{
			tserver_new_event(u, g, TELE_INP_EXPOSE, 0, 0);

			return 0;
		}
	}

	return -1;   /* unknown event */
}


static void handle_event(void)
{
	ggi_event ev;

	TeleEvent g_ev;


	ggiEventRead(vis, &ev, emAll);

	if ((ev.any.type == evKeyPress) &&
	    (ev.key.sym == GIIK_F12)) {

		fprintf(stderr, "ABORTING...\n");
		quit=1;
		return;
	}

	if (translate_to_tele(user, &g_ev, &ev) == 0) {

		int err = tserver_write(user, &g_ev);

		if (err == TELE_ERROR_SHUTDOWN) {

			/* Client has gone away */

			close_connection(1);
			return;
		}
	}
}

static void perf_CHECK(TeleUser *u, TeleEvent *ev)
{
	TeleCmdOpenData *d = (TeleCmdOpenData *) ev->data;

	ggi_mode mode;

	T_Long reply_sequence;

	int err;


	/* get target to check mode */

	mode.graphtype = (uint32) d->graphtype;
	mode.frames    = (uint32) d->frames;
	mode.visible.x = (sint16) d->visible.width;
	mode.visible.y = (sint16) d->visible.height;
	mode.virt.x    = (sint16) d->virt.width;
	mode.virt.y    = (sint16) d->virt.height;
	mode.size.x    = (sint16) d->size.width;
	mode.size.y    = (sint16) d->size.height;
	mode.dpp.x     = (sint16) d->dot.width;
	mode.dpp.y     = (sint16) d->dot.height;

	d->error = ggiCheckMode(vis, &mode);

	/* send result back to client */

	reply_sequence = ev->sequence;

	tserver_new_event(u, ev, TELE_CMD_CHECK,
			  sizeof(TeleCmdOpenData), 0);

	ev->sequence = reply_sequence;

	d->graphtype      = (T_Long) mode.graphtype;
	d->frames         = (T_Long) mode.frames;
	d->visible.width  = (T_Long) mode.visible.x;
	d->visible.height = (T_Long) mode.visible.y;
	d->virt.width     = (T_Long) mode.virt.x;
	d->virt.height    = (T_Long) mode.virt.y;
	d->size.width     = (T_Long) mode.size.x;
	d->size.height    = (T_Long) mode.size.y;
	d->dot.width      = (T_Long) mode.dpp.x;
	d->dot.height     = (T_Long) mode.dpp.y;

	err = tserver_write(u, ev);

	if (err == TELE_ERROR_SHUTDOWN) {

		/* Client has gone away */

		close_connection(1);
		return;
	}
}

static void perf_OPEN(TeleUser *u, TeleEvent *ev)
{
	TeleCmdOpenData *d = (TeleCmdOpenData *) ev->data;

	T_Long reply_sequence;

	int err;


	/* get target to check mode */

	vis_mode.graphtype = (uint32) d->graphtype;
	vis_mode.frames    = (uint32) d->frames;
	vis_mode.visible.x = (sint16) d->visible.width;
	vis_mode.visible.y = (sint16) d->visible.height;
	vis_mode.virt.x    = (sint16) d->virt.width;
	vis_mode.virt.y    = (sint16) d->virt.height;
	vis_mode.dpp.x     = (sint16) d->dot.width;
	vis_mode.dpp.y     = (sint16) d->dot.height;

	d->error = ggiSetMode(vis, &vis_mode);

	if (! d->error) {
		mode_up = 1;
	}


	/* send result back to client */

	reply_sequence = ev->sequence;

	tserver_new_event(u, ev, TELE_CMD_OPEN,
			  sizeof(TeleCmdOpenData), 0);

	ev->sequence = reply_sequence;

	d->graphtype      = (T_Long) vis_mode.graphtype;
	d->frames         = (T_Long) vis_mode.frames;
	d->visible.width  = (T_Long) vis_mode.visible.x;
	d->visible.height = (T_Long) vis_mode.visible.y;
	d->virt.width     = (T_Long) vis_mode.virt.x;
	d->virt.height    = (T_Long) vis_mode.virt.y;
	d->dot.width      = (T_Long) vis_mode.dpp.x;
	d->dot.height     = (T_Long) vis_mode.dpp.y;

	err = tserver_write(u, ev);

	if (err == TELE_ERROR_SHUTDOWN) {

		/* Client has gone away */

		close_connection(1);
		return;
	}
}

static void perf_GETPIXELFMT(TeleUser *u, TeleEvent *ev)
{
	TeleCmdPixelFmtData *d = (TeleCmdPixelFmtData *) ev->data;
	T_Long reply_sequence;
	int err;
	
	const ggi_pixelformat * format;
	format = ggiGetPixelFormat(vis);

	reply_sequence = ev->sequence;
	tserver_new_event(u, ev, TELE_CMD_GETPIXELFMT,
			  sizeof(TeleCmdPixelFmtData), 0);
	ev->sequence = reply_sequence;

	/* send result back to client */

	d = (TeleCmdPixelFmtData *)ev->data;

	d->depth          = (T_Long) format->depth;
	d->size           = (T_Long) format->size;
	
	d->red_mask       = (T_Long) format->red_mask;
	d->green_mask     = (T_Long) format->green_mask;
	d->blue_mask      = (T_Long) format->blue_mask;
	d->alpha_mask     = (T_Long) format->alpha_mask;
	d->clut_mask      = (T_Long) format->clut_mask;
	d->fg_mask        = (T_Long) format->fg_mask;
	d->bg_mask        = (T_Long) format->bg_mask;
	d->texture_mask   = (T_Long) format->texture_mask;
	d->flags          = (T_Long) format->flags;
	d->stdformat      = (T_Long) format->stdformat;

	err = tserver_write(u, ev);

	if (err == TELE_ERROR_SHUTDOWN) {
		/* Client has gone away */

		close_connection(1);
		return;
	}
}

static void perf_CLOSE(TeleUser *u)
{
	fprintf(stderr, "Client closed.\n");

	close_connection(0);
}


static void perf_FLUSH(TeleUser *u)
{
	ggiFlush(vis);
}	/* perf_FLUSH */


static void perf_PUTBOX(TeleUser *u, TeleCmdGetPutData *d)
{
	/* Put a pixel matrix */
	ggi_pixel *src = (ggi_pixel *)d->pixel;

	if ((d->x < 0) || (d->y < 0) ||
	    (d->x + d->width  > vis_mode.virt.x) ||
	    (d->y + d->height > vis_mode.virt.y)) {

		fprintf(stderr, "teleserver: ILLEGAL PUTBOX (%d,%d) "
			"%dx%d.\n", (int) d->x, (int) d->y,
			(int) d->width, (int) d->height);
		return;
	}	/* if */

	ggiPutBox(vis, d->x, d->y, d->width, d->height, src);
}	/* perf_PUTBOX */


static void perf_GETBOX(TeleUser *u, TeleEvent *ev)
{
	/* Get a pixel matrix */
	TeleCmdGetPutData *d = (TeleCmdGetPutData *) ev->data;
	T_Long reply_sequence;
	uint8 *dest;

	if ((d->x < 0) || (d->y < 0) ||
	    (d->x + d->width  >= vis_mode.virt.x) ||
	    (d->y + d->height >= vis_mode.virt.y))
	{
		fprintf(stderr, "teleserver: ILLEGAL GETBOX.\n");
		return;
	}	/* if */

	if ((d->width * d->height * d->bpp) >
	    ((ev->size - ev->rawstart) * (signed)sizeof(long)))
	{
		fprintf(stderr, "teleserver: NOT ENOUGH ROOM FOR GETBOX.\n");
		return;
	}	/* if */

	reply_sequence = ev->sequence;

	tserver_new_event(u, ev, TELE_CMD_GETBOX,
		sizeof(TeleCmdGetPutData)-4, d->width * d->height * d->bpp);

	ev->sequence = reply_sequence;

	dest = (uint8 *)d->pixel;

	ggiGetBox(vis, d->x, d->y, d->width, d->height, dest);

	tserver_write(u, ev);
}	/* perf_GETBOX */


static void perf_DRAWBOX(TeleUser *u, TeleCmdDrawBoxData *d)
{
	/* Put a solid box */

	if ((d->x < 0) || (d->y < 0) ||
	    (d->x + d->width  > vis_mode.virt.x) ||
	    (d->y + d->height > vis_mode.virt.y))
	{
		fprintf(stderr, "teleserver: ILLEGAL DRAWBOX.\n");
		return;
	}	/* if */

	ggiSetGCForeground(vis, (ggi_pixel)d->pixel);
	ggiDrawBox(vis, d->x, d->y, d->width, d->height);
}	/* perf_DRAWBOX */


static void perf_COPYBOX(TeleUser *u, TeleCmdCopyBoxData *d)
{
	/* Copy a box on-screen */

	if ((d->sx < 0) || (d->sy < 0) ||
	    (d->dx < 0) || (d->dy < 0) ||
	    (d->width <= 0) || (d->height <= 0) ||
	    (d->sx + d->width  > vis_mode.virt.x) ||
	    (d->sy + d->height > vis_mode.virt.y) ||
	    (d->dx + d->width  > vis_mode.virt.x) ||
	    (d->dy + d->height > vis_mode.virt.y))
	{
		fprintf(stderr, "teleserver: ILLEGAL COPYBOX.\n");
		return;
	}	/* if */

	ggiCopyBox(vis, d->sx, d->sy, d->width, d->height, d->dx, d->dy);
}	/* perf_COPYBOX */


static void perf_DRAWLINE(TeleUser *u, TeleCmdDrawLineData *d)
{
	/* draw a solid line */

	ggiSetGCForeground(vis, (ggi_pixel)d->pixel);
	ggiDrawLine(vis, d->x, d->y, d->xe, d->ye);
}	/* perf_DRAWLINE */


static void perf_SETORIGIN(TeleUser *u, TeleCmdSetOriginData *d)
{
	/* !!! validate */

	ggiSetOrigin(vis, d->x, d->y);
}


static void perf_SETPALETTE(TeleUser *u, TeleCmdSetPaletteData *d)
{
	int i;

	/* !!! validate */

	for (i=0; i < d->len; i++, d->start++) {

		ggi_color col;

		col.r = (d->colors[i] & 0xff0000) >> 8;
		col.g = (d->colors[i] & 0x00ff00)     ;
		col.b = (d->colors[i] & 0x0000ff) << 8;

		/* horribly inefficient */

		ggiSetPalette(vis, d->start, 1, &col);
	}
}


static void perf_PUTSTR(TeleUser *u, TeleCmdPutStrData *d)
{
	/* Note: If your compiler fails here, then it
	 * is _not_ ANSI C99 compliant
	 */
	char s[d->length + 1];
	int i;

	for(i = 0; i <= d->length; ++i) {
		s[i] = (char)(d->text[i] & 0xFF);
	}	/* for */

	ggiSetGCForeground(vis, (ggi_pixel)d->fg);
	ggiSetGCBackground(vis, (ggi_pixel)d->bg);
	ggiPuts(vis, d->x, d->y, s);
}


static void perf_GETCHARSIZE(TeleUser *u, TeleEvent *ev)
{
	TeleCmdGetCharSizeData *d = (TeleCmdGetCharSizeData *) ev->data;
	T_Long reply_sequence;


	reply_sequence = ev->sequence;

	tserver_new_event(u, ev, TELE_CMD_GETCHARSIZE,
			sizeof(TeleCmdGetCharSizeData), 0);

	ev->sequence = reply_sequence;
	ggiGetCharSize(vis, &d->width, &d->height);

	tserver_write(u, ev);
}	/* perf_GETCHARSIZE */



static void handle_command(TeleUser *u)
{
	TeleEvent ev;

	int err;


	err = tserver_read(u, &ev);

	if (err == TELE_ERROR_SHUTDOWN) {

		/* Client has gone away */

		close_connection(1);
		return;
	}

	if (err == TELE_ERROR_BADEVENT) {
		fprintf(stderr,  "teleserver: Error reading event.\n");
		return;
	}

	if (err < 0) {
		fprintf(stderr, "teleserver: Unspecified read error.\n");
		return;
	}

	if (((ev.type & TELE_EVENT_TYPE_MASK) != TELE_INP_BASE) &&
	    ((ev.type & TELE_EVENT_TYPE_MASK) != TELE_CMD_BASE)) {

		fprintf(stderr, "teleserver: unrecognised event "
			"(0x%08x).\n", (int) ev.type);
		return;
	}

	switch (ev.type) {

		case TELE_CMD_CHECK:
			perf_CHECK(u, &ev);
			break;

		case TELE_CMD_OPEN:
			perf_OPEN(u, &ev);
			break;
		
	        case TELE_CMD_GETPIXELFMT:
		        perf_GETPIXELFMT(u, &ev);
                        break;

		case TELE_CMD_CLOSE:
			perf_CLOSE(u);
			break;

		case TELE_CMD_FLUSH:
			perf_FLUSH(u);
			break;

		case TELE_CMD_PUTBOX:
			perf_PUTBOX(u, (TeleCmdGetPutData *) ev.data);
			break;

		case TELE_CMD_GETBOX:
			perf_GETBOX(u, &ev);
			break;

		case TELE_CMD_DRAWBOX:
			perf_DRAWBOX(u, (TeleCmdDrawBoxData *) ev.data);
			break;

		case TELE_CMD_COPYBOX:
			perf_COPYBOX(u, (TeleCmdCopyBoxData *) ev.data);
			break;

		case TELE_CMD_DRAWLINE:
			perf_DRAWLINE(u, (TeleCmdDrawLineData *) ev.data);
			break;

		case TELE_CMD_SETORIGIN:
			perf_SETORIGIN(u, (TeleCmdSetOriginData *) ev.data);
			break;

		case TELE_CMD_SETPALETTE:
			perf_SETPALETTE(u, (TeleCmdSetPaletteData *) ev.data);
			break;

		case TELE_CMD_PUTSTR:
			perf_PUTSTR(u, (TeleCmdPutStrData *) ev.data);
			break;

		case TELE_CMD_GETCHARSIZE:
			perf_GETCHARSIZE(u, &ev);
			break;

		default:
			fprintf(stderr, "teleserver: UNKNOWN EVENT "
				"(0x%08x)\n", (int) ev.type);
			break;
	}
}

static void check_command(TeleUser *u)
{
	if (tserver_poll(u)) {
		handle_command(u);
		busy++;
	}
}

static void usage(void)
{
	fprintf(stderr,
		"\nUSAGE:  teleserver  [OPTIONS...]\n\n"
		"OPTIONS:\n"
		"	-d --display   <display num>\n"
		"	-t --target    <target spec>\n"
		"	-h --help\n"
		"	-V --version\n\n");
}

static int handle_args(int argc, char **argv)
{
	argc--; argv++;

#	define CMP_OPT(x, short, long)  \
		((strcmp((x), short) == 0) || (strcmp((x), long) == 0))

	while (argc > 0) {

		if (CMP_OPT(argv[0], "-h", "--help")) {
			usage();
			return -2;
		}

		if (CMP_OPT(argv[0], "-V", "--version")) {
			fprintf(stderr, VERSION_STRING "\n");
			return -2;
		}

		if ((argc > 1) && CMP_OPT(argv[0], "-d", "--display")) {
			sscanf(argv[1], "%d", &display_num);
			argc -= 2; argv += 2;
			continue;
		}

		if ((argc > 1) && CMP_OPT(argv[0], "-t", "--target")) {
			target_name = argv[1];
			argc -= 2; argv += 2;
			continue;
		}

		/* unknown option */

		fprintf(stderr, "\nteleserver: Unknown option '%s'.\n",
			argv[0]);
		fprintf(stderr, "(use --help for usage summary).\n\n");

		return -1;
	}

#	undef CMP_OPT

	return 0;
}

int main(int argc, char *argv[])
{
	struct timeval cur_time;
	struct timeval flush_time;

	/* initialize */

	if (handle_args(argc, argv) < 0) {
		return 1;
	}

	/* open visual */

	if (ggiInit() < 0) {
		fprintf(stderr, "teleserver: Error initializing GGI.\n");
		return 1;
	}

	if (tserver_init(&serv, display_num) < 0) {
		ggiExit();
		return 3;
	}

	ggCurTime(&flush_time);

	printf("TeleServer Ready.\n");
	fflush(stdout);


	/* main loop */

	for (quit=0; !quit; ) {
		struct timeval tv;

		int millies;

		busy=0;

		/* check for incoming connections */

		if (tserver_check(&serv)) {
			handle_connection();
			busy++;
		}

		/* check for user input */

		tv.tv_sec = tv.tv_usec = 0;

		if (mode_up && (ggiEventPoll(vis, emAll, &tv) != 0)) {
			handle_event();
			busy++;
		}

		/* check for client commands */

		if (user) {
			check_command(user);
		}

		if (! busy) {
			ggUSleep(TSERVER_SLEEP_TIME * 1000);
		}

		ggCurTime(&cur_time);


		millies = (cur_time.tv_sec  - flush_time.tv_sec)  * 1000 +
		          (cur_time.tv_usec - flush_time.tv_usec) / 1000;

		if (millies >= TSERVER_FLUSH_TIME) {
			if (mode_up) {
				ggiFlush(vis);
			}
			flush_time = cur_time;
		}
	}

	/* shut down */
	tserver_exit(&serv);

	ggiExit();

	return 0;
}
