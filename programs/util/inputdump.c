/* $Id: inputdump.c,v 1.9 2004/08/09 11:39:08 pekberg Exp $
******************************************************************************

   inputdump.c - display input events

   Authors:	1998 Andrew Apted	[andrew@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************

   This program outputs (to stderr) information about the GII input
   devices present and the events they produce, whilst showing some
   half-useful stuff on the screen.

   USAGE:  inputdump  [ --mode <mode> ]  [ --target <target> ]
                      [ --input <input> ]  [ --long ]  [ --short ]
                      [ --no-pmove ]

******************************************************************************
*/

#include <ggi/ggi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "giik2str.h"

static struct timeval start_time;
static int firstevent = 1;
static enum { SHOW_NIL, SHOW_SHORT, SHOW_LONG } do_show = SHOW_NIL;
static int no_pmove = 0;

static ggi_visual_t vis;
static ggi_mode vis_mode;
static ggi_coord vis_ch;
static int placement = 0;


#define MAX_NR_DEV	128
#define MAX_NR_VAL	128
#define MAX_DRAW_VAL	128
#define MAX_NR_BUT	512

typedef struct mydev_info
{
	uint32 origin;
	int known;

	gii_cmddata_getdevinfo DI;
	gii_cmddata_getvalinfo *VI[MAX_NR_VAL];
	
	sint32 axes[MAX_NR_VAL+4];
	sint32 buttons[MAX_NR_BUT];  /* labels */
	sint32 pbuttons[MAX_NR_BUT];  /* ptr-buttons */

	ggi_coord top;
	int val_h, ptr_h;
	int val_x;

	sint32 cur_x, cur_y, cur_z, cur_w;

} mydev_info;

static gii_valrange default_range = { -200, 0, 200 };

static mydev_info *InputDevices[MAX_NR_DEV] = { NULL, };

static mydev_info *cur_dev = NULL;


static ggi_color black  = { 0x0000, 0x0000, 0x0000 };
static ggi_color blue   = { 0x0000, 0x0000, 0xffff };
static ggi_color green  = { 0x0000, 0xffff, 0x0000 };
static ggi_color cyan   = { 0x0000, 0xffff, 0xffff };
static ggi_color red    = { 0xffff, 0x0000, 0x0000 };
static ggi_color yellow = { 0xffff, 0xffff, 0x0000 };
static ggi_color orange = { 0xffff, 0x8888, 0x0000 };
static ggi_color white  = { 0xffff, 0xffff, 0xffff };


/* ------------------------------------------------------------------------ */


#define CALC_POS(r,v,w)  ((int) ((w) * (   \
	(float) ((v) - (r)->min) /          \
	(float) ((r)->max - (r)->min)        \
	)))

static void draw_bar(int x, int y, sint32 val, gii_valrange *range)
{
	int w = vis_ch.x * 13;
	int h = vis_ch.y * 2 / 3 + 1;

	int left, right, center;
	int overflow = 0;


	if (val < range->min) {
		val = range->min; overflow = 1;
	}
	if (val > range->max) {
		val = range->max; overflow = 1;
	}

	left   = CALC_POS(range, val, w);
	center = CALC_POS(range, range->center, w);

	if (left < center) {
		right = center;
	} else {
		right = 1+left;
		left  = 1+center;
	}

	ggiSetGCForeground(vis, ggiMapColor(vis, &black));
	ggiDrawBox(vis, x, y, w+1, h);

	ggiSetGCForeground(vis, ggiMapColor(vis, overflow ? &orange : &red));
	ggiDrawBox(vis, x + left, y, right-left, h);

	ggiSetGCForeground(vis, ggiMapColor(vis, &blue));
	ggiDrawVLine(vis, x + center, y, h);
}

static void draw_inp_valuator(mydev_info *M, int n)
{
	gii_valrange *range;

	if (M->VI[n]) {
		range = &M->VI[n]->range;
	} else {
		range = &default_range;
	}

	draw_bar(M->top.x + vis_ch.x * M->val_x,
		 M->top.y + vis_ch.y * (n+1),
		 M->axes[n], range);
}

static void draw_inp_mouserel(mydev_info *M, int n, int change)
{
	gii_valrange range = { -64, 0, +64 };

	draw_bar(M->top.x + vis_ch.x * M->val_x,
		 M->top.y + vis_ch.y * (1 + M->val_h + n),
		 change, &range);
}

