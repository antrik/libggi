/* $Id: findleaks.c,v 1.11 2005/01/29 08:47:45 cegger Exp $
******************************************************************************

   Helps to find memory leaks in LibGGI and targets.

   Written in 1998 by Marcus Sundberg	[marcus@ggi-project.org]

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

#include "config.h"
#include <ggi/ggi.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <fcntl.h>

#define _get_ggi_alloced() 0

static ggi_visual_t vis;

#if 0
static void wait4key(void)
{
	getchar();
}
#endif

static void err(char *s,...)
{
	printf("Fatal error: ");
	printf(s);
	exit(1);
}

#if defined(__linux__ ) || defined(__CYGWIN__)
static int pid;
static char statpath[1024];

static void fill_info(void)
{
	pid = getpid();
	sprintf(statpath, "/proc/%d/status", pid);
}

static long get_size(void)
{
	long size=0;
	FILE *fil;
	char buf[1024];

	if ((fil = fopen(statpath, "r")) == NULL) {
		err("opening %s\n", statpath);
	}
	while (fgets(buf, 1023, fil) != NULL) {
		if (fscanf(fil, "VmSize:%ld", &size) == 1) break;
	}
	fclose(fil);
	return size;
}


static void
inform_mem(char *info, long prev, long now, long allocprev, long allocnow)
{
	printf(info);
	printf("Used before   : %6ld, after: %6ld,  Change: %6ld\n",
		   prev, now, now-prev);
	printf("Alloced before: %6ld, after: %6ld,  Change: %6ld\n",
		   allocprev, allocnow, allocnow-allocprev);
	fflush(stdout);
	fflush(stderr);
}

#else

static void fill_info(void)
{
	fprintf(stderr, "fill_info(): not implemented for your platform\n");
}

static long get_size(void)
{
	fprintf(stderr, "get_size(): not implemented for your platform\n");
	return 0;
}

static void inform_mem(char *info, long prev, long now, long allocprev, long allocnow)
{
	printf(info);
	fprintf(stderr, "inform_mem(): not implemented for your platform\n");
	printf("Check memory stats now, then press any key\n");
	fflush(stdout);
	fflush(stderr);
	getchar();
}
#endif


int
main(int argc, char *argv[])
{
	int i;
	long binit, a_binit;
	long prev, curr;
	long aprev, acurr;

	fill_info();

	binit = get_size();
	a_binit = _get_ggi_alloced();
	if (ggiInit() != 0)
		err("first ggiInit() failed!\n");

	prev = get_size();
	aprev = _get_ggi_alloced();
	for (i=2; i <= 80; i++) {
		putchar('.');
		fflush(stdout);
		ggiExit();
		if (ggiInit() != 0)
			err("ggiInit() number %d failed\n", i);
	}
	curr = get_size();
	acurr = _get_ggi_alloced();
	inform_mem("\nggiInit()\n", prev, curr, aprev, acurr);

	if ((vis = ggiOpen(NULL)) == NULL)
		err("first ggiOpen() failed!\n");

	prev = get_size();
	aprev = _get_ggi_alloced();
	for (i=2; i < 20; i++) {
		putchar('.');
		fflush(stdout);
		ggiClose(vis);
		if ((vis = ggiOpen(NULL)) == NULL)
			err("ggiOpen() number %d failed\n", i);
	}
	curr = get_size();
	acurr = _get_ggi_alloced();
	inform_mem("\nggiOpen()\n", prev, curr, aprev, acurr);

	prev = get_size();
	aprev = _get_ggi_alloced();
	for (i=2; i < 20; i++) {
		putchar('.');
		fflush(stdout);
		ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO,
				   NULL);
	}
	curr = get_size();
	acurr = _get_ggi_alloced();
	inform_mem("\nggiCheckSimpleMode()\n", prev, curr, aprev, acurr);

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	if (ggiSetSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO) != 0) {
		err("Unable to set default mode!\n");
	}
	prev = get_size();
	aprev = _get_ggi_alloced();
	for (i=2; i < 20; i++) {
		putchar('.');
		fflush(stdout);
		if (ggiSetSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO)
		    != 0) {
			err("Unable to set default mode %d!\n", i);
		}
	}
	curr = get_size();
	acurr = _get_ggi_alloced();
	inform_mem("\nggiSetSimpleMode()\n", prev, curr, aprev, acurr);

	ggiExit();

	curr = get_size();
	acurr = _get_ggi_alloced();
	inform_mem("\nggiExit()\n", binit, curr, a_binit, acurr);

	return 0;
}
