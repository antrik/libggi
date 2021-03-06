/* $Id: init.c,v 1.75 2008/01/21 22:56:53 cegger Exp $
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

#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/gg.h>
#include <ggi/internal/gg_replace.h>	/* for snprintf() */

#include "swar.h"

#include <string.h>
#include <stdarg.h>


/* Exported variables */
void                 *_ggi_global_lock = NULL;

/* The libggi API */
static ggfunc_api_op_init ggi_init;
static ggfunc_api_op_exit ggi_exit;
static ggfunc_api_op_attach ggi_attach;
static ggfunc_api_op_detach ggi_detach;
/*
static ggfunc_api_op_getenv ggi_getenv;
*/
static ggfunc_api_op_plug ggi_plug;
static ggfunc_api_op_unplug ggi_unplug;

static struct gg_api_ops ggi_ops = {
	ggi_init,
	ggi_exit,
	ggi_attach,
	ggi_detach,
	NULL, /* ggi_getenv, */
	ggi_plug,
	ggi_unplug
};

static struct gg_api ggi = GG_API_INIT("ggi", 3, 0, &ggi_ops);

struct gg_api * libggi = &ggi;

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
	return ggInitAPI(libggi);
}


int ggiExit(void)
{
	return ggExitAPI(libggi);
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
		/* another visual is already opened on this stem */
		return GGI_EBUSY;

	if((vis = _ggiNewVisual()) == NULL) {
		return GGI_ENOMEM;
	}
	
	vis->instance.stem = stem;
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
	match.config = ggi.config;
	ggConfigIterTarget(&match);
	GG_ITER_FOREACH(&match) {
		DPRINT_CORE("Trying %s with options \"%s\"\n", match.target, match.options);
		if (_ggiOpenDL(vis, ggi.config,
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

	_ggiDestroyVisual(vis);
	GGI_PRIV(v) = NULL;

	DPRINT_CORE("ggiClose: done!\n");

	return 0;
}



/* API ops */

static int
ggi_init(struct gg_api* api)
{
	int err;
	const char *str, *confdir;
	char *conffile;

/*
	api->ops.open   = _ggiOpenModule;
	api->ops.close  = _ggiCloseModule;
	api->ops.env       = _ggiGetEnv;
	api->ops.publisher = _ggiGetPublisher;
	api->ops.dump      = _ggiDump;
*/

	err = _ggiSwarInit();
	if (err) return err;

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
		goto err0;
	}
	_ggi_global_lock = ggLockCreate();
	if (_ggi_global_lock == NULL) {
		fprintf(stderr,"LibGGI: unable to initialize global mutex.\n");
		err = GGI_EUNKNOWN;
		goto err1;
	}


#ifdef HAVE_CONFFILE
	confdir = ggiGetConfDir();
	/* two extra bytes needed. One for the slash and one for the terminator (\0) */
	conffile = malloc(CONF_SIZE);
	if (!conffile) {
		fprintf(stderr, "LibGGI: unable to allocate memory for config filename.\n");
		err = GGI_ENOMEM;
		goto err2;
	}

#ifndef PIC
	snprintf(conffile, CONF_OFFSET, "string@%p", conffile + CONF_OFFSET);
#endif
	snprintf(conffile + CONF_OFFSET, CONF_SIZE - CONF_OFFSET,
		CONF_FORMAT, confdir, GGICONFFILE);

	err = ggLoadConfig(conffile, &ggi.config);
	if (err != GGI_OK)
		fprintf(stderr,"LibGGI: couldn't open %s.\n", conffile);

	free(conffile);
#else /* HAVE_CONFFILE */
	{
		char arrayconf[40];
		snprintf(arrayconf, sizeof(arrayconf),
			"array@%p", (const void *)_ggibuiltinconf);
		err = ggLoadConfig(arrayconf, &ggi.config);
		if (err != GGI_OK) {
			fprintf(stderr, "LibGGI: fatal error - "
					"could not load builtin config\n");
			goto err2;
		}
	}
#endif /* HAVE_CONFFILE */
	if (err == GGI_OK) {
		_ggiInitBuiltins();
		DPRINT_CORE("ggiInit() successful\n");
		return GGI_OK;
	}

err2:
	ggLockDestroy(_ggi_global_lock);
err1:
	ggLockDestroy(_ggiVisuals.mutex);
err0:
	_ggiLibIsUp = 0;
	return err;
}


static void
ggi_exit(struct gg_api *api)
{

	DPRINT_CORE("ggiExit called\n");

	DPRINT_CORE("ggiExit: really destroying.\n");
	while (!GG_SLIST_EMPTY(&_ggiVisuals.visual)) {
		ggiClose(GG_SLIST_FIRST(&_ggiVisuals.visual)->instance.stem);
	}

	ggLockDestroy(_ggiVisuals.mutex);
	ggLockDestroy(_ggi_global_lock);

	_ggiExitBuiltins();

	ggFreeConfig(ggi.config);
	_ggiLibIsUp = 0;

	/* Reset global variables to initialization value.
	 * Otherwise there's a memory corruption when libggi
	 * is re-initialized within an application. */
	ggi.config = NULL;
	_ggi_global_lock = NULL;

	DPRINT_CORE("ggiExit: done!\n");
}


static int
ggi_attach(struct gg_api* api, struct gg_stem *stem)
{
	return GGI_OK;
}

static void
ggi_detach(struct gg_api* api, struct gg_stem *stem)
{
	if (GGI_PRIV(stem))
		ggiClose(stem);
}

int _ggiOpenModule(struct gg_api *api, struct gg_module *_module,
			struct gg_stem *stem, const char *argstr,
			void *argptr,
			struct gg_instance **res);
int _ggiCloseModule(struct gg_api *api, struct gg_instance *instance);


static int
ggi_plug(struct gg_api * api,
	 struct gg_module *_module,
	 struct gg_stem *stem,
	 const char * argstr,
	 void *argptr,
	 struct gg_instance **res)
{
	return _ggiOpenModule(api, _module, stem, argstr, argptr, res);
}

static int
ggi_unplug(struct gg_api * api, struct gg_instance *instance)
{
	return _ggiCloseModule(api, instance);
}