static void draw_inp_buttons(mydev_info *M)
{
	int x = M->top.x;
	int y = M->top.y + vis_ch.y * (M->val_h + M->ptr_h + 1);

	char buf[40];

	int i;

	ggiSetGCBackground(vis, ggiMapColor(vis, &black));
	ggiSetGCForeground(vis, ggiMapColor(vis, &green));

	for (i=0; i < MAX_NR_BUT; i++) {
		int label = M->buttons[i];

		if (label < 0) continue;

		if ((label == GIIK_NIL) || (label == GIIK_VOID)) {
			sprintf(buf, "button %03d        ", i);
		} else {
			sprintf(buf, "button %03d 0x%04x ", i, label);
		}
		
		ggiPuts(vis, x, y, buf);  y += vis_ch.y;
	}

	for (i=0; i < MAX_NR_BUT; i++) {
		int label = M->pbuttons[i];

		if (label < 0) continue;

		sprintf(buf, "ptrbtn %03d        ", i);
		
		ggiPuts(vis, x, y, buf);  y += vis_ch.y;
	}

	ggiPuts(vis, x, y, "                    ");
}

static void draw_inp_device(mydev_info *M)
{
	char buf[40];

	int i;
	int w = 0;

	sprintf(buf, "Unknown 0x%04x", M->origin);
	
	ggiSetGCBackground(vis, ggiMapColor(vis, &black));
	ggiSetGCForeground(vis, ggiMapColor(vis, &yellow));

	ggiPuts(vis, M->top.x, M->top.y, "              ");
	ggiPuts(vis, M->top.x, M->top.y, M->known ? M->DI.longname : buf);

	if (M->val_h > 0) {
		for (i=0; i < M->val_h; i++) {
			int tmp;
			if (!M->VI[i]) {
				sprintf(buf, "? %d", i);
				tmp = strlen(buf);
			}
			else
				tmp = strlen(M->VI[i]->longname);
			if (tmp > w)
				w = tmp;
		}
	}
	if (M->ptr_h > 0 && w < 2)
		w = 2;
	if (w > (vis_mode.virt.x / 2) / vis_ch.x - 14) {
		w = (vis_mode.virt.x / 2) / vis_ch.x - 14;
		if (w < 0)
			w = 0;
	}
	if (w > sizeof(buf) - 1)
		w = sizeof(buf) - 1;
	M->val_x = w;

	if (M->val_h > 0) {
		for (i=0; i < M->val_h; i++) {
			int y = M->top.y + (i+1) * vis_ch.y;

			ggiSetGCBackground(vis, ggiMapColor(vis, &black));
			ggiSetGCForeground(vis, ggiMapColor(vis, &white));

			memset(buf, ' ', M->val_x);
			buf[M->val_x] = '\0';
			ggiPuts(vis, M->top.x, y, buf);
			sprintf(buf, "? %d", i);
			ggiPuts(vis, M->top.x, y, M->VI[i] ? 
				M->VI[i]->longname : buf);

			draw_inp_valuator(M, i);
		}
	}

	if (M->ptr_h > 0) {
		int y = M->top.y + (M->val_h + 1) * vis_ch.y;

		ggiSetGCBackground(vis, ggiMapColor(vis, &black));
		ggiSetGCForeground(vis, ggiMapColor(vis, &cyan));

		ggiPuts(vis, M->top.x, y, "mx   ");  y += vis_ch.y;
		ggiPuts(vis, M->top.x, y, "my   ");  y += vis_ch.y;
		ggiPuts(vis, M->top.x, y, "mz   ");  y += vis_ch.y;
		ggiPuts(vis, M->top.x, y, "mw   ");  y += vis_ch.y;
	}
}

static void calc_placement(ggi_coord *c)
{
	ggi_coord t;

	int p = placement;

	t.x = vis_mode.virt.x / 2;
	t.y = vis_mode.virt.y / 2;
	
	c->x = c->y = 0;

	while (p != 0) {
		if (p & 1) c->x += t.x;
		if (p & 2) c->y += t.y;

		t.x /= 2; t.y /= 2; p >>= 2;
	}
}

