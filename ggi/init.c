/* $Id: init.c,v 1.64 2007/02/26 01:04:36 pekberg Exp $
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
void                 *_ggi_global_lock = NULL;

/* The libggi API */
static int _ggiInit(struct gg_api*);
static struct gg_api _libggi = GG_API("ggi", GG_VERSION(3,0,0,0), _ggiInit);
struct gg_api * libggi = &_libggi;

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


#ifdef PIC
/* Dynamic version */
#define CONF_OFFSET (0)
#define CONF_SIZE (strlen(confdir) + 1 + strlen(GGICONFFILE) + 1)
#define CONF_FORMAT "%s/%s"
#else /* PIC */
/* Static version, with a config stub to disable dynamic modules */
#define CONF_STUB ".set ignore-dynamic-modules\n.require "
#define CONF_OFFSET (40)
#define CONF_SIZE (CONF_OFFSET + strlen(CONF_STUB) + \
		   strlen(confdir) + 1 + strlen(GGICONFFILE) + 2)
#define CONF_FORMAT CONF_STUB "%s/%s\n"
#endif /* PIC */


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


static int
_ggiAttach(struct gg_api* api, struct gg_stem *stem)
{
	return GGI_OK;
}

static void
_ggiDetach(struct gg_api* api, struct gg_stem *stem)
{
	if (GGI_PRIV(stem))
		ggiClose(stem);
}

/*
 * Initialize the structures for the library
 */

int ggiInit(void)
{
	return ggInitAPI(libggi);
}

static void _ggiExit(struct gg_api*);


static int
_ggiInit(struct gg_api* api)
{
	int err;
	const char *str, *confdir;
	char *conffile;
	
	api->ops.exit   = _ggiExit;
	api->ops.attach = _ggiAttach;
	api->ops.detach = _ggiDetach;
/*
	api->ops.open   = _ggiOpenModule;
	api->ops.close  = _ggiCloseModule;
	api->ops.env       = _ggiGetEnv;
	api->ops.publisher = _ggiGetPublisher;
	api->ops.dump      = _ggiDump;
*/

	err = _ggiSwarInit();
	if (err) return err;

	err = ggiExtensionInit();
	if (err) {
		fprintf(stderr, "LibGGI: unable to initialize extension manager\n");
		goto err0;
	}

	_ggiLibIsUp = 1;

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

	_ggiVisuals.mutex = ggLockCreate();
	if (_ggiVisuals.mutex == NULL) {
		fprintf(stderr, "LibGGI: unable to initialize core mutex.\n");
		err = GGI_EUNKNOWN;
		goto err1;
	}
	_ggi_global_lock = ggLockCreate();
	if (_ggi_global_lock == NULL) {
		fprintf(stderr,"LibGGI: unable to initialize global mutex.\n");
		err = GGI_EUNKNOWN;
		goto err2;
	}


#ifdef HAVE_CONFFILE
	confdir = ggiGetConfDir();
	/* two extra bytes needed. One for the slash and one for the terminator (\0) */
	conffile = malloc(CONF_SIZE);
	if (!conffile) {
		fprintf(stderr, "LibGGI: unable to allocate memory for config filename.\n");
		err = GGI_ENOMEM;
		goto err3;
	}

#ifndef PIC
	snprintf(conffile, CONF_OFFSET, "string@%p", conffile + CONF_OFFSET);
#endif
	snprintf(conffile + CONF_OFFSET, CONF_SIZE - CONF_OFFSET,
		CONF_FORMAT, confdir, GGICONFFILE);

	err = ggLoadConfig(conffile, &_ggiConfigHandle);
	if (err != GGI_OK)
		fprintf(stderr,"LibGGI: couldn't open %s.\n", conffile);

	free(conffile);
#else /* HAVE_CONFFILE */
	{
		char arrayconf[40];
		snprintf(arrayconf, sizeof(arrayconf),
			"array@%p", (const void *)_ggibuiltinconf);
		err = ggLoadConfig(arrayconf, &_ggiConfigHandle);
		if (err != GGI_OK) {
			fprintf(stderr, "LibGGI: fatal error - "
					"could not load builtin config\n");
			goto err3;
		}
	}
#endif /* HAVE_CONFFILE */
	if (err == GGI_OK) {
		_ggiInitBuiltins();
		DPRINT_CORE("ggiInit() successful\n");
		return GGI_OK;
	}

err3:
	ggLockDestroy(_ggi_global_lock);
err2:
	ggLockDestroy(_ggiVisuals.mutex);
err1:
	_ggiLibIsUp = 0;
	ggiExtensionExit();
err0:
	return err;
}

