/* $Id: get.c,v 1.2 2007/04/04 13:56:32 ggibecka Exp $
******************************************************************************

   This is a regression-test for Get function handling.

   Adapted from mode regression test written in 2004 by Christoph Egger
   Written in 2007 by Andreas Beck

   This software is placed in the public domain and can be used freely
   for any purpose. It comes without any kind of warranty, either
   expressed or implied, including, but not limited to the implied
   warranties of merchantability or fitness for a particular purpose.
   Use it at your own risk. the author is not responsible for any damage
   or consequences raised by use or inability to use this program.

******************************************************************************
*/


#include "config.h"
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gii.h>
#include <ggi/ggi.h>
#include <ggi/errors.h>

#include <string.h>

#include "testsuite.inc.c"

#define REPEATS 100
struct teststruct {
	unsigned char *changed_pixels;
	ggi_color *getcol,*refcol;
	ggi_pixel *getpix,*reference;
};

static void free_teststruct(struct teststruct *ts) 
{
	if (ts->changed_pixels) free(ts->changed_pixels);
	if (ts->getcol) free(ts->getcol);
	if (ts->refcol) free(ts->refcol);
	if (ts->getpix) free(ts->getpix);
	if (ts->reference) free(ts->reference);
}

static int allocate_teststruct(struct teststruct *ts,int size) 
{
	/* Preset all fields to NULL so freeing is easy
	 */
	ts->changed_pixels=NULL;
	ts->getcol=ts->refcol=NULL;
	ts->getpix=ts->reference=NULL;
	/* Allocate all fields
	 */
	ts->changed_pixels=malloc(size*sizeof(ts->changed_pixels[0]));
	ts->getcol=malloc(size*sizeof(ts->getcol[0]));
	ts->refcol=malloc(size*sizeof(ts->refcol[0]));
	ts->getpix=malloc(size*sizeof(ts->getpix[0]));
	ts->reference=malloc(size*sizeof(ts->reference[0]));
	/* If anything failed, free and fail.
	 */
	if (!ts->changed_pixels || 
	    !ts->getcol ||
	    !ts->refcol ||
	    !ts->getpix ||
	    !ts->reference) {
	    	free_teststruct(ts);
		return 1;
	}
	/* Init the changed pixel array
	 */
	memset(ts->changed_pixels,0,size*sizeof(ts->changed_pixels[0]));
	return 0;
}

static void randomize_teststruct(ggi_visual_t vis,struct teststruct *ts,int size) 
{
	int i;
	
	for(i=0;i<size;i++) {
		ts->getcol[i].r=ts->refcol[i].r=random();
		ts->getcol[i].g=ts->refcol[i].g=random();
		ts->getcol[i].b=ts->refcol[i].b=random();
	}
	ggiPackColors(vis,ts->getpix   ,ts->getcol,size);
	ggiPackColors(vis,ts->reference,ts->refcol,size);
}

static void check_teststruct(ggi_visual_t vis,struct teststruct *ts,int size) 
{
	int i;
	
	ggiUnpackPixels(vis,ts->getpix   ,ts->getcol,size);
	ggiUnpackPixels(vis,ts->reference,ts->refcol,size);
	for(i=0;i<size;i++) {
		if (ts->getcol[i].r!=ts->refcol[i].r ||
		    ts->getcol[i].g!=ts->refcol[i].g ||
		    ts->getcol[i].b!=ts->refcol[i].b )
		    	ts->changed_pixels[i]=1;
	}
}