static mydev_info *find_input_device(uint32 origin)
{
	unsigned int i, j;

	if ((origin == GII_EV_ORIGIN_NONE) ||
	    (origin & GII_EV_ORIGIN_SENDEVENT)) {
		return NULL;
	}

	for (i=0; i < MAX_NR_DEV; i++) {
		if (InputDevices[i] && InputDevices[i]->origin == origin) {
			/* found it */
			return InputDevices[i];
		}
	}

	/* not found - add a new one */

	for (i=0; i < MAX_NR_DEV; i++) {
		if (! InputDevices[i]) {
			break;
		}
	}

	if (i >= MAX_NR_DEV) {
		fprintf(stderr, "Too many input devices !\n");
		return NULL;
	}

	InputDevices[i] = (mydev_info *) malloc(sizeof(mydev_info));

	InputDevices[i]->origin = origin;
	InputDevices[i]->known  = 0;

	calc_placement(& InputDevices[i]->top);
	placement++; 

	memset(InputDevices[i]->axes, 0, (MAX_NR_VAL+4) * sizeof(sint32));
	for (j = 0; j < MAX_NR_BUT; j++) {
		InputDevices[i]->buttons[j] = -1;
		InputDevices[i]->pbuttons[j] = -1;
	}
	
	InputDevices[i]->val_h = 0;
	InputDevices[i]->ptr_h = 0;

	InputDevices[i]->cur_x = 0;
	InputDevices[i]->cur_y = 0;
	InputDevices[i]->cur_z = 0;
	InputDevices[i]->cur_w = 0;

	draw_inp_device(InputDevices[i]);

	return InputDevices[i];
}


/* ------------------------------------------------------------------------ */


#if 0
#define TO_SI(v,n)					   \
	((v->SI_mul == 0) ? (n) :			   \
	 (float) (v->SI_add + (n)) * (float) v->SI_mul /   \
	 (float) v->SI->SI_div * pow(2.0, v->SI_shift))
#endif

static void show_common(gii_event *ev)
{
	int sec, usec;

	if (firstevent) {
		start_time.tv_sec = ev->any.time.tv_sec;
		start_time.tv_usec = ev->any.time.tv_usec;
		firstevent = 0;
	}

	sec  = ev->any.time.tv_sec  - start_time.tv_sec;
	usec = ev->any.time.tv_usec - start_time.tv_usec;

	while (usec < 0) {
		usec += 1000000;
		sec--;
	}

	fprintf(stderr, "    size=0x%02x origin=0x%08x "
		"time=%d.%06d err=%d\n", ev->any.size, ev->any.origin,
		sec, usec, ev->any.error);
}

static void show_key(gii_key_event *ev)
{
	if (do_show != SHOW_NIL) {
		fprintf(stderr, "button=0x%02x label=%s sym=%s "
			"modifiers=0x%02x\n", ev->button,
			giik2str(ev->label, 0),	giik2str(ev->sym, 1),
			ev->modifiers);
	}

	/* update display */

	if (cur_dev) {
		int b = ev->button;

		if ((b == GII_BUTTON_NIL) || (b >= MAX_NR_BUT)) 
			return;

		if (ev->type == evKeyRelease) {
			cur_dev->buttons[b] = -1;
		} else {
			cur_dev->buttons[b] = ev->label;
		}

		draw_inp_buttons(cur_dev);
	}
}

static void show_pmove(gii_pmove_event *ev)
{
	if (!no_pmove && do_show != SHOW_NIL) {
		fprintf(stderr, "x=%-3d y=%-3d z=%-3d wheel=%-3d\n",
			ev->x, ev->y, ev->z, ev->wheel);
	}

	/* update display */

	if (cur_dev) {
		int dx = (ev->type == evPtrRelative) ?
			ev->x : (ev->x - cur_dev->cur_x);
		int dy = (ev->type == evPtrRelative) ?
			ev->y : (ev->y - cur_dev->cur_y);
		int dz = (ev->type == evPtrRelative) ?
			ev->z : (ev->z - cur_dev->cur_z);
		int dw = (ev->type == evPtrRelative) ?
			ev->wheel : (ev->wheel - cur_dev->cur_w);

		if (cur_dev->ptr_h != 4) {
			cur_dev->ptr_h = 4;
			draw_inp_device(cur_dev);
		}

		draw_inp_mouserel(cur_dev, 0, dx);
		draw_inp_mouserel(cur_dev, 1, dy);
		draw_inp_mouserel(cur_dev, 2, dz);
		/* We multiply by eight to make the bar more visible */
		draw_inp_mouserel(cur_dev, 3, dw*8);

		cur_dev->cur_x += dx;  cur_dev->cur_y += dy;
		cur_dev->cur_z += dz;  cur_dev->cur_w += dw;
	}
}

