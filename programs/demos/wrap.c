/* $Id: wrap.c,v 1.5 2002/08/02 22:37:41 cegger Exp $
******************************************************************************

   wrap.c - run a libGGI application inside our own visual, essential for
            emulation

   Authors:	2001 Stefan Seefeld		[stefan@berlin-consortium.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

/* Include the necessary headers used for e.g. error-reporting.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>

/* Include the LibGGI declarations.
 */
#include <ggi/gii.h>
#include <ggi/ggi.h>
#include <ggi/ggi-unix.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <signal.h>
#include <errno.h>

/* For autoconf inline support */
#include "config.h"


/* GNU Hurd hasn't defined it.
 */
#ifndef PATH_MAX
#define PATH_MAX	255
#endif


struct client_t
{
	ggi_visual_t visual;
	const char   socket[PATH_MAX];
	int          sockfd;
	int          semid;
	int          shmid;
	pid_t        pid;  
};

typedef struct client_t client_t;

void init_client(client_t *client, ggi_mode *mode, const char *command)
{
	struct sockaddr_un address;
	socklen_t len;
	size_t bufsize;
	char text[128];

	/* initialize the connection
	 */
	client->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (client->sockfd == -1) {
		fprintf(stderr, "error in socket %s\n", strerror(errno));
	}	/* if */

	address.sun_family = AF_UNIX;
	strcpy(address.sun_path, tmpnam(0));
	strcpy((char *)client->socket, address.sun_path);

	if (bind(client->sockfd, (const struct sockaddr *)(&address),
		 sizeof(struct sockaddr_un)))
	{
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
	sprintf(text, "display-memory:-input:shmid:%d", client->shmid);

	if ((client->visual = ggiOpen(text, NULL)) == NULL) {
		ggiPanic("Ouch - can't open the shmem target !");
	}	/* if */

	mode->frames = 1;
	mode->size.x = GGI_AUTO;
	mode->size.y = GGI_AUTO;
	ggiSetMode(client->visual, mode);

	/* start the program
	 */
	sprintf(text, "display-ipc:-input -socket=%s -semid=%d -shmid=%d",
		client->socket, client->semid, client->shmid);
	setenv("GGI_DISPLAY", text, 1);
	ggiSPrintMode(text, mode);
	setenv("GGI_DEFMODE", text, 1);
	client->pid = fork();
	if (client->pid == -1) {
		perror("fork");
		exit(-1);
	} else if (client->pid == 0) {
		execlp("/bin/sh","/bin/sh","-c", command, NULL);
		_exit(-1);
	}	/* if */

	listen(client->sockfd, 1);
	len = sizeof(struct sockaddr_un);
	client->sockfd = accept(client->sockfd,
			  (struct sockaddr *)(&address), &len);
	if (client->sockfd == -1) {
		perror("accept");
	}	/* if */

}	/* init_client */


void exit_client(client_t *client)
{
	ggiClose(client->visual);
	shmctl(client->shmid, IPC_RMID, NULL);
	unlink(client->socket);
}	/* exit_client */

/* return 1 if we got called by the client
 * return 0 if nothing is to do (either because we got
 *		interrupted by a signal, or because we
 *		just forwarded an event)
 * return -1 on error
 */
int wait_for_something(ggi_visual_t master, client_t *client)
{
	/* wait for the client to notify us
	 */
	fd_set fds;
	ggi_event event;
	ggi_event_mask mask;
	int nfds;

	FD_SET(client->sockfd, &fds);
	mask = emAll;
	nfds = ggiEventSelect(master, &mask, client->sockfd + 1, &fds, 0, 0, 0);
	if (nfds == -1) {
		if (errno == EINTR || errno == EAGAIN) errno = 0;
		return 0;
	} else if (nfds == 0) {
		ggiEventRead(master, &event, mask);
		ggiEventSend(client->visual, &event);
		return 0;
	} else if (FD_ISSET(client->sockfd, &fds)) {
		return 1;
	} else {
		return -1;
	}	/* if */

}	/* wait_for_something */


int repair_screen(client_t *client, ggi_visual_t visual)
{
	char tag;
	int region[4];

	/* read what region was damaged and repair it
	 */
	if (read(client->sockfd, &tag, 1) == 1 && /* read 'F' (like 'flush'); */
	   read(client->sockfd, (char *)region, 4*sizeof(int)) == 4*sizeof(int))
	{
		ggiCrossBlit(client->visual, region[0], region[1], region[2],
				region[3], visual, region[0], region[1]);
		ggiFlush(visual);
		return 1;
	}	/* if */
	return 0;
}	/* repair_screen */


int main(int argc, char **argv)
{
	ggi_visual_t visual;
	ggi_mode mode =
	{	/* This will cause the default mode to be set */
		1,			/* 1 frame */
		{GGI_AUTO,GGI_AUTO},	/* Default size */
		{GGI_AUTO,GGI_AUTO},	/* Virtual */
		{0,0},			/* size in mm don't care */
		GT_AUTO,		/* Mode */
		{GGI_AUTO,GGI_AUTO}	/* Font size */
	};

	const char *command;
	client_t client;

	/* Get the arguments from the command line. 
	 * Set defaults for optional arguments.
	 */
	if (argc == 1 || ((strcmp(*argv, "-h")==0) ||
	   (strcmp(*argv, "--help")==0)))
	{  
		fprintf(stderr, "Usage: %s <program>\n", argv[0]);
		exit(1);
		return 1;	/* Tell the user how to call and fail. */
	} else command = argv[1];

	/* Open up GGI and a visual.
	 */
	if (ggiInit() != 0) {
		fprintf(stderr, "unable to initialize LibGGI, exiting.\n");
		exit(1);
	}	/* if */

	visual = ggiOpen(NULL);

	if (visual == NULL) {
		fprintf(stderr, "unable to open default visual, exiting.\n");
		ggiExit();
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
		ggiExit();
		return 2;
	}	/* if */

	init_client(&client, &mode, command);

	/* run
	 */
	while (1) {
		int status = wait_for_something(visual, &client);
		if (!status) continue;
		if (status == -1 || !repair_screen(&client, visual)) break;
	};

	kill(client.pid, SIGHUP);
	exit_client(&client);
	ggiClose(visual);
	ggiExit();	

	return 0;
}	/* main */
