/* $Id: common.inc.c,v 1.5 2004/05/27 08:44:19 cegger Exp $
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


#define EXPECTED2PASS	0
#define EXPECTED2FAIL	1


/* global variables */
static int num_tests = 0;
static int num_successfultests = 0;
static int num_failedtests = 0;
static int num_asserterrors = 0;

static int expected_current_testresult = EXPECTED2PASS;


static void printteststart(const char *file, const char *funcname, int expected)
{
	printf("%s: Running %s...", file, funcname);
	fflush(stdout);

	num_tests++;
	expected_current_testresult = expected;
}


#define printassert(x, fmt...)	\
	if (!(x)) {		\
		printf(fmt);	\
		fflush(stdout);\
		num_asserterrors++;	\
	}


static void printsuccess(void)
{
	switch (expected_current_testresult) {
	case EXPECTED2PASS:
		printf("passed, as expected\n");
		break;
	case EXPECTED2FAIL:
		printf("passed - UNEXPECTED!\n");
		break;
	}
	fflush(stdout);

	num_successfultests++;
}


static void printfailure(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	switch (expected_current_testresult) {
	case EXPECTED2PASS:
		printf("failed - UNEXPECTED!\n");
		break;
	case EXPECTED2FAIL:
		printf("failed, as expected\n");
		break;
	}
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
