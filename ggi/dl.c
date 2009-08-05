/* $Id: dl.c,v 1.42 2009/08/05 07:59:40 cegger Exp $
******************************************************************************

   Graphics library for GGI. Library extensions dynamic loading.

   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 2000 Marcus Sundberg		[marcus@ggi-project.org]

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
#include <ggi/internal/ggi-module.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>


#define GGI_SYMNAME_MAX		255
#define GGI_SYMNAME_PREFIX	"GGIdl_"


/* Open the dynamic libary requested
 */
static int _ggiLoadDL(const char *filename, const char *symprefix,
		      int type, ggi_dlhandle **dlh,
		      const char *realsym)
{
	ggi_dlhandle hand;
	char symname[GGI_SYMNAME_MAX+1], *extptr;
	const char *nameptr;

	DPRINT_LIBS("_ggiLoadDL(\"%s\", 0x%x) called \n", filename, type);

	hand.name = NULL;
	hand.usecnt = 0;

	hand.handle = ggGetScope(filename);
	DPRINT_LIBS("hand.handle=%p\n", hand.handle);
	if (hand.handle == NULL) {
		DPRINT_LIBS("Error loading module %s\n", filename);
		return GGI_ENOFILE;
	}

	/* HACK if a realsym is given, use that immediatly */
	if(realsym) {
		ggstrlcpy(symname, realsym, GGI_SYMNAME_MAX);
		goto getsymbol;
	}
	nameptr = (const char *)strrchr(filename, '/');
	if (!nameptr) {
		nameptr = filename;
	} else {
		nameptr++;
	}
	
	snprintf(symname, sizeof(symname), "%s%s", symprefix, nameptr);
	extptr = strrchr(symname, '.');
	if (extptr) {
		*extptr = '\0';
	}
getsymbol:
	hand.entry = (ggifunc_dlentry*)ggFromScope(hand.handle, symname);
	DPRINT_LIBS("&(%s) = %p\n", symname, hand.entry);
	if (hand.entry == NULL) {
		ggDelScope(hand.handle);
		return GGI_ENOFUNC;
	}

	/* Get pointers */
	hand.entry(GGIFUNC_open, (void**)&hand.open);
	hand.entry(GGIFUNC_exit, (void**)&hand.exit);
	hand.entry(GGIFUNC_close, (void**)&hand.close);
	DPRINT_LIBS("hand.open = %p\n", hand.open);
	DPRINT_LIBS("hand.exit = %p\n", hand.exit);
	DPRINT_LIBS("hand.close = %p\n", hand.close);

	*dlh = malloc(sizeof(**dlh));
	if (*dlh == NULL) {
		ggDelScope(hand.handle);
		return GGI_ENOMEM;
	}
	memcpy(*dlh, &hand, sizeof(ggi_dlhandle));

	return 0;
}


/* Probe a DL
 */
static int _ggiProbeDL(struct ggi_visual *vis, struct gg_config *conffilehandle,
		const char *api, const char *args, void *argptr,
		int type, ggi_dlhandle **dlh, uint32_t *dlret)
{
	int err;
	struct gg_location_iter match;

	DPRINT_LIBS("_ggiProbeDL(%p, \"%s\", \"%s\", %p, 0x%x) called\n",
			vis, api, args ? args : "(null)", argptr, type);
	
	match.name = api;
	match.config = conffilehandle;
	ggConfigIterLocation(&match);
	err =  GGI_ENOMATCH;
	GG_ITER_FOREACH(&match) {
		err = _ggiLoadDL(match.location,
				GGI_SYMNAME_PREFIX, type, dlh,
				match.symbol);
		if (err == GGI_OK) break;
	}
	GG_ITER_DONE(&match);
	
	if(err) {
		DPRINT_LIBS("LibGGI: could not probe lib for sublib: %s\n",
			    api);
		return err;
	}
	
	dlh[0]->type = type;
	dlh[0]->visual = vis;

	err = dlh[0]->open(vis, *dlh, args, argptr, dlret);
	DPRINT_LIBS("%d = dlh[0]->open(%p, %p, \"%s\", %p, %d) - %s\n",
		       err, vis, *dlh, args ? args : "(null)", argptr, *dlret,
		       api);
	if (err) {
		ggDelScope(dlh[0]->handle);
		free(*dlh);
		*dlh = NULL;
		return err;
	}

	return 0;
}


/****** Open and Close a DL *********/
static int
_ggiAddDL(struct ggi_visual *vis, struct gg_config *conffilehandle,
	      const char *api, const char *args, void *argptr,
	      int type)
{
	ggi_dlhandle_l *tmp;
	ggi_dlhandle *dlh = NULL;
	uint32_t dlret = 0;
	int err;

	DPRINT_LIBS("_ggiAddDL(%p, \"%s\", \"%s\", 0x%x) called\n",
		       vis, api, args ? args : "(null)", type);

	err = _ggiProbeDL(vis, conffilehandle, api,
			args, argptr, type, &dlh, &dlret);
	if (err) return err;

	if (type == GGI_DLTYPE_INTERNAL) {
		if (dlret & GGI_DL_OPDISPLAY) {
			tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;

			GG_SLIST_INSERT_HEAD(&vis->opdisplay->head.dlhandle,
					tmp, dllist);
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPCOLOR) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;

			GG_SLIST_INSERT_HEAD(&vis->opcolor->head.dlhandle,
					tmp, dllist);
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPDRAW) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;

			GG_SLIST_INSERT_HEAD(&vis->opdraw->head.dlhandle,
					tmp, dllist);
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPGC) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;

			GG_SLIST_INSERT_HEAD(&vis->opgc->head.dlhandle,
					tmp, dllist);
			dlh->usecnt++;
		}
	} else {
		dlh->usecnt = 1;
	}

	if (dlh->usecnt == 0) {
		fprintf(stderr,
			"LibGGI: %s (%s) -> 0x%.8"PRIx32" - no operations in this library\n",
			api, args ? args : "(null)", dlret);
		ggDelScope(dlh->handle);
		free(dlh);
		return GGI_ENOFUNC;
	} else {
		tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
		tmp->handle = dlh;

		GG_SLIST_INSERT_HEAD(&LIBGGI_DLHANDLE(vis), tmp, dllist);
	}

	dlh->name = strdup(api);

	return 0;
}