static void show_pbutton(gii_pbutton_event *ev)
{
	if (do_show != SHOW_NIL) {
		fprintf(stderr, "ptrbutton=0x%02x\n", ev->button);
	}

	/* update display */

	if (cur_dev) {
		int b = ev->button;

		if ((b == GII_PBUTTON_NONE) || (b >= MAX_NR_BUT)) 
			return;

		cur_dev->pbuttons[b] = (ev->type == evPtrButtonRelease) ? 
					-1 : GIIK_VOID;

		draw_inp_buttons(cur_dev);
	}
}

static void show_valuator(gii_val_event *ev)
{
	uint32 i;

	if (do_show != SHOW_NIL) {
		fprintf(stderr, "0x%02x..0x%02x =", ev->first,
			ev->first+ev->count-1);

		for(i=0; i < ev->count; i++) {
			fprintf(stderr, " %-6d", ev->value[i]);
		}

		fprintf(stderr, "\n");
	}

	/* update display */

	if (cur_dev) for (i=0; i < ev->count; i++) {

		int n = ev->first + i;
	
		if (n >= MAX_NR_VAL) continue;

		if (ev->type == evValRelative) {
			cur_dev->axes[n] += ev->value[i];
		} else {
			cur_dev->axes[n] = ev->value[i];
		}

		draw_inp_valuator(cur_dev, n);
	}
}

static void show_devinfo(gii_cmd_event *ev)
{	
	gii_event qev;
	gii_cmddata_getdevinfo *DI = (void *) ev->data;
	gii_cmddata_getvalinfo *VI = (void *) &qev.cmd.data;
	
	int s = (do_show != SHOW_NIL);

	if (ev->origin & GII_EV_ORIGIN_SENDEVENT) {
		if (s) fprintf(stderr, "(empty query)\n");
		return;
	}
	
	if (s) {
	  fprintf(stderr, "short='%s' long='%s'\n"
		"    axes=%d buttons=%d generate=0x%06x\n", 
		DI->shortname, DI->longname,
		DI->num_axes, DI->num_buttons, DI->can_generate);
	}

	/* ask device for valuator info */

	if (DI->num_axes > 0) {
		qev.any.size   = sizeof(gii_cmd_event);
		qev.any.type   = evCommand;
		qev.any.origin = GII_EV_ORIGIN_NONE;
		qev.any.target = GII_EV_TARGET_ALL;
		qev.cmd.code   = GII_CMDCODE_GETVALINFO;

		VI->number = GII_VAL_QUERY_ALL;
		
		giiEventSend(ggiJoinInputs(vis, NULL), &qev);
	}

	/* update display */

	if (cur_dev) {
		cur_dev->DI = *DI;
		cur_dev->known = 1;

		if (cur_dev->DI.num_axes >= MAX_NR_VAL) {
			cur_dev->DI.num_axes = MAX_NR_VAL-1;
		}
		if (cur_dev->DI.num_buttons >= MAX_NR_BUT) {
			cur_dev->DI.num_buttons = MAX_NR_BUT-1;
		}

		cur_dev->val_h = cur_dev->DI.num_axes;
		if (cur_dev->val_h > MAX_DRAW_VAL) {
			/* Only draw this many valuators. */
			cur_dev->val_h = MAX_DRAW_VAL;
		}

		if (cur_dev->DI.can_generate & emPtrMove) {
			cur_dev->ptr_h = 4;
		}

		draw_inp_device(cur_dev);
	}
}

