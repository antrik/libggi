/* $Id: visual.c,v 1.10 2005/07/30 11:38:51 cegger Exp $
******************************************************************************

   VT switch handling for Linux console

   Copyright (C) 1999 Marcus Sundber	[marcus@ggi-project.org]

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

/* We want SA_NOMASK from glibc 2.1 */
#define _GNU_SOURCE

#include "config.h"
#include <ggi/display/linvtsw.h>
#include <ggi/internal/ggi_debug.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#ifdef HAVE_SYS_VT_H
#include <sys/vt.h>
#else
#include <linux/vt.h>
#endif
#ifdef HAVE_SYS_KD_H
#include <sys/kd.h>
#else
#include <linux/kd.h>
#endif
#ifdef HAVE_LINUX_TASKS_H
#include <linux/tasks.h>
#endif

#ifndef PID_MAX
#define PID_MAX 0x8000
#else
#if PID_MAX > 0x10000
/* Make sure we don't get stuck in a huge loop... */
#define PID_MAX 0x10000
#endif
#endif


/* We can only run on one VT at a time. */
static ggi_linvtsw_arg vthandling;
static int 	vtfd = -1;
static int	origvtnum;
static int	vt_switched_away;
static int	switchpending = 0;

struct vislist {
	ggi_visual	*vis;
	ggi_linvtsw_arg  args;
	struct vislist	*next;
};

static int refcount = 0;
static void *vt_lock;
static struct vislist *vtvisuals = NULL;


static int
vt_add_vis(ggi_visual *vis, ggi_linvtsw_arg *args)
{
	struct vislist *curr = vtvisuals, *newent;

	newent = malloc(sizeof(struct vislist));
	if (newent == NULL) return GGI_ENOMEM;

	newent->vis = vis;
	newent->args = *args;
	newent->next = NULL;

	if (curr == NULL) {
		vtvisuals = newent;
	} else {
		while (curr->next != NULL) curr = curr->next;
		curr->next = newent;
	}

	return 0;
}


static int
vt_del_vis(ggi_visual *vis)
{
	struct vislist *curr, *prev = NULL;

	curr = vtvisuals;
	while (curr->vis != vis) {
		prev = curr;
		curr = curr->next;
		if (curr == NULL) return GGI_ENOTFOUND;
	}
	if (prev == NULL) {
		vtvisuals = curr->next;
	} else {
		prev->next = curr->next;
	}
	free(curr);

	return 0;
}


/*
******************************************************************************
 VT switch handling
******************************************************************************
*/

#ifdef SIGUNUSED
#define SWITCHSIG	SIGUNUSED
#else
/* Just a random signal which is rarely used */
#define SWITCHSIG	SIGXCPU
#endif


static ggi_visual *vtvisual;


static inline void
setsig(int sig, void (*func)(int))
{
	struct sigaction sa;

	sa.sa_handler = func;
	sa.sa_flags = 0;
#ifdef SA_RESTART
	/* Restart syscalls if possible */
	sa.sa_flags |= SA_RESTART;
#endif
#ifdef SA_NOMASK
	/* Don't block the signal in our handler */
	sa.sa_flags |= SA_NOMASK;
#endif
	sigemptyset(&sa.sa_mask);
	sigaction(sig, &sa, NULL);
	
	/* Make sure things like blocking select() are interrupted by the
	   switch signal */
	siginterrupt(sig, 1);
}


/* Release the VT */
static void
release_vt(void *arg)
{
	struct vislist *curr = vtvisuals;

	if (vt_switched_away) return;

	DPRINT_MISC("acknowledging VT-switch\n");
	while (curr) {
		if (curr->args.switching) {
			curr->args.switching(curr->args.funcarg);
		}
		curr = curr->next;
	}
	ioctl(vtfd, VT_RELDISP, 1);
	switchpending = 0;
	vt_switched_away = 1;
}


