/* $Id: input.c,v 1.4 2004/08/09 17:33:07 ggibecka Exp $
******************************************************************************

   This is a regression-test for visual <-> input association.

   Written in 2004 by Christoph Egger

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

#include <string.h>

#include "testsuite.inc.c"

typedef struct ggi_originbounds {
	uint32 first,last;
} ggi_originbounds_t;

ggi_visual_t vis;
ggi_mode mode;
gii_input_t inp;

static int ggi_findorigin(ggi_visual_t visual,ggi_originbounds_t *origins) {

	gii_input_t     input;
	gii_cmddata_getdevinfo dummy;
	uint32 n;
	uint32 origin;
	
	input=ggiGetInput(visual);
	origins->first=0xffffffff;origins->last=0x00000000;
	for(n=0;0==giiQueryDeviceInfoByNumber(input,n,&origin,&dummy);n++) {
		if (origins->first>origin) origins->first=origin;
		if (origins->last <origin) origins->last =origin;
	}
	return (origins->first>origins->last);
}


static int precase(void)
{
	if (dontrun) return 0;

	ggiInit();

	vis = ggiOpen(NULL);

	ggiCheckSimpleMode(vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);

	ggiSetMode(vis, &mode);

	return 0;
}

static int postcase(void)
{
	if (dontrun) return 0;

	ggiClose(vis);
	ggiExit();

	return 0;
}


static void testcase1(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	inp = ggiDetachInput(vis);


	if (inp == NULL) {
		printfailure("expected return value: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return;
	}


	printsuccess();
	return;
}


static void testcase2(const char *desc)
{
	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	inp = ggiJoinInputs(vis, inp);

	if (inp == NULL) {
		printfailure("expected return value: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return;
	}

	printsuccess();
	return;
}

static int ggi_opentestvis(ggi_visual_t *visual,ggi_originbounds_t *ori, 
			    gii_input_t *input, char *visname) {

	*visual=ggiOpen(NULL);
	if (*visual == NULL) {
		printfailure("expected return value for ggiOpen: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return -1;
	}
	ggiCheckSimpleMode(*visual, GGI_AUTO, GGI_AUTO, GGI_AUTO, GT_AUTO, &mode);
	ggiSetMode(*visual, &mode);
	if (ggi_findorigin(*visual,ori)) {
		printfailure("expected return value for findorigin: 0\n"
			"actual return value: !=0\n");
		return -1;
	}
	*input = giiJoinInputs(*input,ggiDetachInput(*visual));
	if (*input == NULL) {
		printfailure("joined inps: expected return value: \"non-NULL\"\n"
			"actual return value: \"NULL\"\n");
		return -1;
	}
	return 0;
}

static int ggi_closetestvis(ggi_visual_t *visual,ggi_originbounds_t *ori, 
			    gii_input_t *input, char *visname,int islast) {

	uint32 origins;
	gii_input_t newhand;
	int haveone=0;
	int errcode;

	for(origins=ori->first;origins<=ori->last;origins++) {
		switch(errcode=giiSplitInputs(*input,&newhand,origins,0)) {
			case 0:
				giiClose(newhand);
				haveone=1;
				break;
			case 1:
				giiClose(*input);
				*input=newhand;
				haveone=1;
				break;
			default:
				// fprintf(stderr,"ERROR: %d\n",errcode);
				break;
		}
	}
	if (islast) {
		if (haveone) {
			printfailure("last haveone expected return value: 0\n"
				"actual return value: !=0\n");
			return -1;
		}
		giiClose(*input);
	} else {
		if (!haveone) {
			printfailure("haveone expected return value: !=0\n"
				"actual return value: 0\n");
			return -1;
		}
	}
	ggiClose(*visual);
	return 0;
}

static void testcase3(const char *desc)
{
	ggi_visual_t vis1,vis2;
	ggi_originbounds_t oris1,oris2;
	gii_input_t joinedinps;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	joinedinps=NULL;
	
	if (ggi_opentestvis(&vis1,&oris1,&joinedinps,"vis1")) return;
	if (ggi_opentestvis(&vis2,&oris2,&joinedinps,"vis2")) return;

	if (ggi_closetestvis(&vis1,&oris1,&joinedinps,"vis1",0)) return;
	if (ggi_closetestvis(&vis2,&oris2,&joinedinps,"vis2",1)) return;

	printsuccess();
	return;
}

static void testcase4(const char *desc)
{
	ggi_visual_t vis1,vis2;
	ggi_originbounds_t oris1,oris2;
	gii_input_t joinedinps;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	joinedinps=NULL;
	
	if (ggi_opentestvis(&vis1,&oris1,&joinedinps,"vis1")) return;
	if (ggi_opentestvis(&vis2,&oris2,&joinedinps,"vis2")) return;

	if (ggi_closetestvis(&vis2,&oris2,&joinedinps,"vis2",0)) return;
	if (ggi_closetestvis(&vis1,&oris1,&joinedinps,"vis1",1)) return;

	printsuccess();
	return;
}


int main(int argc, char * const argv[])
{
	int rc;

	parseopts(argc, argv);
	printdesc("Regression testsuite visual <-> input association\n\n");

	rc = precase();
	if (rc != 0) exit(rc);

	testcase1("Check that input detaches from visual correctly.");
	testcase2("Check that input attaches to visual correctly.");
	testcase3("Check that joining/splitting multiple visuals works (overlapping).");
	testcase4("Check that joining/splitting multiple visuals works (bracketed).");

	rc = postcase();
	if (rc != 0) exit(rc);

	printsummary();

	return 0;
}
