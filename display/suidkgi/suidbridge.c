/* $Id: suidbridge.c,v 1.3 2005/01/14 09:38:14 pekberg Exp $
******************************************************************************

   Display-SUID: suid suidbrige

   Copyright (C) 1998 Andreas Beck    [becka@ggi-project.org]

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
#include <unistd.h>
#include <stdlib.h>
#include <sys/fcntl.h>

#include <kgi/module.h>
#include <ggi/internal/gg_replace.h>

#define	KGI_8x8_FONT
#define	KGI_8x14_FONT
#define	KGI_9x16_FONT

#define MAX_NR_DEFAULT_FONTS 10

#include "font.inc"
#include "clut.inc"

kgi_fb_ops text16_color_io, text16_color_mem;

static struct kgi_graphic mygraphic;
static struct kgi_display *mydisplay;
#define mydevice mygraphic.dev

#if 0
void * kgi_malloc_small (size_t size, int priority)
{
        return( malloc ( size ) );
}
void kgi_free_small ( const void *obj )
{
        free ( obj );
        return;
}
#endif

#define printk printf

static struct kgi_font *font[MAX_NR_DEFAULT_FONTS] =
{	/* !!! FIXME: for now there must be only one font per height*/
	#ifdef KGI_9x16_FONT

		&font_9x16,
	#endif

	#ifdef KGI_8x14_FONT

		&font_8x14,
	#endif

	#ifdef KGI_8x8_FONT

		&font_8x8,
	#endif
};

/*	Set the visible area of the current frame to originate from the pixel
**	(x,y). <dev> needs to be attached to a display. If the display supports
**	set_origin then dev->org[frame] is updated. The hardware is setup too.
*/
void kgi_set_origin(struct kgi_device *dev, kgi_uint frame, kgi_uint x, kgi_uint y)
{
	register struct kgi_display *dpy = dev->dpy;

	ASSERT(dpy);
	ASSERT(frame < NR_FB);

	DEBUG2("setorigin %d,%d\n",x,y);
	if (dpy->set_origin) {

		DEBUG2("doing setorigin %d,%d\n",x,y);
		dev->org[frame].x = x;
		dev->org[frame].y = y;
		(dpy->set_origin)(dpy, x, y);
	}
}

/*	Set the split line for frame 0. dev needs to be attached to a display.
**	If the display supports the split line feature, dev->split is updated.
**	The hardware is setup too.
*/
void kgi_set_split(struct kgi_device *dev, kgi_uint line)
{
	register struct kgi_display *dpy = dev->dpy;

	if (dpy->set_split) {

		dev->split = line;
		(dpy->set_split)(dpy, line);
	}
}

struct kgi_font *kgi_default_font(kgi_modereq *mode)
{
	int i = 0;

	while (font[i] && (font[i]->dy > mode->dpp.y)) {

		i++;
	}

	DEBUG2("returning font %p\n",font[i]);
	return font[i];
}

kgi_clut *kgi_default_clut(kgi_modereq *mode)
{
	switch(mode->graphtype) {

		case KGIGT_TEXT16:	return &default_clut_16;

		case KGIGT_15BIT:
		case KGIGT_16BIT:
		case KGIGT_24BIT:
		case KGIGT_32BIT:	return &default_gamma;

		default:	return &default_clut_256;
	}
}

                    

int kgi_register_display(struct kgi_display *dpy, int id)
{
	mydisplay=dpy;
	id=id;	/* We don't care ... */

	DEBUG2("registering display as %p,%d\n",dpy,id);
	dpy->conflict_setup = 0xFFFFFFFF;
	dpy->conflict_text  = 0xFFFFFFFF;
	dpy->conflict_graph = 0xFFFFFFFF;
	dpy->id = id;
	dpy->refcnt = 0;
	dpy->dev = NULL;

	mydevice.dpy=dpy;

	return 0;
}

