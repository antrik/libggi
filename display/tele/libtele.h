/* $Id: libtele.h,v 1.5 2005/07/31 15:30:37 soyt Exp $
******************************************************************************

   libtele.h

   Copyright (C) 1998 Andrew Apted	[andrew@ggi-project.org]
                 2002 Tobias Hunger     [tobias@fresco.org]

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

#ifndef _LIBTELE_H
#define _LIBTELE_H

#include <ggi/ggi.h>


/*** TYPES ***/

typedef int32_t  T_Long;


/*** COMMON INFO ***/

#define TELE_PORT_BASE		27780
#define TELE_FIFO_BASE		"/tmp/.tele"

#define TELE_BIG_ENDIAN		'B'	/* on-the-wire values */
#define TELE_LITTLE_ENDIAN	'L'
#define TELE_NORMAL_ENDIAN	'N'	/* only in received events */
#define TELE_REVERSE_ENDIAN	'R'

#define TELE_OK			 0
#define TELE_ERROR		-1	/* catch-all */

#define TELE_ERROR_SHUTDOWN	-400	/* other end has vanished */
#define TELE_ERROR_BADEVENT	-401	/* error reading event */
#define TELE_ERROR_INVALID	-402	/* invalid request */
#define TELE_ERROR_OUTOFMEM	-403	/* out of memory */
#define TELE_ERROR_BUSY		-404	/* server is already in use */

#define TELE_MINIMAL_EVENT	6	/* number of T_Longs */

#define TELE_MAXIMUM_RAW(cmdtype)  \
	(sizeof(TeleEvent) - TELE_MINIMAL_EVENT*4 - sizeof(cmdtype) - 4)

#define TELE_MAXIMUM_TLONG(cmdtype)  (TELE_MAXIMUM_RAW(cmdtype) / 4)


typedef struct tele_event
{
	unsigned char size;	/* total number of T_Longs */
	unsigned char endian;	/* endianness */
	unsigned char rawstart;	/* number of T_Longs before raw data */
	unsigned char dummy;

	T_Long type;		/* type of event */
	T_Long device;		/* device for input event */
	T_Long sequence;	/* used for replies */

	struct {
		T_Long sec;
		T_Long nsec;
	} time;			/* timestamp of event */

	T_Long data[255 - TELE_MINIMAL_EVENT];	/* event specific stuff */
} TeleEvent;


typedef enum tele_event_type
{
	TELE_INP_BASE = 0x4900,		/* 'I' << 8 */

	TELE_INP_KEY,		/* key pressed */
	TELE_INP_KEYUP,		/* key released */
	TELE_INP_BUTTON,	/* button pressed (mouse, joy, tablet) */
	TELE_INP_BUTTONUP,	/* button released */

	TELE_INP_MOUSE,		/* relative mouse */
	TELE_INP_JOYSTICK,	/* absolute joystick */
	TELE_INP_TABLET,	/* absolute tablet (or pointer) */

	TELE_INP_EXPOSE,	/* screen needs refreshing */

	TELE_CMD_BASE = 0x4300,		/* 'C' << 8 */

	TELE_CMD_CHECK,		/* check mode */
	TELE_CMD_OPEN,		/* open window / set mode */
	TELE_CMD_GETPIXELFMT,   /* request mode info from the client */
	TELE_CMD_CLOSE,		/* close window / reset mode */
	TELE_CMD_FLUSH,		/* flush data */

	TELE_CMD_PUTBOX,	/* write a box of pixels */
	TELE_CMD_GETBOX,	/* read a box of pixels */
	TELE_CMD_DRAWBOX,	/* draw a solid box */
	TELE_CMD_COPYBOX,	/* copy a box elsewhere */
	TELE_CMD_PUTSTR,	/* write a string */
	TELE_CMD_GETCHARSIZE,	/* get char size */
	TELE_CMD_DRAWLINE,	/* draw a solid line */

	TELE_CMD_SETORIGIN,	/* pan around */
	TELE_CMD_SETFRAME,	/* change frames */
	TELE_CMD_SETPALETTE,	/* change the palette */

	TELE_EVENT_TYPE_MASK = 0xff00

} TeleEventType;

typedef enum tele_device_type
{
	TELE_DEVICE_KEYBOARD	= 0x4b00,	/* 'K' << 8 */
	TELE_DEVICE_MOUSE	= 0x5d00,	/* 'M' << 8 */
	TELE_DEVICE_JOYSTICK	= 0x5a00,	/* 'J' << 8 */
	TELE_DEVICE_TABLET	= 0x5400,	/* 'T' << 8 */

	TELE_DEVICE_TYPE_MASK	= 0xff00,
	TELE_DEVICE_UNIT_MASK	= 0x00ff

} TeleDeviceType;


/*** Input event structures ***/

