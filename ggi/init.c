/* $Id: init.c,v 1.48 2005/12/22 13:16:18 pekberg Exp $
******************************************************************************

   LibGGI initialization.

   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
  
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
#include <ggi/internal/ggi_debug.h>
#include <ggi/gg.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */

#include "ext.h"
#include "swar.h"

#include <string.h>
#include <stdarg.h>


/* Exported variables */
uint32_t             _ggiDebug         = 0;
void                 *_ggi_global_lock = NULL;

/* Global variables */
void                 *_ggiConfigHandle = NULL;

/* Static variables */
static struct {
	void		*mutex;		/* Lock when changing.. */
	int		 visuals;	/* # of visuals active */
	GG_SLIST_HEAD(visual, ggi_visual) visual; /* Linked list of all visuals */
} _ggiVisuals;	/* This is for remembering visuals. */

static int            _ggiLibIsUp      = 0;
#ifdef HAVE_CONFFILE
static char           ggiconfstub[512] = GGICONFDIR;
static char          *ggiconfdir       = ggiconfstub + GGITAGLEN;
#else
extern const char const *_ggibuiltinconf[];
#endif

void _ggiInitBuiltins(void);
void _ggiExitBuiltins(void);



/* 
 * Returns the handle of the config file
 */


const void *_ggiGetConfigHandle(void)
{
	return _ggiConfigHandle;
}


/* 
 * Returns the directory where global config files are kept
 */


const char *ggiGetConfDir(void)
{
#ifdef HAVE_CONFFILE
#if defined(__WIN32__) && !defined(__CYGWIN__)
	/* On Win32 we allow overriding of the compiled in path. */
	const char *envdir = getenv("GGI_CONFDIR");
	if (envdir) return envdir;
#endif
	return ggiconfdir;
#else /* HAVE_CONFFILE */
	return NULL;
#endif /* HAVE_CONFFILE */
}

/*
 * Initialize the structures for the library
 */

int ggiInit(void)
{
	int err;
	const char *str, *confdir;
	char *conffile;

	_ggiLibIsUp++;
	if (_ggiLibIsUp > 1) return 0;	/* Initialize only at first call. */

	err = _ggiSwarInit();
	if (err) return err;

	err = ggiExtensionInit();
	if (err) {
		fprintf(stderr, "LibGGI: unable to initialize extension manager\n");
		goto err0;
	}

	_ggiVisuals.visuals = 0;
	GG_SLIST_INIT(&_ggiVisuals.visual);

	str = getenv("GGI_DEBUGSYNC");
	if (str != NULL) {
		_ggiDebug |= DEBUG_SYNC;
	}

	str = getenv("GGI_DEBUG");
	if (str != NULL) {
		_ggiDebug |= atoi(str) & DEBUG_ALL;
		DPRINT_CORE("%s Debugging=%d\n",
				DEBUG_ISSYNC ? "sync" : "async",
				_ggiDebug);
	}

	str = getenv("GGI_DEFMODE");
	if (str != NULL) {
		_ggiSetDefaultMode(str);
	}

	err = giiInit();
	if (err) {
		fprintf(stderr, "LibGGI: unable to initialize LibGII\n");
		goto err1;
	}

	_ggiVisuals.mutex = ggLockCreate();
	if (_ggiVisuals.mutex == NULL) {
		fprintf(stderr, "LibGGI: unable to initialize core mutex.\n");
		err = GGI_EUNKNOWN;
		goto err2;
	}
	_ggi_global_lock = ggLockCreate();
	if (_ggi_global_lock == NULL) {
		fprintf(stderr,"LibGGI: unable to initialize global mutex.\n");
		err = GGI_EUNKNOWN;
		goto err3;
	}


#ifdef HAVE_CONFFILE
	confdir = ggiGetConfDir();
	/* two extra bytes needed. One for the slash and one for the terminator (\0) */
	conffile = malloc(strlen(confdir) + 1 + strlen(GGICONFFILE)+1);
	if (!conffile) {
		fprintf(stderr, "LibGGI: unable to allocate memory for config filename.\n");
		err = GGI_ENOMEM;
		goto err4;
	}

	/* Note, sprintf() is safe here since conffile is dynamically
	 * allocated
	 */
	sprintf(conffile, "%s/%s", confdir, GGICONFFILE);

	err = ggLoadConfig(conffile, &_ggiConfigHandle);
	free(conffile);
#else
	{
		char arrayconf[40];
		snprintf(arrayconf, 40, "array@%p", _ggibuiltinconf);
		err = ggLoadConfig(arrayconf, &_ggiConfigHandle);
		if (err != GGI_OK) {
			fprintf(stderr, "LibGGI: fatal error - "
					"could not load builtin config\n");
			goto err4;
		}
	}
#endif /* HAVE_CONFFILE */
	if (err == GGI_OK) {
		_ggiInitBuiltins();
		DPRINT_CORE("ggiInit() successfull\n");
		return GGI_OK;
	}
	fprintf(stderr,"LibGGI: couldn't open %s.\n", conffile);


err4:
	ggLockDestroy(_ggi_global_lock);
err3:
	ggLockDestroy(_ggiVisuals.mutex);
err2:
	giiExit();
	_ggiLibIsUp--;
err1:
	ggiExtensionExit();
err0:
	return err;
}

