/* $Id: findleaks.c,v 1.1 2001/05/12 23:03:32 cegger Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <ggi/ggi.h>

#if !defined(DEBUG) || !(__GLIBC__ >= 2)
#define _get_ggi_alloced() 0
#endif

ggi_visual_t vis;

void
wait4key(void)
{
	getchar();
}

void
err(char *s,...)
{
	printf("Fatal error: ");
	printf(s);
	exit(1);
}

#ifdef __linux__
int pid;
char statpath[1024];

void
fill_info(void)
{
	pid = getpid();
	sprintf(statpath, "/proc/%d/status", pid);
}

long
get_size(void)
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


void
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

void
fill_info(void)
{
}

long
get_size(void)
{
	return 0;
}

void
inform_mem(char *info, long prev, long now, long allocprev, long allocnow)
{
	printf(info);
	printf("Check memory stats now!\n");
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