typedef struct tele_inp_key_data
{
	T_Long key;		/* key with modifiers */
	T_Long label;		/* key without modifiers */
	T_Long button;		/* raw scancode */
	T_Long modifiers;	/* modifiers in effect */

} TeleInpKeyData;

typedef struct tele_inp_button_data	/* for mouse, joystick & tablet */
{
	T_Long button;		/* button number */

} TeleInpButtonData;

typedef struct tele_inp_axis_data	/* for mouse, joystick & tablet */
{
	T_Long count;		/* number of axes */
	T_Long axes[1]; 	/* value of each axis */

} TeleInpAxisData;


/*** Command event structures ***/

typedef struct tele_cmd_open_data
{
	T_Long error;		/* replied: error value */

	/* everything below is sent _and_ replied */

	T_Long graphtype;
	T_Long frames;

	struct {
		T_Long width;
		T_Long height;
	} visible;

	struct {
		T_Long width;
		T_Long height;
	} virt;

	struct {
		T_Long width;
		T_Long height;
	} dot;

	struct {
		T_Long width;
		T_Long height;
	} size;

} TeleCmdOpenData;

typedef struct tele_cmd_pixelfmt_data
{
  T_Long depth;
  T_Long size;

  T_Long red_mask;
  T_Long green_mask;
  T_Long blue_mask;
  T_Long alpha_mask;
  T_Long clut_mask;
  T_Long fg_mask;
  T_Long bg_mask;
  T_Long texture_mask;

  T_Long flags;
  T_Long stdformat;
} TeleCmdPixelFmtData;

typedef struct tele_cmd_getput_data
{
	T_Long x;
	T_Long y;
	T_Long width;
	T_Long height;
	T_Long bpp;		/* used by getbox */

	T_Long pixel[1];	/* raw: (replied): array of width*height*bpp */

} TeleCmdGetPutData;

typedef struct tele_cmd_drawbox_data
{
	T_Long x;
	T_Long y;
	T_Long width;
	T_Long height;

	T_Long pixel;

} TeleCmdDrawBoxData;

typedef struct tele_cmd_copybox_data
{
	T_Long sx;
	T_Long sy;
	T_Long dx;
	T_Long dy;
	T_Long width;
	T_Long height;

} TeleCmdCopyBoxData;

typedef struct tele_cmd_putstr_data
{
	T_Long x;
	T_Long y;
        T_Long length;
        T_Long fg;
        T_Long bg;
	T_Long text[1];    /* raw: ASCII string, zero terminated */

} TeleCmdPutStrData;

typedef struct tele_cmd_getcharsize_data
{
	T_Long width;
	T_Long height;

} TeleCmdGetCharSizeData;

typedef struct tele_cmd_drawline_data
{
	T_Long x;
	T_Long y;
	T_Long xe;
	T_Long ye;

	T_Long pixel;

} TeleCmdDrawLineData;

typedef struct tele_cmd_setorigin_data
{
	T_Long x;
	T_Long y;

} TeleCmdSetOriginData;

typedef struct tele_cmd_setframe_data
{
	T_Long read;
	T_Long write;
	T_Long display;

} TeleCmdSetFrameData;

typedef struct tele_cmd_setpalette_data
{
	T_Long start;
	T_Long len;

	T_Long colors[1];    /* RGB 8:8:8 */

} TeleCmdSetPaletteData;


/*** SERVER INFO ***/

typedef struct tele_server
{
	int conn_fd;
	int inet;
	int display;
	int endianness;
} TeleServer;

typedef struct tele_user
{
	int sock_fd;
	TeleServer *server;
	T_Long seq_ctr;

} TeleUser;

extern int tserver_init(TeleServer *s, int display);
extern int tserver_exit(TeleServer *s);
extern int tserver_check(TeleServer *s);
extern int tserver_open(TeleServer *s, TeleUser *u);

extern int tserver_poll(TeleUser *u);
extern int tserver_read(TeleUser *u, TeleEvent *event);
extern int tserver_write(TeleUser *u, TeleEvent *event);
extern int tserver_close(TeleUser *u);

extern void *tserver_new_event(TeleUser *u, TeleEvent *event,
			       TeleEventType type,
			       int data_size, int raw_size);

/*** CLIENT INFO ***/

typedef struct tele_client
{
	int sock_fd;
	int inet;
	int display;
	int endianness;
	T_Long seq_ctr;

} TeleClient;

extern int tclient_open(TeleClient *c, const char *addrspec);
extern int tclient_close(TeleClient *c);

extern int tclient_poll(TeleClient *c);
extern int tclient_read(TeleClient *c, TeleEvent *event);
extern int tclient_write(TeleClient *c, TeleEvent *event);

extern void *tclient_new_event(TeleClient *c, TeleEvent *event,
			       TeleEventType type,
			       int data_size, int raw_size);

#endif /* _LIBTELE_H */