static void show_valinfo(gii_cmd_event *ev)
{
	int s = (do_show != SHOW_NIL);

	gii_cmddata_getvalinfo *VI = (void *) ev->data;
	
	if (ev->origin & GII_EV_ORIGIN_SENDEVENT) {
		if (s) fprintf(stderr, "(empty query)\n");
		return;
	}
	
	if (s) {
	  fprintf(stderr, "num=0x%02x short='%s' long='%s'\n"
		"    raw_range=%d..%d..%d unit=",
		VI->number, VI->shortname, VI->longname,
		VI->range.min, VI->range.center, VI->range.max);

	  switch (VI->phystype) {
	    case GII_PT_TIME:            fprintf(stderr, "s");       break;
	    case GII_PT_FREQUENCY:       fprintf(stderr, "Hz");      break;
	    case GII_PT_LENGTH:          fprintf(stderr, "m");       break;
	    case GII_PT_VELOCITY:        fprintf(stderr, "m/s");     break;
	    case GII_PT_ACCELERATION:    fprintf(stderr, "m/s^2");   break;
	    case GII_PT_ANGLE:           fprintf(stderr, "rad");     break;
	    case GII_PT_ANGVELOCITY:     fprintf(stderr, "rad/s");   break;
	    case GII_PT_ANGACCELERATION: fprintf(stderr, "rad/s^2"); break;
	    case GII_PT_AREA:            fprintf(stderr, "m^2");     break;
	    case GII_PT_VOLUME:          fprintf(stderr, "m^3");     break;
	    case GII_PT_MASS:            fprintf(stderr, "kg");      break;
	    case GII_PT_FORCE:           fprintf(stderr, "N ");      break;
	    case GII_PT_PRESSURE:        fprintf(stderr, "Pa");      break;
	    case GII_PT_TORQUE:          fprintf(stderr, "Nm");      break;
	    case GII_PT_ENERGY:          fprintf(stderr, "J");       break;
	    case GII_PT_POWER:           fprintf(stderr, "W");       break;
	    case GII_PT_TEMPERATURE:     fprintf(stderr, "K");       break;
	    case GII_PT_CURRENT:         fprintf(stderr, "A");       break;
	    case GII_PT_VOLTAGE:         fprintf(stderr, "V");       break;
	    case GII_PT_RESISTANCE:      fprintf(stderr, "ohm)");    break;
	    case GII_PT_CAPACITY:        fprintf(stderr, "farad");   break;
	    case GII_PT_INDUCTIVITY:     fprintf(stderr, "henry");   break;
	    default:                     fprintf(stderr, "???");     break;
	  }
	
	  fprintf(stderr, "\n");
	}

	/* update display */

	if (cur_dev && VI->number < MAX_NR_VAL) {

		if (cur_dev->VI[VI->number]) {
			free(cur_dev->VI[VI->number]);
		}

		cur_dev->VI[VI->number] =
			malloc(sizeof(gii_cmddata_getvalinfo));

		*cur_dev->VI[VI->number] = *VI;

		cur_dev->axes[VI->number] = VI->range.center;

		draw_inp_device(cur_dev);
	}
}

static void show_command(gii_cmd_event *ev)
{
	int s = (do_show != SHOW_NIL);

	switch (ev->code) {
		case GII_CMDCODE_GETDEVINFO:
			if (s) fprintf(stderr, "GetDevInfo: ");
			show_devinfo(ev);
			return;

		case GII_CMDCODE_GETVALINFO:
			if (s) fprintf(stderr, "GetValInfo: ");
			show_valinfo(ev);
			return;
	}

	if (s) fprintf(stderr, "code=%0x08x\n", ev->code);
}

static void show_expose(gii_expose_event *ev)
{
	int i, s = (do_show != SHOW_NIL);

	if (s) fprintf(stderr, "Expose: from=(%d,%d) size=(%d,%d)\n", 
			ev->x, ev->y, ev->w, ev->h);

	/* update display */

	ggiSetGCForeground(vis, ggiMapColor(vis, &black));
	ggiFillscreen(vis);
	
	for (i=0; i < MAX_NR_DEV; i++) {
		if (InputDevices[i]) {
			draw_inp_device(InputDevices[i]);
		}
	}
}

static void show_event(gii_event *ev)
{
	int s = (do_show != SHOW_NIL);

	switch(ev->any.type)
	{
		case evKeyPress:
			if (s) fprintf(stderr, "KeyPress:    ");
			show_key(&ev->key);
			break;

		case evKeyRepeat:
			if (s) fprintf(stderr, "KeyRepeat:   ");
			show_key(&ev->key);
			break;

		case evKeyRelease:
			if (s) fprintf(stderr, "KeyRelease:  ");
			show_key(&ev->key);
			break;

		case evValAbsolute:
			if (s) fprintf(stderr, "ValAbsolute: ");
			show_valuator(&ev->val);
			break;

		case evValRelative:
			if (s) fprintf(stderr, "ValRelative: ");
			show_valuator(&ev->val);
			break;

		case evPtrAbsolute:
			if (!no_pmove && s) fprintf(stderr, "PtrAbsolute: ");
			show_pmove(&ev->pmove);
			break;

		case evPtrRelative:
			if (!no_pmove && s) fprintf(stderr, "PtrRelative: ");
			show_pmove(&ev->pmove);
			break;

		case evPtrButtonPress:
			if (s) fprintf(stderr, "PtrButtonPress:   ");
			show_pbutton(&ev->pbutton);
			break;

		case evPtrButtonRelease:
			if (s) fprintf(stderr, "PtrButtonRelease: ");
			show_pbutton(&ev->pbutton);
			break;

		case evExpose:
			show_expose(&ev->expose);
			break;

		case evCommand:
			if (s) fprintf(stderr, "Command: ");
			show_command(&ev->cmd);
			break;

		case evInformation:
			if (s) fprintf(stderr, "Information: ");
			show_command(&ev->cmd);
			break;

		default:
			if (s) fprintf(stderr, "Unknown: type=0x%02x\n",
				ev->any.type);
	}

	if (do_show == SHOW_LONG) {
		show_common(ev);
	}
	fflush(stderr);
}


