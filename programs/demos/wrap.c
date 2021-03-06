/* $Id: wrap.c,v 1.28 2008/03/14 13:44:35 cegger Exp $
******************************************************************************

   wrap.c - run a libGGI application inside our own visual, essential for
            emulation

   Authors:	2001 Stefan Seefeld		[stefan@fresco.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

/* For autoconf inline support */
#include "config.h"

/* Include the LibGGI declarations.
 */
#include <ggi/gg.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>

/* Include the necessary headers used for e.g. error-reporting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#ifdef HAVE_UNISTD_H
# include <unistd.h>
#endif
#ifdef HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
#ifdef HAVE_SYS_IPC_H
# include <sys/ipc.h>
#endif
#ifdef HAVE_SYS_SHM_H
# include <sys/shm.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#if defined(__WIN32__) && !defined(__CYGWIN__)
# ifdef HAVE_WINSOCK2_H
#  include <winsock2.h>
# endif
# ifdef HAVE_WINSOCK_H
#  include <winsock.h>
# endif
#endif

#ifdef HAVE_SYS_UN_H
# include <sys/un.h>
#endif
#include <signal.h>
#include <errno.h>


/* GNU Hurd hasn't defined it.
 */
#ifndef PATH_MAX
# ifdef _POSIX_PATH_MAX
#  define PATH_MAX _POSIX_PATH_MAX
# else
#  define PATH_MAX 4096		/* Should be enough for most systems */
# endif
#endif


struct client_t {
	ggi_visual_t visual;
	char socket[PATH_MAX];
	int sockfd;
	int semid;
	int shmid;
	pid_t pid;
};

static void init_client(struct client_t * client, ggi_mode * mode,
			const char *command)
{
	struct sockaddr_un address;
#ifdef HAVE_SOCKLEN_T
	socklen_t len;
#else
	int len;
#endif
	size_t bufsize;
	char text[128];
	static char display[182 + 14];
	static char defmode[182 + 14];

	/* initialize the connection
	 */
	client->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (client->sockfd == -1) {
		fprintf(stderr, "error in socket %s\n", strerror(errno));
	}	/* if */

	address.sun_family = AF_UNIX;
	ggstrlcpy(address.sun_path, tmpnam(0), sizeof(address.sun_path));
	ggstrlcpy(client->socket, address.sun_path, sizeof(client->socket));

	if (bind(client->sockfd, (const struct sockaddr *) (&address),
		 sizeof(struct sockaddr_un))) {
		fprintf(stderr, "error in bind: %s\n", strerror(errno));
	}	/* if */

	/* serialization has yet to be implemented
	 */
	client->semid = 0;
	bufsize = mode->virt.x * mode->virt.y
	    * GT_ByPP(mode->graphtype) + 64 * 1024;

	client->shmid = shmget(IPC_PRIVATE, bufsize, IPC_CREAT | 0666);

	/* Open a shared "memory-visual" which is simply a simulated 
	 * display in shared memory.
	 */
	snprintf(text, sizeof(text),
		"display-memory:-input:shmid:%i", client->shmid);

	client->visual = ggNewStem(libgii, libggi, NULL);
	if (client->visual == NULL) {
		ggPanic("Ouch - can't initialize stem with libggi !\n");
	}
	if ((ggiOpen(client->visual, text, NULL)) < 0) {
		ggPanic("Ouch - can't open the shmem target !\n");
	}	/* if */

	mode->frames = 1;
	mode->size.x = GGI_AUTO;
	mode->size.y = GGI_AUTO;
	ggiSetMode(client->visual, mode);

	/* start the program
	 */
	snprintf(text, sizeof(text),
		"display-ipc:-input -socket=%s -semid=%i -shmid=%i",
		client->socket, client->semid, client->shmid);
	snprintf(display, sizeof(display), "GGI_DISPLAY=%s", text);
	putenv(display);

	ggiSNPrintMode(text, sizeof(text), mode);
	snprintf(defmode, sizeof(defmode), "GGI_DEFMODE=%s", text);
	putenv(defmode);

	client->pid = fork();
	if (client->pid == -1) {
		perror("fork");
		exit(-1);
	} else if (client->pid == 0) {
		execlp("/bin/sh", "/bin/sh", "-c", command, (void *) NULL);
		_exit(-1);
	}	/* if */
	
	listen(client->sockfd, 1);
	len = sizeof(struct sockaddr_un);
	client->sockfd = accept(client->sockfd,
				(struct sockaddr *) (&address), &len);
	if (client->sockfd == -1) {
		perror("accept");
	}	/* if */
}	/* init_client */


static void exit_client(struct client_t * client)
{
	ggiClose(client->visual);
	ggDelStem(client->visual);
	shmctl(client->shmid, IPC_RMID, NULL);
	unlink(client->socket);
}	/* exit_client */