int ggiExit(void)
{
	DPRINT_CORE("ggiExit called\n");
	if (!_ggiLibIsUp) return GGI_ENOTALLOC;

	if (_ggiLibIsUp > 1) {
		_ggiLibIsUp--;
		return _ggiLibIsUp;
	}

	DPRINT_CORE("ggiExit: really destroying.\n");
	while (!GG_SLIST_EMPTY(&_ggiVisuals.visual)) {
		ggiClose(GG_SLIST_FIRST(&_ggiVisuals.visual));
	}

	ggLockDestroy(_ggiVisuals.mutex);
	ggLockDestroy(_ggi_global_lock);

	ggiExtensionExit();

	_ggiExitBuiltins();

	ggFreeConfig(_ggiConfigHandle);
	giiExit();
	_ggiLibIsUp = 0;

	/* Reset global variables to initialization value.
	 * Otherwise there's a memory corruption when libggi
	 * is re-initialized within an application. */
	_ggiConfigHandle = NULL;
	_ggi_global_lock = NULL;

	DPRINT_CORE("ggiExit: done!\n");
	return 0;
}

void ggiPanic(const char *format,...)
{
	va_list ap;

	DPRINT_CORE("ggiPanic called\n");

	va_start(ap,format);
	vfprintf(stderr,format,ap);
	fflush(stderr);
	va_end(ap);

	while(ggiExit()>0);	/* kill all instances ! */
	exit(1);
}

/* Make sure str contains a valid variable name. Kill everything but
 * [a-zA-Z0-9].
 */
static void mangle_variable(char *str)
{
	for(;*str;str++) {
		/**/ if ( ( *str>='A' && *str<='Z' ) ||
			  ( *str>='0' && *str<='9' ) ) continue;
		else if (   *str>='a' && *str<='z' ) *str+='A'-'a';
		else *str='_';
	}
}

/* Opens a visual.
 */
