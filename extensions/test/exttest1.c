/* $Id: exttest1.c,v 1.9 2007/12/17 22:31:49 cegger Exp $
******************************************************************************

   Test extension test.

   Copyright (C) 1997 Uwe Maurer - uwe_maurer@t-online.de
   Copyright (C) 1998 Andreas Beck - becka@ggi-project.org
  
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

#include "config.h"
#include <ggi/ggi.h>

#include <stdio.h>

#include "test1.h"
#include "test2.h"

int
main(void)
{
	ggi_visual_t vis1,vis2;
	ggi_mode mode;
	int temp;
	ggInit();
	vis1 = ggNewStem(libggi, NULL);
	printf("O.K.\n");

	temp = ggiTest1Init(); printf("Init1 : %i\n",temp);
	temp = ggiTest1Init(); printf("Init1b: %i\n",temp);

	ggiOpen(vis1, "display-memory",NULL);
	ggiCheckSimpleMode(vis1, 320, 200, 1, GT_8BIT, &mode);
	ggiSetMode(vis1, &mode);
	ggiCheckSimpleMode(vis1, 320, 400, 1, GT_8BIT, &mode);
	ggiSetMode(vis1, &mode);

	ggiTest1Attach(vis1);
	ggiCheckSimpleMode(vis1, 320, 400, 1, GT_8BIT, &mode);
	ggiSetMode(vis1, &mode);
	ggiTest1Attach(vis1);
	ggiTest1PrintLocaldata(vis1);
/*	ggiTest2PrintLocaldata(vis1); This is illegal. Should hopefully segfault. */
	ggiTest1SetLocaldata  (vis1,"Localdata1 changed.");
	ggiTest1PrintLocaldata(vis1);
	ggiTest1Detach(vis1);
	ggiTest1Detach(vis1);
	ggiTest1Detach(vis1);

	vis2 = ggNewStem(libggi, NULL);
	temp = ggiTest2Init(); printf("Init2 : %i\n",temp);

	ggiOpen(vis2, "display-memory",NULL);
	ggiCheckSimpleMode(vis2, 320, 200, 1, GT_8BIT, &mode);
	ggiSetMode(vis1, &mode);

	ggiTest2Attach(vis2);
	ggiTest2Attach(vis2);
	ggiTest2PrintLocaldata(vis2);
	ggiTest2SetLocaldata  (vis2,"Hello ! Test !");
	ggiTest2PrintLocaldata(vis2);
	ggiTest2Detach(vis2);
	ggiTest2Detach(vis2);
	ggiTest2Detach(vis2);

	ggiClose(vis1);
	ggiClose(vis2);

	ggiTest2Exit();
	ggiTest2Exit();

	ggiTest1Exit();
	ggiTest1Exit();
	ggiTest1Exit();

	ggiExit();
	ggExit();
	return 0;
}