static struct gg_instance *init_fdselect(struct gg_stem *stem, int fd)
{
	char arg[128];
	struct gg_instance *inp;

	snprintf(arg, sizeof(arg), "-read=%i", fd);
	inp = ggPlugModule(libgii, stem, "input-fdselect", arg,NULL);
	if (!inp) {
		ggPanic("Ouch - can't open the fdselect inputlib !");
	}	/* if */
	return inp;
}	/* init_fdselect */

static void exit_fdselect(struct gg_instance *inp)
{
	ggClosePlugin(inp);
}	/* exit_fdselect */

/* return 1 if we got called by the client
 * return 0 if nothing is to do (either because we got
 *		interrupted by a signal, or because we
 *		just forwarded an event)
 * return -1 on error
 */
static int wait_for_something(ggi_visual_t master, struct client_t * client)
{
	/* wait for the client to notify us
	 */
	gii_event event;
#if 0
	fd_set fds;
	gii_event_mask mask;
	int nfds;
#endif

	giiEventRead(master, &event, emAll);
	if (event.any.type == evInformation &&
		event.cmd.code == GII_CMDFLAG_PRIVATE)
	{
		return 1;
	}	/* if */
	giiEventSend(client->visual, &event);
	return 0;

	/* What about errors ??? */
#if 0
	FD_SET(client->sockfd, &fds);
	mask = emAll;
	nfds =
	    giiEventSelect(master, &mask, client->sockfd + 1, &fds, 0, 0,
			   0);
	if (nfds == -1) {
		if (errno == EINTR || errno == EAGAIN)
			errno = 0;
		return 0;
	} else if (nfds == 0) {
		giiEventRead(master, &event, mask);
		giiEventSend(client->visual, &event);
		return 0;
	} else if (FD_ISSET(client->sockfd, &fds)) {
		return 1;
	} else {
		return -1;
	}	/* if */
#endif
}	/* wait_for_something */


static int repair_screen(struct client_t * client, ggi_visual_t visual)
{
	char tag;
	int region[4];

	/* read what region was damaged and repair it
	 */
	if (read(client->sockfd, &tag, 1) == 1 &&	/* read 'F' (like 'flush'); */
	    read(client->sockfd, (char *) region,
		 4 * sizeof(int)) == 4 * sizeof(int)) {
		ggiCrossBlit(client->visual, region[0], region[1],
			     region[2], region[3], visual, region[0],
			     region[1]);
		ggiFlush(visual);
		return 1;
	}	/* if */
	return 0;
}	/* repair_screen */


int main(int argc, const char * const argv[])
{
	int rc;
	ggi_visual_t visual;
	ggi_mode mode = {	/* This will cause the default mode to be set */
		1,		/* 1 frame */
		{GGI_AUTO, GGI_AUTO},	/* Default size */
		{GGI_AUTO, GGI_AUTO},	/* Virtual */
		{0, 0},		/* size in mm don't care */
		GT_AUTO,	/* Mode */
		{GGI_AUTO, GGI_AUTO}	/* Font size */
	};

	const char *command;
	struct client_t client;
	struct gg_instance *fdselect;

	/* Get the arguments from the command line. 
	 * Set defaults for optional arguments.
	 */
	if (argc == 1 || ((strcmp(*argv, "-h") == 0) ||
			  (strcmp(*argv, "--help") == 0))) {
		fprintf(stderr, "Usage: %s <program>\n", argv[0]);
		exit(1);
		return 1;	/* Tell the user how to call and fail. */
	} else
		command = argv[1];

	/* Open up GGI and a visual.
	 */
	visual = ggNewStem(libgii, libggi, NULL);
	if (visual == NULL) {
		fprintf(stderr, "unable to initialize stem with libggi\n");
		exit (1);
	}	/* if */

	rc = ggiOpen(visual, NULL);
	if (rc < 0) {
		fprintf(stderr,
			"unable to open default visual, exiting.\n");
		ggDelStem(visual);
		exit(1);
	}	/* if */

	/* Go to async-mode.
	 */
	ggiSetFlags(visual, GGIFLAG_ASYNC);

	/* Check and set the mode ...
	 */
	ggiCheckMode(visual, &mode);
	if (ggiSetMode(visual, &mode)) {
		fprintf(stderr, "Can't set mode\n");
		ggiClose(visual);
		ggDelStem(visual);
		return 2;
	}	/* if */

	init_client(&client, &mode, command);
	fdselect = init_fdselect(visual, client.sockfd);

	/* run
	 */
	while (1) {
		int status = wait_for_something(visual, &client);
		if (!status)
			continue;
		if (status == -1 || !repair_screen(&client, visual))
			break;
	};

	kill(client.pid, SIGHUP);
	exit_fdselect(fdselect);
	exit_client(&client);
	ggiClose(visual);
	ggDelStem(visual);

	return 0;
}	/* main */