/*	Check if a mode is valid. This returns a valid <mode> and 0 if
**	the mode can be done. NOTE: <cmd> must be either CMD_PROPOSE or
**	CMD_CHECK. <dpy> and <mode> must be valid.
*/
inline int kgi_check_mode(struct kgi_display *dpy, struct kgi_mode *mode, 
			enum kgi_tm_cmd cmd)
{
	ASSERT(dpy);
	ASSERT(mode);
	ASSERT((cmd == CMD_PROPOSE) || (cmd == CMD_CHECK));

	DEBUG2("checkmode entered\n");
	if (mode->request.frames >= NR_FB) {

		return GGI_ENOMEM;
	}

	return (dpy->check_mode)(dpy, mode, cmd);
}

/* !!!	Check, adopt calling interface to accept a device instead of a display.
*/
inline void kgi_set_mode(struct kgi_display *dpy, struct kgi_mode *mode)
{
	ASSERT(dpy);
	ASSERT(mode);

	DEBUG2("setmode entered\n");
	if (memcmp(&(dpy->mode), mode, sizeof(struct kgi_mode))) {

		(dpy->set_mode)(dpy, mode);
	}
}

/*	Set the pointer window to the given minimal and maximal values.
*/
void kgi_set_pointer_window(struct kgi_device *dev, kgi_uint x1, kgi_uint x2,
	kgi_uint y1, kgi_uint y2)
{
	/* forget it. */
}

void get_primary_fb(long *size,long *psize,long *pstart)
{
	*size  =mygraphic.mmio[0]->size;
	*psize =mygraphic.mmio[0]->psize;
	*pstart=mygraphic.mmio[0]->pstart;
	return;
}


/****************** graphics.c reproduced here *********************/

/*
**	IOCTL services
*/

static int graph_driver_command(struct kgi_graphic *graph, kgi_uint cmd, kgi_uint arg)
{
	int err,i;
	struct kgi_mmio_region *mmio;
/*	struct kgi_accel *accel; */
	struct kgi_mode helpkgimode;

	switch(cmd) {

	case DRIVER_TESTDRIVER:		/* check if driver is present	*/
		return 0x12345678;/*FIXME: WAS : DEVICEFILE_MAGIC;*/

	case DRIVER_TESTMODE:		/* check if mode is possible	*/
		memcpy(&helpkgimode,graph->io_buf,sizeof(kgi_modereq));
		err=kgi_check_mode(mydevice.dpy,&helpkgimode, CMD_PROPOSE);
		memcpy(graph->io_buf,&helpkgimode,sizeof(kgi_modereq));
		return err;

	case DRIVER_SETGRAPHMODE:	/* set mode for device	*/
		memcpy(&helpkgimode,graph->io_buf,sizeof(kgi_modereq));
#if 0
		if (mmapcnt[mydevice.id - MAX_NR_CONSOLES]) {

			return GGI_EBUSY;
		}
#endif

		if ((err = kgi_check_mode(mydevice.dpy, &helpkgimode, CMD_PROPOSE))) {
			memcpy(graph->io_buf,&helpkgimode,sizeof(kgi_modereq));
			return err;
		}
		memcpy(&(mydevice.mode), &helpkgimode, sizeof(mydevice.mode));
		kgi_set_split(&mydevice, mydevice.mode.virt.y);
		memcpy(graph->io_buf,&(mydevice.mode),sizeof(kgi_modereq));

#define      MMIO            (graph->mmio)

		mmio = helpkgimode.mmio;
		for (i = 0; i < MAX_NR_MMIO_REGIONS; i++) {

			MMIO[i] = mmio;
			mmio = mmio ? mmio->next : NULL;
			ASSERT(! MMIO_MAP[i]);

			#ifdef DEBUG

				if (MMIO[i]) {

					TRACE(printk(
					"%s:%i: mmio%i: '%s', @%.8lx,"\
					" psize %lx, total %lx\n",
						__FILE__,__LINE__, i,
						MMIO[i]->name, 
						(unsigned long)MMIO[i]->pstart,
						(unsigned long)MMIO[i]->psize, 
						(unsigned long)MMIO[i]->size));
				}
			#endif
		}
#if 0
		accel = helpkgimode.accel;
		for (i = 0; i < MAX_NR_ACCEL_REGIONS; i++) {

			ACCEL[i] = accel;
			accel = accel ? accel->next : NULL;

			#ifdef DEBUG

				if (ACCEL[i]) {

					TRACE(printk(KERN_DEBUG \
						"%s:%i: accel%i : '%s'\n",
						__FILE__,__LINE__,
						i, ACCEL[i]->name));
				}
			#endif
		}

		for (i = 0; i < helpkgimode.request.frames; i++) {

			ggi_clut *clut = kgi_default_clut(&(helpkgimode.request));

			ASSERT(clut);

			DEV.org[i].x = DEV.org[i].y = 0;
			memcpy(&(DEV.lut[i]), clut, sizeof(ggi_clut));
		}

#endif
		kgi_set_pointer_window(&mydevice,  0, mydevice.mode.visible.x, 
			0, mydevice.mode.visible.y);

		kgi_set_mode(mydevice.dpy, &helpkgimode);
		return 0;

	case DRIVER_GETGRAPHMODE:	/* return graphic mode		*/
		memcpy(graph->io_buf,&(mydevice.mode),sizeof(mydevice.mode));
		return 0;

	case DRIVER_ACCEL:		/* go to graphic mode		*/
		/* Nonsense here */
		return 0;

	case DRIVER_TEXT:		/* go to text mode (console)	*/
		/* Nonsense here */
		return 0;

#if 0
	case DRIVER_GETINFO:
	case DRIVER_SETINFO:
	case DRIVER_STRATEGY:

	case DRIVER_RAMDAEMON_INSTALL:
	case DRIVER_RAMDAEMON_ASK:
	case DRIVER_RAMDAEMON_CPY:
#endif
	default:
		DEBUG2(KERN_INFO "%s:%i: Unknown command %x\n",\
			__FILE__,__LINE__, cmd);
		return -E(DRIVER, UNKNOWN);
	}
}