static void
switching_handler(int signo)
{
	struct vislist *curr = vtvisuals;
	sigset_t newset, oldset;
	
	/* Block all signals while executing handler */
	sigfillset(&newset);
	sigprocmask(SIG_BLOCK, &newset, &oldset);
	
	if (vt_switched_away) {
		DPRINT_MISC("acquire_vt START\n");

		/* Acknowledge the VT */
		ioctl(vtfd, VT_RELDISP, VT_ACKACQ);

		while (curr) {
			if (curr->args.switchback) {
				curr->args.switchback(curr->args.funcarg);
			}
			curr = curr->next;
		}

		vt_switched_away = 0;

		DPRINT_MISC("acquire_vt DONE\n");
	} else {
		DPRINT_MISC("release_vt START\n");

		if (switchpending) {
			DPRINT_MISC("release already pending\n");
			goto handler_out;
		}

		switchpending = 1;

		if (*vthandling.autoswitch) {
			release_vt(vtvisual);
			if (*vthandling.dohalt) {
				/* Suspend program until switch back */
				sigset_t tmpset = oldset;

				DPRINT_MISC("release_vt SUSPEND\n");

				sigdelset(&tmpset, SWITCHSIG);
				sigprocmask(SIG_SETMASK, &tmpset, NULL);

				while (vt_switched_away) {
					/* Note: we rely on the acquire signal
					   interrupting us  */
					pause();
				DPRINT_MISC("release_vt INTERRUPTED\n");
				}
			}
		} else {
			while (curr) {
				if (curr->args.switchreq) {
				      curr->args.switchreq(curr->args.funcarg);
				}
				curr = curr->next;
			}
		}

		DPRINT_MISC("release_vt DONE\n");
	}
  handler_out:
	sigprocmask(SIG_SETMASK, &oldset, NULL);
}


static int
restore_vt(void)
{
	if (vthandling.vtnum != -1 && vthandling.vtnum != origvtnum) {
		ioctl(vtfd, VT_ACTIVATE, origvtnum);
	}

	return 0;
}

static int
get_newcons(int fd)
{
	int vtnum;

	if (ioctl(fd, VT_OPENQRY, &vtnum) < 0 ||
	    vtnum <= 0) {
		fprintf(stderr,
"L/vtswitch: Unable to get a new virtual console\n");
		return GGI_ENODEVICE;
	}
	
	return vtnum;
}


static char nopermstring[] = 
"L/vtswitch: You are not running on a virtual console and do not have\n\tpermission to open a new one\n";

static int
vtswitch_open(ggi_visual *vis)
{
	char filename[80];
	int fd, dodetach = 0;
	struct vt_stat vt_state;
	struct vt_mode qry_mode;

	/* Query free VT and current VT */
	fd = open("/dev/tty", O_WRONLY);
	if (fd < 0) {
		perror("L/vtswitch: open /dev/tty");
		return GGI_ENODEVICE;
	}

	if (vthandling.novt) {
		vthandling.vtnum = -1;
		dodetach = 1;
	} else if (vthandling.forcenew) {
		if ((vthandling.vtnum = get_newcons(fd)) < 0) {
			close(fd);
			return vthandling.vtnum;
		}
		dodetach = 1;
	} else if (ioctl(fd, VT_GETSTATE, &vt_state) != 0) {
		DPRINT_MISC("L/vtswitch: VT_GETSTATE failed\n");
		close(fd);
		fd = open("/dev/console", O_WRONLY);
		if (fd < 0) {
			fprintf(stderr, nopermstring);
			return GGI_ENODEVICE;
		}
		if ((vthandling.vtnum = get_newcons(fd)) < 0) {
			close(fd);
			return GGI_ENODEVICE;
		}
		dodetach = 1;
	} else {
		vthandling.vtnum = vt_state.v_active;
	}
	if (dodetach) {
		/* Detach from our old controlling tty. */
		int res = ioctl(fd, TIOCNOTTY);	
		DPRINT_MISC("L/vtswitch: TIOCNOTTY = %d\n", res);
	}
	close(fd);

	if (dodetach && geteuid() && setsid() < 0) {
		int i;
		int pid = getpid();
		int ppgid = getpgid(getppid());

		/* We're not running as root and setsid() failed, so we try
		   to set our process group ID that of our parent. */
		setpgid(pid, ppgid);
		if (setsid() < 0) {
			/* We propably have a child process... */
			for (i = 1; i < 5; i++) {
				if (getpgid(pid + i) == pid) {
					setpgid(pid + i, ppgid);
				}
			}
			if (setsid() < 0) {
				/* That failed too. Now we scan the entire
				   process space after possible childs.
				   (This takes 32ms on a P225, so we can live
				   with doing this once at startup time...) */
				for (i = 2; i < PID_MAX; i++) {
					if (getpgid(i) == pid) {
						setpgid(i, ppgid);
					}
				}
				/* Last chance, if this fails we probably
				   can't switch VTs below. */
				setsid();
			}
		}
	}
	if (vthandling.novt) {
		/* Don't open any VT */
		return 0;
	}

	/* Open VT */
	sprintf(filename, "/dev/tty%d", vthandling.vtnum);

	vtfd = open(filename, O_WRONLY);
	if (vtfd < 0) {
		/* Try devfs style device */
		sprintf(filename, "/dev/vc/%d", vthandling.vtnum);
		vtfd = open(filename, O_WRONLY);
		if (vtfd < 0) {
			fprintf(stderr, "L/vtswitch: open %s: %s\n",
				filename, strerror(errno));
			return GGI_ENODEVICE;
		}
	}

        if (ioctl(vtfd, VT_GETSTATE, &vt_state) < 0) {
		fprintf(stderr, "L/vtswitch: unable to get current console\n");
		close(vtfd);
		vtfd = -1;
		return GGI_ENODEVICE;
	}
	origvtnum = vt_state.v_active;
        if (vthandling.vtnum != vt_state.v_active) {
		if (ioctl(vtfd, VT_ACTIVATE, vthandling.vtnum)) {
			fprintf(stderr, nopermstring);
			close(vtfd);
			vtfd = -1;
			return GGI_ENODEVICE;
		}
		while (ioctl(vtfd, VT_WAITACTIVE,
			     vthandling.vtnum) < 0) {
			ggUSleep(150000);
		}
	}

	DPRINT_MISC("L/vtswitch: Using VT %d.\n", vthandling.vtnum);
	
	if (vthandling.onconsole) {
		/* Disable normal text on the console */
		ioctl(vtfd, KDSETMODE, KD_GRAPHICS);
	}
	
	/* Unfortunately, we need to take control over VT switching like
	 * Xfree and SVGAlib does.  Sigh.
	 */
	ioctl(vtfd, VT_GETMODE, &qry_mode);

	qry_mode.mode = VT_PROCESS;
	qry_mode.relsig = SWITCHSIG;
	qry_mode.acqsig = SWITCHSIG;
	
	vt_switched_away = 0;
	vtvisual = vis;

	setsig(SWITCHSIG, switching_handler);

	if (ioctl(vtfd, VT_SETMODE, &qry_mode) < 0) {
		perror("L/vtswitch: Setting VT mode failed");
		restore_vt();
		close(vtfd);
		vtfd = -1;
		return GGI_ENODEVICE;
	}
	
	DPRINT_MISC("L/vtswitch: open OK.\n");

	return 0;
}