int _ggiOpenDL(struct ggi_visual *vis, struct gg_config *conffilehandle,
		const char *api, const char *args, void *argptr)
{
	return _ggiAddDL(vis, conffilehandle,
			api, args, argptr, GGI_DLTYPE_INTERNAL);
}

void _ggiExitDL(struct ggi_visual *vis, ggi_dlhandle_l *lib)
{
	for (; lib != NULL; lib = GG_SLIST_NEXT(lib, dllist)) {
		if (lib->handle->exit) {
			lib->handle->exit(vis, lib->handle);
		}
	}
}

static void _ggiRemoveDL(struct ggi_visual *vis, ggi_dlhandle_l **lib)
{
	ggi_dlhandle_l *tmp, **prev;
	ggi_dlhandle_l *libtmp, **libprev, *libnext;

	for (libprev = lib, libtmp = *lib; libtmp != NULL; libtmp=libnext) {
		libnext = GG_SLIST_NEXT(libtmp, dllist);

		if (libtmp->handle->usecnt <= 0) {
			DPRINT_LIBS("Disposing \"%s\"\n",
				       libtmp->handle->name);

			*libprev = GG_SLIST_NEXT(libtmp, dllist);
			if (libtmp->handle->close) {
				libtmp->handle->close(vis, libtmp->handle);
			}
			DPRINT_LIBS("Closing handle: 0x%x\n",
				       libtmp->handle->handle);
			ggDelScope(libtmp->handle->handle);

			/* Now, clean up the master visual */
			prev = &GG_SLIST_FIRST(&LIBGGI_DLHANDLE(vis));
			GG_SLIST_FOREACH(tmp, &LIBGGI_DLHANDLE(vis), dllist) {
				if (tmp->handle == libtmp->handle) break;
				prev = &GG_SLIST_NEXT(tmp, dllist);
			}
			if (!tmp) {
				DPRINT_LIBS("Error: handle not in master list.\n");
			}
			*prev = GG_SLIST_NEXT(tmp, dllist);
			free(tmp);

			free(libtmp->handle->name);
			free(libtmp->handle);
			free(libtmp);
		} else {
			libprev = &GG_SLIST_NEXT(libtmp, dllist);
		}
	}
}

void _ggiZapDL(struct ggi_visual *vis, ggi_dlhandle_l **lib)
{
	ggi_dlhandle_l *tmp, *next;

	DPRINT_LIBS("_ggiZapDL(%p, %p) called\n", vis, lib);

	for (tmp = *lib; tmp; tmp = GG_SLIST_NEXT(tmp, dllist)) {
		tmp->handle->usecnt--;
	}

	_ggiRemoveDL(vis,lib);

	for (tmp = *lib; tmp; tmp = next) {
		next = GG_SLIST_NEXT(tmp, dllist);
		free(tmp);
	}

	*lib = NULL;
}



int _ggiOpenModule(struct gg_api *api, struct gg_module *_module,
			struct gg_stem *stem, const char *argstr,
			void *argptr,
			struct gg_instance **res);
int _ggiCloseModule(struct gg_api *api, struct gg_instance *instance);


int _ggiOpenModule(struct gg_api *api, struct gg_module *_module,
		   struct gg_stem *stem, const char *argstr,
		   void *argptr,
		   struct gg_instance **res)
{
	int err;
	struct ggi_module_helper *mod_helper;
	struct ggi_helper *helper;
	struct ggi_visual *vis;

	LIB_ASSERT(api == libggi, "API mismatch!");
	LIB_ASSERT(_module->klass == GGI_MODULE_HELPER,
		"module is not a helper!");
	LIB_ASSERT(res != NULL, "invalid res");

	helper = calloc(1, sizeof(*helper));
	if (helper == NULL)
		return GGI_ENOMEM;

	vis = GGI_VISUAL(stem);
	mod_helper = (struct ggi_module_helper *)_module;

	helper->plugin.module = _module;
	helper->visual = vis;

	err = GGI_OK;
	if (mod_helper->setup)
		err = mod_helper->setup(helper, argstr, argptr);
	if (err)
		goto err0;

	GG_LIST_INSERT_HEAD(&vis->helpers, helper, h_list);
	*res = &(helper->plugin);

	return GGI_OK;
err0:
	free(helper);
	return err;
}


int _ggiCloseModule(struct gg_api *api, struct gg_instance *instance)
{
	struct ggi_module_helper *mod_helper;
	struct ggi_helper *helper;

	LIB_ASSERT(api == libggi, "api mismatch!");

	helper = (struct ggi_helper *)instance;
	mod_helper = (struct ggi_module_helper *)helper->plugin.module;

	GG_LIST_REMOVE(helper, h_list);

	if (mod_helper->teardown)
		mod_helper->teardown(helper);

	free(helper);
	return GGI_OK;
}
