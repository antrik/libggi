/* $Id: common.inc.c,v 1.1 2004/05/03 22:39:41 cegger Exp $
******************************************************************************

   common.c - framework for c based regression tests

   Copyright (C) 2004 Christoph Egger

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/

#include "config.h"

#include <stdio.h>
#include <stdarg.h>


/* global variables */
static int num_tests = 0;
static int num_successfultests = 0;
static int num_failedtests = 0;
static int num_asserterrors = 0;


static void printteststart(const char *funcname)
{
	printf("%s: Run %s...", __FILE__, funcname);
	fflush(stdout);

	num_tests++;
}


#define printassert(x, fmt...)	\
	if (!(x)) {		\
		printf(fmt);	\
		fflush(stdout));\
		num_asserterrors++;	\
	}


static void printsuccess(void)
{
	printf("passed\n");
	fflush(stdout);

	num_successfultests++;
}


static void printfailure(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	printf("failed!\n");
	vprintf(fmt, ap);
	fflush(stdout);

	va_end(ap);

	num_failedtests++;
}


static void printsummary(void)
{
	printf("\nSummary:\n");
	printf("%2d tests run\n", num_tests);
	printf("%2d tests successful\n", num_successfultests);
	printf("%2d tests failed\n", num_failedtests);
	printf("%2d assert failures\n", num_asserterrors);
	fflush(stdout);
}
