/* $Id: triple-int.c,v 1.1 2004/10/18 07:59:00 pekberg Exp $
******************************************************************************

   This is a regression-test for triple precision int math.

   Written in 2004 by Peter Ekberg

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
#include <ggi/errors.h>

/*#define HALF_MUL_DIVMOD_3  */ /* Uncomment to not use double type */
/*#define NAIVE_MUL_DIVMOD_3 */ /* Uncomment to use slow naive impl */

#include <ggi/internal/triple-int.h>

#include <string.h>

#include "testsuite.inc.c"

#define DIGITS (sizeof(unsigned)*2)

#ifndef HAVE_RANDOM
#define random  rand
#define srandom srand
#endif

static unsigned random_shift = 0;
static inline unsigned rnd(void)
{
	unsigned urnd = random();
	unsigned shift = BITS_3 - random_shift;
	while(shift < BITS_3) {
		urnd |= random() << shift;
		shift += BITS_3 - random_shift;
	}
	return urnd;
}


static void testcase1(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned z[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	assign_unsigned_3(x, INT_MIN);
	assign_unsigned_3(y, INT_MIN);
	mul_3(x, y);
	z[2] = 0; z[1] = ((unsigned)INT_MIN) >> 1; z[0] = 0;
	if (!eq_3(x, z)) {
		printfailure("Result   %0*x %0*x %0*x\n"
			"Expected %0*x %0*x %0*x\n",
			DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
			DIGITS, z[2], DIGITS, z[1], DIGITS, z[0]);
		return;
	}

	printsuccess();
	return;
}


static void testcase2(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned z[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	assign_unsigned_3(x, 0x59);
	assign_unsigned_3(y, 0x7f);
	mul_3(x, y);
	assign_unsigned_3(z, 0x2c27);
	if (!eq_3(x, z)) {
		printfailure("Result   %0*x %0*x %0*x\n"
			"Expected %0*x %0*x %0*x\n",
			DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
			DIGITS, z[2], DIGITS, z[1], DIGITS, z[0]);
		return;
	}

	printsuccess();
	return;
}


static void testcase3(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned z[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	x[2] = 0; x[1] = 0x59; x[0] = 0;
	y[2] = 0; y[1] = 0x7f; y[0] = 0;
	mul_3(x, y);
	z[2] = 0x2c27; z[1] = 0; z[0] = 0;
	if (!eq_3(x, z)) {
		printfailure("Result   %0*x %0*x %0*x\n"
			"Expected %0*x %0*x %0*x\n",
			DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
			DIGITS, z[2], DIGITS, z[1], DIGITS, z[0]);
		return;
	}

	printsuccess();
	return;
}

static void testcase4(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 7fff 8000 0000 / 0000 8000 0000 -> 0000 0000 ffff */
	a[2] = INT_MAX; a[1] = INT_MIN; a[0] = 0;
	b[2] = 0; b[1] = INT_MIN; b[0] = 0;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = UINT_MAX;
	y[2] = 0; y[1] = 0; y[0] = 0;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}

static void testcase5(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 7fff 8000 0000 / 0000 8000 ffff
	 * -> 0000 0000 fffd, remainder 0000 0003 fffd */
	a[2] = INT_MAX; a[1] = INT_MIN; a[0] = 0;
	b[2] = 0; b[1] = INT_MIN; b[0] = UINT_MAX;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = UINT_MAX-2;
	y[2] = 0; y[1] = 3; y[0] = UINT_MAX-2;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase6(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 4000 7ffe 0000 / 0000 4000 7fff
	 * -> 0000 0000 ffff, remainder 0000 3fff 7fff */
	a[2] = ((unsigned)INT_MIN) >> 1; a[1] = INT_MAX-1; a[0] = 0;
	b[2] = 0; b[1] = ((unsigned)INT_MIN) >> 1; b[0] = INT_MAX;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = UINT_MAX;
	y[2] = 0; y[1] = INT_MAX >> 1; y[0] = INT_MAX;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase7(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 4000 3ffe 0000 / 0000 4000 3fff
	 * -> 0000 0000 ffff, remainder 0000 3fff 3fff */
	a[2] = ((unsigned)INT_MIN) >> 1; a[1] = (INT_MAX >> 1) - 1; a[0] = 0;
	b[2] = 0; b[1] = ((unsigned)INT_MIN) >> 1; b[0] = INT_MAX >> 1;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = UINT_MAX;
	y[2] = 0; y[1] = INT_MAX >> 1; y[0] = INT_MAX >> 1;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase8(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 4000 7fff fffe / 4000 7fff ffff
	 * -> 0000 0000 0000, remainder 4000 7fff fffe */
	a[2] = ((unsigned)INT_MIN) >> 1; a[1] = INT_MAX; a[0] = UINT_MAX - 1;
	b[2] = ((unsigned)INT_MIN) >> 1; b[1] = INT_MAX; b[0] = UINT_MAX;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = 0;
	y[2] = ((unsigned)INT_MIN) >> 1; y[1] = INT_MAX; y[0] = UINT_MAX - 1;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase9(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	/* 4000 3fff fffe / 4000 3fff ffff
	 * -> 0000 0000 0000, remainder 4000 3fff fffe */
	a[2] = ((unsigned)INT_MIN)>>1; a[1] = INT_MAX>>1; a[0] = UINT_MAX-1;
	b[2] = ((unsigned)INT_MIN)>>1; b[1] = INT_MAX>>1; b[0] = UINT_MAX;
	divmod_3(a, b, q, r);
	x[2] = 0; x[1] = 0; x[0] = 0;
	y[2] = ((unsigned)INT_MIN)>>1; y[1] = INT_MAX>>1; y[0] = UINT_MAX-1;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase10(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 5db34a0c a6cfc5c0 2f22edcd / 00000001 e7cac13b 314ed328
	 * -> 312cdc88, remainder 00000001 ba6daff7 4cec608d */
	a[2] = 0x5db34a0c; a[1] = 0xa6cfc5c0; a[0] = 0x2f22edcd;
	b[2] = 0x00000001; b[1] = 0xe7cac13b; b[0] = 0x314ed328;
	divmod_3(a, b, q, r);
	x[2] = 0x00000000; x[1] = 0x00000000; x[0] = 0x312cdc88;
	y[2] = 0x00000001; y[1] = 0xba6daff7; y[0] = 0x4cec608d;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase11(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 6e111062 a97c4d8a de060f63 / 00000001 f1164edf e3bdfa7d
	 * -> 38af31e4, remainder 00000001 c7a6f5e3 186e0b0f */
	a[2] = 0x6e111062; a[1] = 0xa97c4d8a; a[0] = 0xde060f63;
	b[2] = 0x00000001; b[1] = 0xf1164edf; b[0] = 0xe3bdfa7d;
	divmod_3(a, b, q, r);
	x[2] = 0x00000000; x[1] = 0x00000000; x[0] = 0x38af31e4;
	y[2] = 0x00000001; y[1] = 0xc7a6f5e3; y[0] = 0x186e0b0f;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase12(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 791c4932 60bed7bb 8b9a72fd / 00000000 00000001 e8e316ab
	 * -> 3f6b0838 eeb88a41, remainder 00000001 bad18392 */
	a[2] = 0x791c4932; a[1] = 0x60bed7bb; a[0] = 0x8b9a72fd;
	b[2] = 0x00000000; b[1] = 0x00000001; b[0] = 0xe8e316ab;
	divmod_3(a, b, q, r);
	x[2] = 0x00000000; x[1] = 0x3f6b0838; x[0] = 0xeeb88a41;
	y[2] = 0x00000000; y[1] = 0x00000001; y[0] = 0xbad18392;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase13(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 7b99a710 7be0a6ad 47228cf1 / 00000000 00000003 e2b501f2
	 * -> 1fcf5d8a efd713ea, remainder 00000003 0e4bcfbd */
	a[2] = 0x7b99a710; a[1] = 0x7be0a6ad; a[0] = 0x47228cf1;
	b[2] = 0x00000000; b[1] = 0x00000003; b[0] = 0xe2b501f2;
	divmod_3(a, b, q, r);
	x[2] = 0x00000000; x[1] = 0x1fcf5d8a; x[0] = 0xefd713ea;
	y[2] = 0x00000000; y[1] = 0x00000003; y[0] = 0x0e4bcfbd;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase14(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 3ff97f5e e6e4c675 10e81a4c / 0000000f e504a2ed 963fe07c
	 * -> 0406618f, remainder 0000000f afa738ce bc42b908 */
	a[2] = 0x3ff97f5e; a[1] = 0xe6e4c675; a[0] = 0x10e81a4c;
	b[2] = 0x0000000f; b[1] = 0xe504a2ed; b[0] = 0x963fe07c;
	divmod_3(a, b, q, r);
	x[2] = 0x00000000; x[1] = 0x00000000; x[0] = 0x0406618f;
	y[2] = 0x0000000f; y[1] = 0xafa738ce; y[0] = 0xbc42b908;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase15(const char *desc)
{
	unsigned x[3];
	unsigned y[3];
	unsigned q[3];
	unsigned r[3];
	unsigned a[3];
	unsigned b[3];

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	if (BITS_3 != 32) {
		printsuccess();
		return;
	}

	/* 7a9428dc 35ce3c4f f2800602 / 00000000 00000000 00003dab
	 * -> 0001fcdb 5142dada 6ed18467, remainder 00000a35 */
	a[2] = 0x7a9428dc; a[1] = 0x35ce3c4f; a[0] = 0xf2800602;
	b[2] = 0x00000000; b[1] = 0x00000000; b[0] = 0x00003dab;
	divmod_3(a, b, q, r);
	x[2] = 0x0001fcdb; x[1] = 0x5142dada; x[0] = 0x6ed18467;
	y[2] = 0x00000000; y[1] = 0x00000000; y[0] = 0x00000a35;
	if (eq_3(q, x) && eq_3(r, y)) {
		printsuccess();
		return;
	}

	printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
		"Wrong quotient or remainder:\n"
		"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
		"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
		DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
		DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
		DIGITS, q[2], DIGITS, q[1], DIGITS, q[0],
		DIGITS, r[2], DIGITS, r[1], DIGITS, r[0],
		DIGITS, x[2], DIGITS, x[1], DIGITS, x[0],
		DIGITS, y[2], DIGITS, y[1], DIGITS, y[0]);
}


static void testcase16(const char *desc)
{
	int i;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	for (i = 0; i < 1000000; ++i) {
		unsigned a[3];
		unsigned b[3];
		unsigned c[3];
		unsigned d[3];
		unsigned e[3];
		unsigned f[3];

		a[2] = 0; a[1] = 0; a[0] = rnd();
		do {
			b[2] = 0; b[1] = rnd() >> 1; b[0] = rnd();
			c[2] = 0; c[1] = 0; c[0] = rnd();
		} while (eq0_3(b) || ge_3(c, b));
		assign_3(d, a);
		mul_3(d, b);
		add_3(d, c);

		divmod_3(d, b, e, f);
		if (eq_3(e, a) && eq_3(f, c))
			continue;continue;

		printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
			"Wrong quotient or remainder:\n"
			"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
			"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
			DIGITS, d[2], DIGITS, d[1], DIGITS, d[0],
			DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
			DIGITS, e[2], DIGITS, e[1], DIGITS, e[0],
			DIGITS, f[2], DIGITS, f[1], DIGITS, f[0],
			DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
			DIGITS, c[2], DIGITS, c[1], DIGITS, c[0]);
		return;
	}

	printsuccess();
	return;
}


static void testcase17(const char *desc)
{
	unsigned i;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	for (i = 0; i < 1000000; ++i) {
		unsigned bits = i % (BITS_3 * 3 - 2) + 1;
		unsigned a[3];
		unsigned b[3];
		unsigned c[3];
		unsigned d[3];
		unsigned e[3];
		unsigned f[3];

badluck:
		a[2] = rnd() >> 1; a[1] = rnd(); a[0] = rnd();
		a[2] |= (unsigned)INT_MIN;
		a[2] >>= 1;
		rshift_3(a, BITS_3 * 3 - bits);
		do {
			b[2] = rnd() >> 1; b[1] = rnd(); b[0] = rnd();
			b[2] |= (unsigned)INT_MIN;
			b[2] >>= 1;
			rshift_3(b, bits - 1);
			c[2] = rnd() >> 1; c[1] = rnd(); c[0] = rnd();
			rshift_3(c, bits - 1);
		} while (eq0_3(b) || ge_3(c, b));
		assign_3(d, a);
		mul_3(d, b);
		add_3(d, c);
		if (lt0_3(d))
			goto badluck;

		divmod_3(d, b, e, f);
		if (eq_3(e, a) && eq_3(f, c))
			continue;
		continue;

		printfailure("divmod %0*x %0*x %0*x / %0*x %0*x %0*x\n"
			"Wrong quotient or remainder:\n"
			"Result   q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n"
			"Expected q = %0*x %0*x %0*x, r = %0*x %0*x %0*x\n",
			DIGITS, d[2], DIGITS, d[1], DIGITS, d[0],
			DIGITS, b[2], DIGITS, b[1], DIGITS, b[0],
			DIGITS, e[2], DIGITS, e[1], DIGITS, e[0],
			DIGITS, f[2], DIGITS, f[1], DIGITS, f[0],
			DIGITS, a[2], DIGITS, a[1], DIGITS, a[0],
			DIGITS, c[2], DIGITS, c[1], DIGITS, c[0]);
		return;
	}

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite triple precision int math\n\n");

	unsigned tmp_rand = RAND_MAX;
	while (tmp_rand < (unsigned)INT_MIN) {
		++random_shift;
		tmp_rand <<= 1;
	}
	srandom(time(NULL));

	testcase1("00..00 00..00 80..00 * 00..00 00..00 80..00");
	testcase2("00..00 00..00 0..059 * 00..00 00..00 0..07f");
	testcase3("00..00 0..059 00..00 * 00..00 0..07f 00..00");
	testcase4("7f..ff 80..00 00..00 / 00..00 80..00 00..00");
	testcase5("7f..ff 80..00 00..00 / 00..00 80..00 ff..ff");
	testcase6("40..00 7f..fe 00..00 / 00..00 40..00 7f..ff");
	testcase7("40..00 3f..fe 00..00 / 00..00 40..00 3f..ff");
	testcase8("40..00 7f..ff ff..fe / 40..00 7f..ff ff..ff");
	testcase9("40..00 3f..ff ff..fe / 40..00 3f..ff ff..ff");
	testcase10("5db34a0c a6cfc5c0 2f22edcd / 00000001 e7cac13b 314ed328");
	testcase11("6e111062 a97c4d8a de060f63 / 00000001 f1164edf e3bdfa7d");
	testcase12("791c4932 60bed7bb 8b9a72fd / 00000000 00000001 e8e316ab");
	testcase13("7b99a710 7be0a6ad 47228cf1 / 00000000 00000003 e2b501f2");
	testcase14("3ff97f5e e6e4c675 10e81a4c / 0000000f e504a2ed 963fe07c");
	testcase15("7a9428dc 35ce3c4f f2800602 / 00000000 00000000 00003dab");
	testcase16("(n*d+r)/d, n,d,r>0, d>r, should give n with remainder r, "
		"1000000 tests with random data");
	testcase17("(n*d+r)/d, n,d,r>0, d>r, should give n with remainder r, "
		"1000000 tests with random data of other characteristics");

	printsummary();

	return 0;
}
