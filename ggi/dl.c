/* $Id: dl.c,v 1.1 2001/05/12 23:03:14 cegger Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <ggi/internal/internal.h>
#include <ggi/gg.h>


#define GGI_SYMNAME_MAX		255
#define GGI_SYMNAME_PREFIX	"GGIdl_"


/* Open the dynamic libary requested
 */
static int _ggiLoadDL(const char *filename, const char *symprefix,
				int type, ggi_dlhandle **dlh)
{
	ggi_dlhandle hand;
	char symname[GGI_SYMNAME_MAX+1], *nameptr;

	GGIDPRINT_LIBS("_ggiLoadDL(\"%s\", 0x%x) called \n", filename, type);

	hand.name = NULL;
	hand.usecnt = 0;

	if (type & GGI_DLTYPE_GLOBAL) {
		hand.handle = ggLoadModule(filename, GG_MODULE_GLOBAL);
	} else {
		hand.handle = ggLoadModule(filename, 0);
	}
	GGIDPRINT_LIBS("hand.handle=%p\n", hand.handle);
	if (hand.handle == NULL) {
		GGIDPRINT_LIBS("Error loading module %s\n", filename);
		return GGI_ENOFILE;
	}

	nameptr = strrchr(filename, '/');
	if (!nameptr) {
		nameptr = (char*)filename;
	} else {
		nameptr++;
	}
	sprintf(symname, "%s%s", symprefix, nameptr);
	nameptr = strrchr(symname, '.');
	if (nameptr) {
		*nameptr = '\0';
	}

	hand.entry = ggGetSymbolAddress(hand.handle, symname);
	GGIDPRINT_LIBS("&(%s) = %p\n", symname, hand.entry);
	if (hand.entry == NULL) {
		ggFreeModule(hand.handle);
		return GGI_ENOFUNC;
	}

	/* Get pointers */
	hand.entry(GGIFUNC_open, (void**)&hand.open);
	hand.entry(GGIFUNC_exit, (void**)&hand.exit);
	hand.entry(GGIFUNC_close, (void**)&hand.close);
	GGIDPRINT_LIBS("hand.open = %p\n", hand.open);
	GGIDPRINT_LIBS("hand.exit = %p\n", hand.exit);
	GGIDPRINT_LIBS("hand.close = %p\n", hand.close);

	*dlh = malloc(sizeof(**dlh));
	if (*dlh == NULL) {
		ggFreeModule(hand.handle);
		return GGI_ENOMEM;
	}
	memcpy(*dlh, &hand, sizeof(ggi_dlhandle));

	return 0;
}

/* Add an extension DL
 */
ggi_dlhandle *_ggiAddExtDL(ggi_visual *vis, const char *filename,
			   const char *args, void *argptr,
			   const char *symprefix)
{
	ggi_dlhandle_l *tmp;
	ggi_dlhandle *dlh;
	uint32 dlret = 0;
	int err;

	err = _ggiLoadDL(filename, symprefix, GGI_DLTYPE_EXTENSION, &dlh);
	GGIDPRINT_LIBS("_ggiLoadDL returned %d (%p)\n", err, dlh);
	if (err) return NULL;
	
	err = dlh->open(vis, dlh, args, argptr, &dlret);
	GGIDPRINT_LIBS("%d = dlh->open(%p, %p, \"%s\", %p, %d) - %s\n",
		       err, vis, dlh, args ? args : "(null)", argptr, dlret,
		       filename);
	if (err) {
		ggFreeModule(dlh->handle);
		free(dlh);
		return NULL;
	}

	dlh->name = strdup("");
	dlh->usecnt = 1;
	dlh->type = GGI_DLTYPE_EXTENSION;
	dlh->visual = vis;

	tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
	tmp->handle = dlh;
	tmp->next = vis->extlib;
	vis->extlib = tmp;

	tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
	tmp->handle = dlh;
	tmp->next = LIBGGI_DLHANDLE(vis);
	LIBGGI_DLHANDLE(vis) = tmp;

	return dlh;
}