int IO_LOCK=0;

/*	ioctl() services. This is the primary command interface. Other 
**	driver-specific interfaces may be provided using mmio-regions or 
**	by giving direct access to registers where this is safe. 
**	Other OSes might even have other means.
*/
int graph_ioctl( unsigned int cmd, unsigned long arg)
{
	int foo;
	int result = 0;
	struct kgi_graphic *graph = &mygraphic;

	/*	Lock will be non-zero if there is another ioctl() call in
	**	progress but scheduled. With threads this can happen.
	*/
	ASSERT(! IO_LOCK);
	if (IO_LOCK++) {

		printk("%s:%i: ioctl() while other in progress. "
			"please report to " MAINTAINER ".\n",
			__FILE__,__LINE__);
		IO_LOCK--;
		return -EAGAIN; 
	}

#if 0
	IO_CNT = _IOC_SIZE(cmd);
	IO_RES = NULL;
#endif

	/*	Get the data from userspace.
	*/
	graph->io_buf=(void *)arg;

#if 0
	if (cmd & IOC_IN) {}
#endif

	/*	Dispatch the requested command.
	*/
	foo = _IOC_TYPE(cmd);
	if (foo == IOC_DRIVER) {

		DEBUG2(KERN_ERR "%s:%i: driver command %.8x\n",\
			__FILE__,__LINE__,cmd);
		result = graph_driver_command(graph, cmd, arg);

	} else {

		DEBUG2(KERN_ERR "%s:%i: dispatched command %.8x\n",\
			__FILE__,__LINE__,cmd);
		result = ((foo < KGI_NR_SUBSYSTEMS) && (mydevice.dpy->cmd[foo])) 
				? mydevice.dpy->cmd[foo](graph, cmd, arg)
				: -E(DRIVER, UNKNOWN);
	}

#if 0
	/*	write return values into userspace, but only if the call was
	**	successful.
	*/	
	if ((cmd & IOC_OUT) && (result == 0)) {

	}
#endif

	IO_LOCK--;
	DEBUG2("%s:%i: ioctl finished.\n",__FILE__,__LINE__);
	return result;
}


