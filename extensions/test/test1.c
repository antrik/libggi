/* $Id: test1.c,v 1.4 2004/11/13 16:07:00 cegger Exp $
******************************************************************************

   Test extension test1.c

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

#include <ggi/internal/internal.h>

#include <stdio.h>
#include <string.h>

#include "test1.h"

/*
 * The API of Extension #1 looks a bit like this :
 */

/* Extension ID. Defaulting to -1 should make segfault on abuse more likely ... */
ggi_extid ggiTest1ID=-1;

static int changed(ggi_visual_t vis,int whatchanged)
{
	printf("changed called for extension 1 - vis=%p, %i \n",
		(void *)vis, whatchanged);

	switch(whatchanged) {
	case GGI_CHG_APILIST:
		{	int temp;
			char api[GGI_MAX_APILEN];
			char args[GGI_MAX_APILEN];
			for (temp=0;
			    0 == ggiGetAPI(vis, temp, api, args);
			    temp++)
			{
				ggstrlcat(api,"-test1", sizeof(api));
				printf("Would now load #%d: %s(%s)\n",
					temp, api, args);
			}
		}
		break;
	}

	return 0;
}

int ggiTest1Init(void)
{
	ggiTest1ID=ggiExtensionRegister("Ext1",123,changed);
	printf("Initialized Test1 extension. ID: %i\n",ggiTest1ID);

	return ggiTest1ID >= 0 ? 0 : -1;
}

int ggiTest1Exit(void)
{
	int rc;
	rc=ggiExtensionUnregister(ggiTest1ID);
	printf("DeInitailized Test1 extension. rc=%i\n", rc);

	return rc;
}

int ggiTest1Attach(ggi_visual_t vis)
{
	int rc;
	rc=ggiExtensionAttach(vis,ggiTest1ID);
	printf("Attached Test1 extension to %p. rc=%i\n",
		(void *)vis, rc);

	if (rc==0) {	/* We are actually creating the primary instance. */
		strcpy(LIBGGI_EXT(vis,ggiTest1ID),"Test 1 private Data !");
		/* Now fake an "API change" so the right libs get loaded */
		changed(vis,GGI_CHG_APILIST);
	}

	return rc;
}

int ggiTest1Detach(ggi_visual_t vis)
{
	int rc;
	rc=ggiExtensionDetach(vis,ggiTest1ID);
	printf("Detached Test1 extension from %p. rc=%i\n",
		(void *)vis, rc);

	return rc;
}

void ggiTest1PrintLocaldata(ggi_visual_t vis)
{
	printf("%s\n",(char *)LIBGGI_EXT(vis,ggiTest1ID));
}

void ggiTest1SetLocaldata  (ggi_visual_t vis,char *content)
{
	strcpy(LIBGGI_EXT(vis,ggiTest1ID),content);
}
