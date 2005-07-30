/* $Id: linmm_banked.h,v 1.2 2005/07/30 11:40:02 cegger Exp $
******************************************************************************

   linmm_banked header file

   Copyright (C) 1998 Brian S. Julin	[bri@forcade.calyx.com]

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

#ifndef _LINMM_BANKED_H
#define _LINMM_BANKED_H

extern uint8_t *__localafb;
extern uint8_t *__localdfb;
extern uint8_t *__localrfb;
extern uint8_t *__localwfb;

/* Later these get fed from visual.c or CHECKXY global variables */
#define BANKFB __localafb
#define RBANKFB __localrfb
#define LOGBYTPP 0

#endif /* _LINMM_BANKED_H */
