/* $Id: testsuite.inc.c,v 1.1 2004/07/28 09:25:20 cegger Exp $
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif


/* EXPECTED2PASS is used for testcases that are expected to pass.
 * That is for most testcases.
 */
#define EXPECTED2PASS	0

/* EXPECTED2FAIL is used for testcases that are expcted to fail.
 * That is almost useful to test error handling.
 */
#define EXPECTED2FAIL	1


/* global variables */
static int num_tests = 0;
static int num_successfultests = 0;
static int num_failedtests = 0;
static int num_asserterrors = 0;

static int expected_current_testresult = EXPECTED2PASS;

static int verbose = 0;



static void printdesc(const char *desc)
{
	if (!verbose) return;

	printf("%s", desc);
	fflush(stdout);

	return;
}


static void printteststart(const char *file, const char *funcname,
		int expected, const char *description)
{
	if (verbose) {
		printf("%s: %s: %s\n", file, funcname, description);
	}
	printf("%s: Running %s...", file, funcname);
	fflush(stdout);

	num_tests++;
	expected_current_testresult = expected;
}


#define printassert(x, fmt...)	\
	if (!(x)) {		\
		printf(fmt);	\
		fflush(stdout);	\
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



static void printhelp(const char *name)
{
	printf("Usage:  %s [OPTION]\n", name);
	printf("Options:\n");
	printf("    -h      Shows this help\n");
	printf("    -v      Print testcase descriptions\n");
	fflush(stdout);
	exit(0);
}

static void parseopts(int argc, char * const argv[])
{
	int ch;

	while ((ch = getopt(argc, argv, "hv")) != -1) {
		switch (ch) {
		case 'v':
			verbose = 1;
			break;
		case 'h':
		default:
			printhelp(argv[0]);
		}
	}

	argc -= optind;
	argv += optind;

	return;
}