ggi_visual *ggiOpen(const char *driver,...)
{
#define MAX_TARGET_LEN	1024

	va_list drivers;
	ggi_visual *vis;
	char *cp, *inplist;
	char str[MAX_TARGET_LEN];
	char target[MAX_TARGET_LEN];
	int  success=0;
	void *argptr;
	static int globalopencount=0;
	struct gg_target_iter match;
	
	if (!_ggiLibIsUp) return NULL;

	DPRINT_CORE("ggiOpen(\"%s\") called\n", driver);

	if (driver == NULL) {
		void *ret;

		/* If GGI_DISPLAY is set, use it. Fall back to "auto" 
		 * otherwise.
		 */

		cp=getenv("GGI_DISPLAY");
		if (cp != NULL) {
			ret = ggiOpen(cp,NULL);
			return ret;
		}
		driver = "auto";
	}
	if (strcmp(driver,"auto") == 0) {

		void *ret;

		ggDPrintf(1, "LibGGI", "No explicit target specified.\n");

		ret = _ggiProbeTarget();
		return ret;
	}

	if ((vis = _ggiNewVisual()) == NULL) {
		return NULL;
	}

	va_start(drivers, driver);

	argptr = va_arg(drivers, void *);
	va_end(drivers);

	DPRINT_CORE("Loading driver %s\n",driver);

	success = 0;

	match.input  = driver;
	match.config = _ggiConfigHandle;
	ggConfigIterTarget(&match);
	GG_ITER_FOREACH(&match) {
		if (_ggiOpenDL(vis, _ggiConfigHandle,
			match.target,match.options,argptr) == 0)
		{
			success = 1;
			break;
		}
	}
	GG_ITER_DONE(&match);

	if (success) {
		ggLock(_ggiVisuals.mutex);
		GG_SLIST_INSERT_HEAD(&_ggiVisuals.visual, vis, vislist);
		_ggiVisuals.visuals++;
		ggUnlock(_ggiVisuals.mutex);
		DPRINT_CORE("ggiOpen: returning %p\n", vis);
	} else {
		_ggiDestroyVisual(vis);
		DPRINT_CORE("ggiOpen: failure\n");
		return NULL;
	}

	DPRINT_CORE("Loading extra inputs/filters for %s\n",driver);

	inplist=NULL;

	snprintf(str, MAX_TARGET_LEN, "GGI_INPUT_%s_%d", target, ++globalopencount);
	mangle_variable(str);
	if (!inplist) { 
		inplist = getenv(str);
		DPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
	}

	snprintf(str, MAX_TARGET_LEN, "GGI_INPUT_%s", target);
	mangle_variable(str);
	if (!inplist) {
		inplist = getenv(str);
		DPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
	}

	strcpy(str,"GGI_INPUT");
	if (!inplist) {
		inplist = getenv(str);
		DPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
	}

	if (inplist) {
		gii_input *inp = giiOpen(inplist, NULL);

		if (inp == NULL) {
			fprintf(stderr, "LibGGI: failed to load input: %s\n",
				inplist);
		} else {
			vis->input = giiJoinInputs(vis->input, inp);
		}
	}

	if (vis->input == NULL) {
		/* Add dummy input source so we can use sendevent */
		vis->input = giiOpen("null", NULL);
		if (vis->input == NULL) {
			/* Something is wrong here - bail out */
			DPRINT_CORE("Cannot open input-null\n");
			ggiClose(vis);
			return NULL;
		}
	}

	return vis;
#undef MAX_TARGET_LEN
}
	
/* ggiClose
 *	Closes the requested visual
 *      Returns 0 on success, < 0 on error
 */
int ggiClose(ggi_visual *visual)
{
	ggi_visual *vis,*pvis=NULL;

	DPRINT_CORE("ggiClose(\"%p\") called\n", visual);

	if (!_ggiLibIsUp) return GGI_ENOTALLOC;

	DPRINT_CORE("ggiClose: closing\n");

	vis = GG_SLIST_FIRST(&_ggiVisuals.visual);
	while (vis != NULL) {
		if (vis == visual) break;
		pvis = vis;
		vis = GG_SLIST_NEXT(vis, vislist);
	}

	if (vis == NULL) return GGI_EARGINVAL;

	ggLock(_ggiVisuals.mutex);

	if (pvis == NULL) {
		GG_SLIST_FIRST(&_ggiVisuals.visual) = GG_SLIST_NEXT(vis, vislist);
	} else {
		GG_SLIST_NEXT(pvis, vislist) = GG_SLIST_NEXT(vis, vislist);
	}

	_ggiVisuals.visuals--;
	
	ggUnlock(_ggiVisuals.mutex);

	_ggiDestroyVisual(vis);

	DPRINT_CORE("ggiClose: done!\n");

	return 0;
}

#include <ggi/internal/ggilibinit.h>
