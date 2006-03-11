/* $Id: ext.c,v 1.7 2006/03/11 18:49:12 soyt Exp $
******************************************************************************

   LibGGI extension support.

   Copyright (C) 1997 Jason McMullan		[jmcc@ggi-project.org]
   Copyright (C) 1998-2000 Marcus Sundberg	[marcus@ggi-project.org]
   Copyright (C) 2005 Christoph Egger

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

#include "ext.h"
#include <string.h>


static int numextensions = 0;

static GG_TAILQ_HEAD(_ggiExtension, ggi_extension) _ggiExtension
		= GG_TAILQ_HEAD_INITIALIZER(_ggiExtension);


/* extension macros
 */
#define FOREACH_EXTENSION(n)	GG_TAILQ_FOREACH(n, &_ggiExtension, extlist)
#define ADD_EXTENSION(n)	GG_TAILQ_INSERT_TAIL(&_ggiExtension, n, extlist)
#define REMOVE_EXTENSION(n)	GG_TAILQ_REMOVE(&_ggiExtension, n, extlist)
#define HAVE_NO_EXTENSIONS	GG_TAILQ_EMPTY(&_ggiExtension)
#define HAVE_EXTENSIONS		(!GG_TAILQ_EMPTY(&_ggiExtension))


int ggiExtensionInit(void)
{
	return 0;
}

int ggiExtensionExit(void)
{
	ggi_extension *tmp;

	while((tmp = GG_TAILQ_FIRST(&_ggiExtension)) != NULL) {
		REMOVE_EXTENSION(tmp);
		free(tmp);
	}

	LIB_ASSERT(HAVE_NO_EXTENSIONS, "ggi extension list not empty at shutdown\n");

	return 0;
}

/*
  Register an Extension for usage. It will return an extension ID 
  (ggi_extid) that can be used to address this extension.
*/
ggi_extid
ggiExtensionRegister(const char *name, size_t size,
			int (*change)(struct ggi_visual *, int))
{
	ggi_extension *tmp, *ext;

	DPRINT_CORE("ggiExtensionRegister(\"%s\", %d, %p) called\n",
			name, size, change);
	if (HAVE_EXTENSIONS) {
		FOREACH_EXTENSION(tmp) {
			if (strcmp(tmp->name, name) == 0) {
				tmp->initcount++;
				DPRINT_CORE("ggiExtensionRegister: accepting copy #%d of extension %s\n",
					       tmp->initcount,tmp->name);
				return tmp->id;
			}
		}
	}

	ext = malloc(sizeof(ggi_extension));
	if (ext == NULL) return GGI_ENOMEM;

	ext->size = size;
	ext->paramchange = change;
	GG_TAILQ_NEXT(ext, extlist) = NULL;
	ext->initcount = 1;
	ggstrlcpy(ext->name, name, sizeof(ext->name));

	ADD_EXTENSION(ext);

	DPRINT_CORE("ggiExtensionRegister: installing first copy of extension %s\n", name);

	ext->id = numextensions;
	numextensions++;

	return ext->id;
}


/*
  Unregister an Extension. It takes the ggi_extid gotten from
  ggiExtensionRegister. It disallows further calls to ggiExtensionAttach
  (for that extid) and frees memory for the extension registry. 
  It dos NOT automatically ggiExtensionDetach it from all visuals.
*/
int ggiExtensionUnregister(ggi_extid id)
{
	ggi_extension *tmp;

	DPRINT_CORE("ggiExtensionUnregister(%d) called\n", id);
	if (HAVE_NO_EXTENSIONS) return GGI_ENOTALLOC;

	FOREACH_EXTENSION(tmp) {
		if (tmp->id != id) continue;
		if (--tmp->initcount) {
			DPRINT_CORE("ggiExtensionUnregister: removing #%d copy of extension %s\n", tmp->initcount+1, tmp->name);
			/* Removed copy */
			return 0;	
		}

		REMOVE_EXTENSION(tmp);

		DPRINT_CORE("ggiExtensionUnregister: removing last copy of extension %s\n",
				tmp->name);

		free(tmp);

		return 0;
	}

	return GGI_ENOTALLOC;
}

/*
  Make an extension available for a given visual.
  The extension has to be registered for that.
  RC: negative  Error.
       x	for the number of times this extension had already been
        	registered to that visual. So
       0	means you installed the extension as the first one. Note that
     >=0	should be regarded as "success". It is legal to attach an
        	extension multiple times. You might want to set up private
        	data if RC==0.
*/
int ggiExtensionAttach(ggi_visual_t v, ggi_extid id)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	ggi_extension *tmp = NULL;

	DPRINT_CORE("ggiExtensionAttach(%p, %d) called\n", vis, id);

	if (HAVE_EXTENSIONS) {
		FOREACH_EXTENSION(tmp) {
			if (tmp->id == id) break;
		}
	}
	if (! tmp) return GGI_EARGINVAL;

	if (vis->numknownext <= id) {
		ggi_extlist *newlist;
		int extsize = sizeof(*vis->extlist);

		newlist = realloc(vis->extlist, extsize * (id + 1U));
		if (newlist == NULL) return GGI_ENOMEM;

		vis->extlist = newlist;
		memset(&vis->extlist[vis->numknownext], 0,
		       extsize*(id + 1U - vis->numknownext));
		vis->numknownext = id + 1U;
		DPRINT_CORE("ggiExtensionAttach: ExtList now at %p (%d)\n",
			       vis->extlist, vis->numknownext);
	}

	if (LIBGGI_EXTAC(vis, id) == 0) {
		LIBGGI_EXT(vis, id) = malloc(tmp->size);
		if (LIBGGI_EXT(vis, id) == NULL) return GGI_ENOMEM;
	}
	return LIBGGI_EXTAC(vis, id)++;
}

/*
  Destroy an extension for a given visual.
  The extension need not be registered for that anymore, though the extid
  has to be valid. 
  RC: negative	Error.
       x	for the number of times this extension remains attached. So
       0	means you removed the last copy of the extension. Note that
     >=0	should be regarded as "success". It is legal to attach an 
		extension multiple times.
*/
int ggiExtensionDetach(ggi_visual_t v, ggi_extid id)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	DPRINT_CORE("ggiExtensionDetach(%p, %d) called\n", vis, id);

	if (vis->numknownext <= id || LIBGGI_EXTAC(vis, id) == 0) {
		return GGI_EARGINVAL;
	}

	if (--LIBGGI_EXTAC(vis, id)) {
		return LIBGGI_EXTAC(vis, id);
	}

	free(LIBGGI_EXT(vis, id));
	LIBGGI_EXT(vis, id) = NULL;  /* Make sure ... */

	return 0;
}

int ggiIndicateChange(ggi_visual_t v, int whatchanged)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	ggi_extension *tmp = NULL;

	DPRINT_CORE("ggiIndicateChange(%p, 0x%x) called\n",
			vis, whatchanged);

	/* Tell all attached extensions on this visual */
	DPRINT_CORE("ggiIndicateChange: %i changed for %p.\n",
			whatchanged, vis);

	if (HAVE_EXTENSIONS) {
		FOREACH_EXTENSION(tmp) {
			if (tmp->id < vis->numknownext &&
			   LIBGGI_EXTAC(vis, tmp->id))
			{
				tmp->paramchange(vis, whatchanged);
			}
		}
	}

	return 0;
}
