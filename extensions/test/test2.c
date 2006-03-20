/* $Id: test2.c,v 1.6 2006/03/20 17:50:01 pekberg Exp $
******************************************************************************

   Test extension test2.c

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
#include <ggi/internal/internal.h>

#include <stdio.h>
#include <string.h>

#include "test1.h"
#include "test2.h"

/*
 * The API of Extension #2 looks a bit like this :
 * It happens to need Extension #1 internally.
 */

/* Extension ID. Defaulting to -1 should make segfault on abuse more likely ... */
ggi_extid ggiTest2ID=-1;

static int changed(struct ggi_visual *vis,int whatchanged)
{
	printf("load extension 2 - vis: %p %i\n",
		(void *)vis, whatchanged);
	return 0;
}

int ggiTest2Init(void)
{
	ggiTest1Init();	/* Make sure #1 is there - we need it */

	ggiTest2ID=ggiExtensionRegister("Ext2",123,changed);
	printf("Initialized Test2 extension. ID: %i\n",ggiTest2ID);

	return ggiTest2ID >= 0 ? 0 : -1;
}

int ggiTest2Exit(void)
{
	int rc;
	rc=ggiExtensionUnregister(ggiTest2ID);
	printf("DeInitailized Test2 extension. rc=%i\n", rc);

	if (rc>=0) ggiTest1Exit();	/* Release #1. */

	return rc;
}

int ggiTest2Attach(ggi_visual_t v)
{
	int rc;
	struct ggi_visual *vis;
	rc=ggiExtensionAttach(v,ggiTest2ID);
	printf("Attached Test2 extension to %p. rc=%i\n",
		(void *)v, rc);

	if (rc==0) {	/* We are actually creating the primary instance. */
		ggiTest1Attach(v);
		vis = STEM_API_DATA(v,libggi,struct ggi_visual *);
		strcpy(LIBGGI_EXT(vis,ggiTest2ID),"Test 2 private Data !");
	}

	return rc;
}

int ggiTest2Detach(ggi_visual_t v)
{
	int rc;
	rc=ggiExtensionDetach(v,ggiTest2ID);
	printf("Detached Test2 extension from %p. rc=%i\n",
		(void *)v, rc);
	if (rc==0) ggiTest1Detach(v);

	return rc;
}

void ggiTest2PrintLocaldata(ggi_visual_t v)
{
	struct ggi_visual *vis = STEM_API_DATA(v,libggi,struct ggi_visual *);
	ggiTest1PrintLocaldata(v);
	printf("%s\n",(char *)LIBGGI_EXT(vis,ggiTest2ID));
}

void ggiTest2SetLocaldata  (ggi_visual_t v,const char *content)
{
	struct ggi_visual *vis = STEM_API_DATA(v,libggi,struct ggi_visual *);
	strcpy(LIBGGI_EXT(vis,ggiTest2ID),content);
}