static int checkhlineclip(ggi_visual_t vis,ggi_mode *mode,int x,int y,int width) 
{
	
	/* The tests work like this:
	 * Allocate a large enough buffer for the Get (getpix) and fill 
	 * it as well as the reference buffer with random color values.
	 * Then do the get, map back both buffers to colors and compare
	 * which pixels changed.
	 * This of course only yields probabilistic results, so the test 
	 * is repeated several times and all pixels that changed in _any_
	 * test run are noted.
	 * The result is then checked against the expected pattern. I.e.
	 * There should be no changes in the to-be-clipped areas.
	 */

	int i,j,rc,rc2;
	struct teststruct ts;

	allocate_teststruct(&ts,width);
	for(i=0;i<REPEATS;i++) {
		randomize_teststruct(vis,&ts,width);
		ggiGetHLine(vis,x,y,width,ts.getpix);
		check_teststruct(vis,&ts,width);
	}
	if (verbose) {
		fprintf(stderr,"HLine(%d,%d,%d) at mode.virt.x=%d\nChanges: ",x,y,width,mode->virt.x);
		for(i=0;i<width;i++) {
			fprintf(stderr,"%d",ts.changed_pixels[i]);
		}
		fprintf(stderr,"\n");
	}
	rc=rc2=0;
	for(i=x,j=0;i<0&&j<width;i++,j++) {
		if (ts.changed_pixels[j]) {
			rc++;
		}
	}
	if (rc) {
		if (verbose) fprintf(stderr,"%d pixels changed left of screen.\n",rc);
		rc2+=rc;
	}

	for(rc=0;i<mode->virt.x&&j<width;i++,j++) {
		if (ts.changed_pixels[j]==0) {
			rc++;
		}
	}
	if (rc) {
		if (verbose) fprintf(stderr,"%d pixels unchanged inside screen.\n",rc);
		rc2+=rc;
	}

	for(rc=0;j<width;i++,j++) {
		if (ts.changed_pixels[j]) {
			rc++;
		}
	}
	if (rc) {
		if (verbose) fprintf(stderr,"%d pixels changed right of screen.\n",rc);
		rc2+=rc;
	}

	free_teststruct(&ts);
	return rc2;
}

static void testcase1(const char *desc)
{
	int err;
	ggi_visual_t vis;
	ggi_mode mode;

	printteststart(__FILE__, __PRETTY_FUNCTION__, EXPECTED2PASS, desc);
	if (dontrun) return;

	err = giiInit();
	printassert(err >= 0, "giiInit failed with %i\n", err);

	err = ggiInit();
	printassert(err >= 0, "ggiInit failed with %i\n", err);

	vis = ggNewStem(libgii, libggi, NULL);
	printassert(vis != NULL, "ggNewStem failed\n");

	err = ggiOpen(vis, NULL);
	printassert(err == GGI_OK, "ggiOpen() failed with %i\n", err);

	ggiSetFlags(vis, GGIFLAG_ASYNC);

	/* Get the default mode */
	err = ggiCheckGraphMode (vis, GGI_AUTO, GGI_AUTO, GGI_AUTO, GGI_AUTO,
				GT_AUTO, &mode);
	if (err != GGI_OK) {
		printfailure("ggiCheckGraphMode: No graphic mode available\n");
		ggDelStem(vis);
		return;
	}

	err = ggiSetMode(vis, &mode);
	if (err) {
		printfailure("ggiSetMode() failed although ggiCheckGraphMode() was OK!\n");
		ggDelStem(vis);
		return;
	}

	/* Set up some palette so that palettized modes have a chance to work. */
	ggiSetColorfulPalette(vis);

	err=0;
	/* Check a line that is fully gettable */
	err|=checkhlineclip(vis,&mode, mode.virt.x/4,  mode.virt.y/2,mode.virt.x/2);
	/* Check a line that is fully gettable, full width */
	err|=checkhlineclip(vis,&mode, 0,              mode.virt.y/2,mode.virt.x);
	/* Check a line that is partially left of gettable area */
	err|=checkhlineclip(vis,&mode,-mode.virt.x/4,  mode.virt.y/2,mode.virt.x/2);
	/* Check a line that is partially right of gettable area */
	err|=checkhlineclip(vis,&mode, mode.virt.x*3/4,mode.virt.y/2,mode.virt.x/2);
	/* Check a line that is totally left of gettable area */
	err|=checkhlineclip(vis,&mode,-mode.virt.x/2,  mode.virt.y/2,mode.virt.x/2);
	/* Check a line that is totally right of gettable area */
	err|=checkhlineclip(vis,&mode, mode.virt.x,    mode.virt.y/2,mode.virt.x/2);
	/* Check a line that crosses gettable area */
	err|=checkhlineclip(vis,&mode,-mode.virt.x/2,  mode.virt.y/2,mode.virt.x*2);

	ggDelStem(vis);

	if (err) printfailure("One of the HLine tests returned unexpected results.");
	else printsuccess();
	return;
}


static void testcase2(const char *desc)
{
}


static void testcase3(const char *desc)
{
}


static void testcase4(const char *desc)
{
}


static void testcase5(const char *desc)
{
}

int main(int argc, char * const argv[])
{
	parseopts(argc, argv);
	printdesc("Regression testsuite get handling clipping\n\n");

	testcase1("Check that gets fill full buffer on noclip.");
	testcase2("Check clipping at left/right in middle");
	testcase3("Check clipping at top/bottom in middle");
	testcase4("Check fully outside clipping");
	testcase5("Check box clipping");

	printsummary();

	return 0;
}