/****** Open and Close a DL *********/
int _ggiAddDL(ggi_visual *vis, const char *name, const char *args,
	      void *argptr, int type)
{
	ggi_dlhandle_l *tmp;
	ggi_dlhandle *dlh;
	const char *filename;
	uint32 dlret = 0;
	int err;

	GGIDPRINT_LIBS("_ggiAddDL(%p, \"%s\", \"%s\", 0x%x) called\n",
		       vis, name, args ? args : "(null)", type);

	if ((filename = ggMatchConfig(_ggiConfigHandle, name, NULL)) == NULL) {
		GGIDPRINT_LIBS("LibGGI: no config entry for sublib: %s\n",
			name);
		return GGI_ENOMATCH;
	}

	err = _ggiLoadDL(filename, GGI_SYMNAME_PREFIX, type, &dlh);
	GGIDPRINT_LIBS("_ggiLoadDL returned %d (%p)\n", err, dlh);
	if (err) return err;

	dlh->type = type;
	dlh->visual = vis;

	err = dlh->open(vis, dlh, args, argptr, &dlret);
	GGIDPRINT_LIBS("%d = dlh->open(%p, %p, \"%s\", %p, %d) - %s\n",
		       err, vis, dlh, args ? args : "(null)", argptr, dlret,
		       filename);
	if (err) {
		ggFreeModule(dlh->handle);
		free(dlh);
		return err;
	}

	if (type == GGI_DLTYPE_INTERNAL) {
		if (dlret & GGI_DL_OPDISPLAY) {
			tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;
			tmp->next = vis->opdisplay->head.dlhandle;
			vis->opdisplay->head.dlhandle = tmp;
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPCOLOR) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;
			tmp->next = vis->opcolor->head.dlhandle;
			vis->opcolor->head.dlhandle = tmp;
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPDRAW) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;
			tmp->next = vis->opdraw->head.dlhandle;
			vis->opdraw->head.dlhandle = tmp;
			dlh->usecnt++;
		}

		if (dlret & GGI_DL_OPGC) {
			tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
			tmp->handle = dlh;
			tmp->next = vis->opgc->head.dlhandle;
			vis->opgc->head.dlhandle = tmp;
			dlh->usecnt++;
		}
	} else {
		dlh->usecnt = 1;
		tmp = _ggi_malloc(sizeof(ggi_dlhandle_l));
		tmp->handle = dlh;
		tmp->next = vis->extlib;
		vis->extlib = tmp;
	}

	if (dlh->usecnt == 0) {
		fprintf(stderr,
			"LibGGI: %s (%s) -> 0x%.8x - no operations in this library\n",
			name, args ? args : "(null)", dlret);
		ggFreeModule(dlh->handle);
		free(dlh);
		return GGI_ENOFUNC;
	} else {
		tmp = (ggi_dlhandle_l *)_ggi_malloc(sizeof(ggi_dlhandle_l));
		tmp->handle = dlh;
		tmp->next = LIBGGI_DLHANDLE(vis);
		LIBGGI_DLHANDLE(vis) = tmp;
	}

	dlh->name = strdup(name);

	return 0;
}

int
_ggiOpenDL(ggi_visual *vis, const char *name, const char *args, void *argptr)
{
	return _ggiAddDL(vis, name, args, argptr, GGI_DLTYPE_INTERNAL);
}

void _ggiExitDL(ggi_visual *vis, ggi_dlhandle_l *lib)
{
	for (; lib != NULL; lib = lib->next) {
		if (lib->handle->exit) {
			lib->handle->exit(vis, lib->handle);
		}
	}
}

static void _ggiRemoveDL(ggi_visual *vis, ggi_dlhandle_l **lib)
{
	ggi_dlhandle_l *tmp, **prev;
	ggi_dlhandle_l *libtmp, **libprev, *libnext;

	for (libprev = lib, libtmp = *lib; libtmp != NULL; libtmp=libnext) {
		libnext = libtmp->next;
		if (libtmp->handle->usecnt <= 0) {
			GGIDPRINT_LIBS("Disposing \"%s\"\n",
				       libtmp->handle->name);
			*libprev = libtmp->next;
			if (libtmp->handle->close) {
				libtmp->handle->close(vis, libtmp->handle);
			}
			GGIDPRINT_LIBS("Closing handle: 0x%x\n",
				       libtmp->handle->handle);
			ggFreeModule(libtmp->handle->handle);

			/* Now, clean up the master visual */
			prev = &LIBGGI_DLHANDLE(vis);
			for (tmp=LIBGGI_DLHANDLE(vis); tmp; tmp=tmp->next) {
				if (tmp->handle == libtmp->handle) break;
				prev = &tmp->next;
			}
			if (!tmp) {
				GGIDPRINT_LIBS("Error: handle not in master list.\n");
			}
			*prev = tmp->next;
			free(tmp);

			free(libtmp->handle->name);
			free(libtmp->handle);
			free(libtmp);
		} else {
			libprev = &libtmp->next;
		}
	}
}

void _ggiZapDL(ggi_visual *vis, ggi_dlhandle_l **lib)
{
	ggi_dlhandle_l *tmp, *next;

	GGIDPRINT_LIBS("_ggiZapDL(%p, %p) called\n", vis, lib);

	for (tmp = *lib; tmp; tmp = tmp->next) {
		tmp->handle->usecnt--;
	}

	_ggiRemoveDL(vis,lib);

	for (tmp = *lib; tmp; tmp = next) {
		next = tmp->next;
		free(tmp);
	}

	*lib = NULL;
}