static void
vtswitch_close(ggi_visual *vis)
{
	struct vt_mode qry_mode;

	if (vtfd >= 0) {
		if (vthandling.onconsole) {
			ioctl(vtfd, KDSETMODE, KD_TEXT);
		}
		if (ioctl(vtfd, VT_GETMODE, &qry_mode) == 0) {
			qry_mode.mode = VT_AUTO;
			ioctl(vtfd, VT_SETMODE, &qry_mode);
		}
		signal(SWITCHSIG, SIG_DFL);
		restore_vt();

		close(vtfd);
		vtfd = -1;
	}

	DPRINT_MISC("L/vtswitch: close OK.\n");
}

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
			const char *args, void *argptr, uint32_t *dlret)
{
	ggi_linvtsw_arg *myargs = (ggi_linvtsw_arg *) argptr;
	int err;

	if (myargs == NULL) {
		ggiPanic("Target tried to use linvtsw helper in a wrong way!\n");
	}

	DPRINT_MISC("L/vtswitch: GGIdlinit(%p,...) called\n", vis);

	myargs->vtnum = -1;

	ggLock(_ggi_global_lock);
	if (vt_lock == NULL) {
		vt_lock = ggLockCreate();
		if (vt_lock == NULL) {
			ggUnlock(_ggi_global_lock);
			return GGI_ENOMEM;
		}
	}
	ggUnlock(_ggi_global_lock);

	DPRINT_MISC("L/vtswitch: have private mutex\n");

	ggLock(vt_lock);
	err = vt_add_vis(vis, myargs);
	if (err) {
		ggUnlock(vt_lock);
		return err;
	}
	refcount++;
	if (refcount > 1) {
		myargs->vtnum = vthandling.vtnum;
		myargs->doswitch = release_vt;
		myargs->refcount = refcount;
		ggUnlock(vt_lock);

		return 0;
	}
	
	vthandling = *myargs;

	err = vtswitch_open(vis);
	if (err) {
		refcount--;
		vt_del_vis(vis);
		ggUnlock(vt_lock);
		return err;
	}
	myargs->vtnum = vthandling.vtnum;
	myargs->doswitch = release_vt;
	myargs->refcount = refcount;
	ggUnlock(vt_lock);

	return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
	/* Make sure we're only called once */
	if (refcount == 0) return 0;
	
	ggLock(vt_lock);
	refcount--;
	vt_del_vis(vis);
	if (refcount == 0) {
		vtswitch_close(vis);
	}
	ggUnlock(vt_lock);

	return 0;
}
		

EXPORTFUNC
int GGIdl_linvtsw(int func, void **funcptr);

int GGIdl_linvtsw(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
