/* $Id: blit32.c,v 1.1 2001/05/12 23:02:37 cegger Exp $
******************************************************************************

   Display-trueemu : blit32

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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
#include <stdlib.h>

#include <ggi/internal/ggi-dl.h>

#include <ggi/display/trueemu.h>


/**************************************************
 ***
 ***  Blit functions for 32 bit BGR0 source
 ***
 **************************************************/


#define R_OFF  2
#define G_OFF  1
#define B_OFF  0

#define SRC_STEP  4


#include "genblit.c"


TrueemuBlits _ggi_trueemu_blit32_table =
{
	_ggi_trueemu_blit_b32_d0,
	_ggi_trueemu_blit_b24_d0,

	_ggi_trueemu_blit_b16_d0,
	_ggi_trueemu_blit_b16_d2_ev,
	_ggi_trueemu_blit_b16_d2_od,
	_ggi_trueemu_blit_b16_d4_ev,
	_ggi_trueemu_blit_b16_d4_od,
	
	_ggi_trueemu_blit_b8_d0,
	_ggi_trueemu_blit_b8_d2_ev,
	_ggi_trueemu_blit_b8_d2_od,
	_ggi_trueemu_blit_b8_d4_ev,
	_ggi_trueemu_blit_b8_d4_od,
	
	_ggi_trueemu_blit_b4_d0,
	_ggi_trueemu_blit_b4_d2_ev,
	_ggi_trueemu_blit_b4_d2_od,
	_ggi_trueemu_blit_b4_d4_ev,
	_ggi_trueemu_blit_b4_d4_od,
};