int ggiExit(void)
{
	return ggExitAPI(libggi);
}

void _ggiExit(struct gg_api *api)
{

	DPRINT_CORE("ggiExit called\n");

	DPRINT_CORE("ggiExit: really destroying.\n");
	while (!GG_SLIST_EMPTY(&_ggiVisuals.visual)) {
		ggiClose(GG_SLIST_FIRST(&_ggiVisuals.visual)->stem);
	}

	ggLockDestroy(_ggiVisuals.mutex);
	ggLockDestroy(_ggi_global_lock);

	ggiExtensionExit();

	_ggiExitBuiltins();

	ggFreeConfig(_ggiConfigHandle);
	_ggiLibIsUp = 0;

	/* Reset global variables to initialization value.
	 * Otherwise there's a memory corruption when libggi
	 * is re-initialized within an application. */
	_ggiConfigHandle = NULL;
	_ggi_global_lock = NULL;

	DPRINT_CORE("ggiExit: done!\n");
}


int ggiAttach(struct gg_stem *s)
{
	return ggAttach(libggi, s);
}

int ggiDetach(struct gg_stem *s)
{
	return ggDetach(libggi, s);
}


/* Opens a visual.
 */
int ggiOpen(ggi_visual_t stem, const char *driver,...)
{
	va_list drivers;
	int  success;
	void *argptr = NULL;
	struct gg_target_iter match;
	struct ggi_visual *vis;

	DPRINT_CORE("ggiOpen(\"%s\") called\n", driver);

	if (GGI_PRIV(stem))
		/* anther visual is already opened on this stem */
		return GGI_EBUSY;

	if((vis = _ggiNewVisual()) == NULL) {
		return GGI_ENOMEM;
	}
	
	vis->stem = stem;
	GGI_PRIV(stem) = vis;
	
	if (driver == NULL) {
		/* If GGI_DISPLAY is set, use it. Fall back to "auto" 
		 * otherwise.
		 */
		if((driver = getenv("GGI_DISPLAY")) == NULL)
			driver = "display-auto";
		
	} else {
		va_start(drivers, driver);
		argptr = va_arg(drivers, void *);
		va_end(drivers);
	}
	
	DPRINT_CORE("Loading driver %s\n", driver);
	
	success = 0;

	match.input  = driver;
	match.config = _ggiConfigHandle;
	ggConfigIterTarget(&match);
	GG_ITER_FOREACH(&match) {
		DPRINT_CORE("Trying %s with options \"%s\"\n", match.target, match.options);
		if (_ggiOpenDL(vis, _ggiConfigHandle,
			       match.target,match.options,argptr) == 0) {
			success = 1;
			break;
		}
	}
	GG_ITER_DONE(&match);

	if (!success) {
		_ggiDestroyVisual(vis);
		GGI_PRIV(stem) = NULL;
		DPRINT_CORE("ggiOpen: failure\n");
		return GGI_ENOTFOUND;
	}

	ggLock(_ggiVisuals.mutex);
	GG_SLIST_INSERT_HEAD(&_ggiVisuals.visual, vis, vislist);
	_ggiVisuals.visuals++;
	ggUnlock(_ggiVisuals.mutex);

	ggBroadcast(libggi->channel, GGI_OBSERVE_VISUAL_OPENED, vis);

	DPRINT_CORE("ggiOpen: success\n");

	return GGI_OK;
}
	
/* ggiClose
 *	Closes the requested visual
 *      Returns 0 on success, < 0 on error
 */
int ggiClose(ggi_visual_t v)
{
	struct ggi_visual *vis,*visual,*pvis=NULL;
	
	visual = GGI_VISUAL(v);
	
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

	ggBroadcast(libggi->channel, GGI_OBSERVE_VISUAL_CLOSED, vis);

	ggLock(_ggiVisuals.mutex);

	if (pvis == NULL) {
		GG_SLIST_FIRST(&_ggiVisuals.visual) = GG_SLIST_NEXT(vis, vislist);
	} else {
		GG_SLIST_NEXT(pvis, vislist) = GG_SLIST_NEXT(vis, vislist);
	}

	_ggiVisuals.visuals--;
	
	ggUnlock(_ggiVisuals.mutex);

	vis->stem = NULL;
	_ggiDestroyVisual(vis);
	GGI_PRIV(v) = NULL;

	DPRINT_CORE("ggiClose: done!\n");

	return 0;
}