static void usage(void)
{
	fprintf(stderr, 
		"\n\nUSAGE:  inputdump  [OPTION]...\n\n"
		"where OPTION is one of:\n"
		"    -m --mode      <mode spec>\n"
		"    -t --target    <target spec>\n"
		"    -i --input     <input spec>\n"
		"    -s --short\n"
		"    -l --long\n"
		"    -p --no-pmove\n"
		"    -h --help\n\n");
}

int main(int argc, char **argv)
{ 
	char *prog;
	char *target_str = NULL;
	char *input_str  = NULL;
	int   quit = 0;
	gii_event ev;
	int ch_w, ch_h;

	ggiParseMode("", &vis_mode);

	/* handle args */
	prog = *argv++; argc--;

#define CMP_OPT(short, long, extra)  \
	((argc >= (extra+1)) && \
	 ((strcmp(*argv, short) == 0) ||  \
	  (strcmp(*argv, long) == 0)))

	while (argc > 0) {

		if (CMP_OPT("-h", "--help", 0)) {
			usage();
			exit(1);
		}

		if (CMP_OPT("-m", "--mode", 1)) {
			ggiParseMode(argv[1], &vis_mode);
			argv += 2; argc -= 2;
			continue;
		}

		if (CMP_OPT("-t", "--target", 1)) {
			target_str = argv[1];
			argv += 2; argc -= 2;
			continue;
		}
			
		if (CMP_OPT("-i", "--input", 1)) {
			input_str = argv[1];
			argv += 2; argc -= 2;
			continue;
		}
			
		if (CMP_OPT("-s", "--short", 0)) {
			do_show = SHOW_SHORT; 
			argv++; argc--;
			continue;
		}
			
		if (CMP_OPT("-l", "--long", 0)) {
			do_show = SHOW_LONG; 
			argv++; argc--;
			continue;
		}
			
		if (CMP_OPT("-p", "--no-pmove", 0)) {
			no_pmove = 1;
			argv++; argc--;
			continue;
		}

		fprintf(stderr, "Unknown option '%s'.\n", *argv);
		usage();
		exit(1);
	}

#undef CMP_OPT


	/* setup graphics mode */

	if (ggiInit() != 0) {
		fprintf(stderr, "Unable to initialize LibGGI, exiting.\n");
		exit(1);
	}

	vis = ggiOpen(target_str, NULL);

	if (vis == NULL) {
		fprintf(stderr, "Failed to open visual.\n");
		ggiExit();
		exit(1);
	}

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	ggiCheckMode(vis, &vis_mode);

	if (ggiSetMode(vis, &vis_mode) < 0) {
		fprintf(stderr, "Could not set mode.\n");
		ggiClose(vis);
		ggiExit();
		exit(1);
	}

	if (GT_SCHEME(vis_mode.graphtype) == GT_PALETTE) {
		ggiSetColorfulPalette(vis);
	}
	
	ggiGetCharSize(vis, &ch_w, &ch_h);
	vis_ch.x = ch_w; vis_ch.y = ch_h;

	if (do_show != SHOW_NIL) {
		fprintf(stderr, "Mode is: ");
		ggiFPrintMode(stderr, &vis_mode);
		fprintf(stderr, "\n");
	}

	/* add extra input */

	if (input_str) {
		ggiJoinInputs(vis, giiOpen(input_str, NULL));
	}

	/* main loop */

	do {
		int n;

		ggiEventPoll(vis, emAll, NULL);
		
		n = ggiEventsQueued(vis, emAll);

		while (n) {
			ggiEventRead(vis, &ev, emAll);
			
			if ((ev.any.type == evKeyPress) && 
			    (ev.key.sym == GIIUC_Escape)) {
				quit = 1;
				break;
			} else {
				cur_dev = find_input_device(ev.any.origin);
				show_event(&ev);
			}
			n--;
		}
		ggiFlush(vis);
	} while (! quit);

	ggiClose(vis);
	ggiExit();

	exit(0);
}
