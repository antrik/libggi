/* $Id: init.c,v 1.3 2002/01/27 09:15:24 cegger Exp $
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#include <ggi/internal/internal.h>
#include <ggi/gg.h>


/* Exported variables */
EXPORTVAR uint32      _ggiDebugState   = 0;
EXPORTVAR int         _ggiDebugSync    = 0;
EXPORTVAR void       *_ggi_global_lock = NULL;

/* Global variables */
void                 *_ggiConfigHandle = NULL;

/* Static variables */
static struct {
	void		*mutex;		/* Lock when changing.. */
	int		 visuals;	/* # of visuals active */
	ggi_visual	*visual;	/* Linked list of all visuals */
} _ggiVisuals;	/* This is for remembering visuals. */

static int            _ggiLibIsUp      = 0;
static char           ggiconfstub[512] = GGICONFDIR;
static char          *ggiconfdir       = ggiconfstub + GGITAGLEN;
static int            numextensions    = 0;
static ggi_extension *_ggiExtension    = NULL;


/* 
 * Returns the directory where global config files are kept
 */

const char *ggiGetConfDir(void)
{
#ifdef __WIN32__
	/* On Win32 we allow overriding of the compiled in path. */
	const char *envdir = getenv("GGI_CONFDIR");
	if (envdir) return envdir;
#endif
	return ggiconfdir;
}


/*
 * Initalize the strutures for the library
 */

int ggiInit(void)
{
	int err;
	const char *str, *confdir;
	char *conffile;

	_ggiLibIsUp++;
	if (_ggiLibIsUp > 1) return 0;	/* Initialize only at first call. */

	err = giiInit();
	if (err) {
		fprintf(stderr, "LibGGI: unable to initialize LibGII\n");
		return err;
	}

	if ((_ggiVisuals.mutex = ggLockCreate()) == NULL) {
		fprintf(stderr, "LibGGI: unable to initialize core mutex.\n");
		giiExit();
		return GGI_EUNKNOWN;
	}
	if ((_ggi_global_lock = ggLockCreate()) == NULL) {
		fprintf(stderr,"LibGGI: unable to initialize global mutex.\n");
		ggLockDestroy(_ggiVisuals.mutex);
		giiExit();
		return GGI_EUNKNOWN;
	}
	_ggiVisuals.visuals = 0;
	_ggiVisuals.visual = NULL;

	str = getenv("GGI_DEBUG");
	if (str != NULL) {
		_ggiDebugState = atoi(str);
		GGIDPRINT_CORE("Debugging=%d\n", _ggiDebugState);
	}

	str = getenv("GGI_DEBUGSYNC");
	if (str != NULL) {
		_ggiDebugSync = 1;
	}
		
	str = getenv("GGI_DEFMODE");
	if (str != NULL) {
		_ggiSetDefaultMode(str);
	}

	confdir = ggiGetConfDir();
	conffile = malloc(strlen(confdir) + 1 + strlen(GGICONFFILE)+1);
	if (!conffile) {
		fprintf(stderr, "LibGGI: unable to allocate memory for config filename.\n");
	} else {
#ifdef HAVE_SNPRINTF
		snprintf(conffile, strlen(confdir) + strlen(GGICONFFILE) + 2,
			"%s/%s", confdir, GGICONFFILE);
#else
		sprintf(conffile, "%s/%s", confdir, GGICONFFILE);
#endif
		err = ggLoadConfig(conffile, &_ggiConfigHandle);
		if (err == GGI_OK) {
			free(conffile);
			return GGI_OK;
		}
		fprintf(stderr,"LibGGI: couldn't open %s.\n", conffile);
		free(conffile);
	}

	ggLockDestroy(_ggiVisuals.mutex);
	ggLockDestroy(_ggi_global_lock);
	giiExit();
	_ggiLibIsUp--;

	return err;
}

int ggiExit(void)
{
	ggi_extension *tmp, *next;

	GGIDPRINT_CORE("ggiExit called\n");
	if (!_ggiLibIsUp) return GGI_ENOTALLOC;

	if (_ggiLibIsUp > 1) {
		_ggiLibIsUp--;
		return _ggiLibIsUp;
	}

	GGIDPRINT_CORE("ggiExit: really destroying.\n");
	while (_ggiVisuals.visual != NULL) ggiClose(_ggiVisuals.visual);

	ggLockDestroy(_ggiVisuals.mutex);
	ggLockDestroy(_ggi_global_lock);
	
	for (tmp = _ggiExtension; tmp; tmp = next) {
		next = tmp->next;
		free(tmp);
	}

	ggFreeConfig(_ggiConfigHandle);
	giiExit();
	_ggiLibIsUp = 0;

	GGIDPRINT_CORE("ggiExit: done!\n");
	return 0;
}

void ggiPanic(const char *format,...)
{
	va_list ap;

	GGIDPRINT_CORE("ggiPanic called\n");

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
	va_list drivers;
	ggi_visual *vis;
	char *cp, *inplist;
	char str[1024];
	char target[1024];
	int  success=0;
	void *argptr;
	static int globalopencount=0;

	if (!_ggiLibIsUp) return NULL;

	GGIDPRINT_CORE("ggiOpen(\"%s\") called\n", driver);

	if (driver==NULL) {
		void *ret;
		
		/* If GGI_DISPLAY is set, use it. Fall back to "auto" 
		 * otherwise.
		 */

		cp=getenv("GGI_DISPLAY");
		if (cp!=NULL) {
			ret = ggiOpen(cp,NULL);
			return ret;
		}
		driver="auto";
	}
	if (strcmp(driver,"auto")==0) {

		void *ret;
		
		/* Try the X display.. */
		cp=getenv("DISPLAY");
		if (cp!=NULL) {
			strcpy(str,"display-x:");
#ifdef HAVE_STRNCAT
			strncat(str, cp, 1024);
#else
			strcat(str, cp);
#endif
			ret = ggiOpen(str,NULL);
			if (ret != NULL)
				return ret;
		}

#if 0
		/* Try the KGI console.. */
		ret=ggiOpen("display-KGI:/dev/graphic",NULL);
		if (ret != NULL)
			return ret;
#endif
		
		/* Try the framebuffer console.. */
		ret = ggiOpen("display-fbdev", NULL);
		if (ret != NULL)
			return ret;

		/* Try svgalib target.. */
		ret = ggiOpen("display-svga",NULL);
		if (ret != NULL)
			return ret;

		/* Try AAlib target.. */
		ret = ggiOpen("display-aa",NULL);
		if (ret != NULL)
			return ret;

		/* This fallthrough is for a smooth transition, when someone 
		 * makes a display-auto target :
		 */
	}

	if ((vis = _ggiNewVisual()) == NULL) {
		return NULL;
	}

	va_start(drivers,driver);

	argptr=va_arg(drivers,void *);
	va_end(drivers);

	GGIDPRINT_CORE("Loading driver %s\n",driver);

	do {
		if (ggParseTarget((char *)driver,target,1024) == NULL) {
			break;
		}

		if (strlen(target) == 0) {
			fprintf(stderr, "LibGGI: Missing target descriptor !\n");
			break;
		}
		
		cp=strchr(target, ':');

		if (cp != NULL) {
			*cp++ = 0;
		}
		if (_ggiOpenDL(vis,target,cp,argptr) == 0) {
			success = 1;
		}

	} while (0);
	

	va_end(drivers);

	if (success) {
		ggLock(_ggiVisuals.mutex);
		vis->next=_ggiVisuals.visual;
		_ggiVisuals.visual=vis;
		_ggiVisuals.visuals++;
		ggUnlock(_ggiVisuals.mutex);
		GGIDPRINT_CORE("ggiOpen: returning %p\n", vis);
	} else {
		_ggiDestroyVisual(vis);
		GGIDPRINT_CORE("ggiOpen: failure\n");
		return NULL;
	}

	GGIDPRINT_CORE("Loading extra inputs/filters for %s\n",driver);

	inplist=NULL;

#ifdef HAVE_SNPRINTF
	snprintf(str, 1024, "GGI_INPUT_%s_%d", target, ++globalopencount);
#else
	sprintf(str, "GGI_INPUT_%s_%d", target, ++globalopencount);
#endif
	mangle_variable(str);
	if (!inplist) { 
		inplist = getenv(str);
		GGIDPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
	}

#ifdef HAVE_SNPRINTF
	snprintf(str, 1024, "GGI_INPUT_%s", target);
#else
	sprintf(str, "GGI_INPUT_%s", target);
#endif
	mangle_variable(str);
	if (!inplist) {
		inplist = getenv(str);
		GGIDPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
	}

	strcpy(str,"GGI_INPUT");
	if (!inplist) {
		inplist = getenv(str);
		GGIDPRINT_CORE("Checking %s : %s\n",str,inplist ? inplist : "(nil)");
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
			GGIDPRINT_CORE("Cannot open input-null\n");
			ggiClose(vis);
			return NULL;
		}
	}
	
	return vis;
}
	
/* ggiClose
 *	Closes the requested visual
 *      Returns 0 on success, < 0 on error
 */
int ggiClose(ggi_visual *visual)
{
	ggi_visual *vis,*pvis=NULL;

	GGIDPRINT_CORE("ggiClose(\"%p\") called\n", visual);

	if (!_ggiLibIsUp) return GGI_ENOTALLOC;

	GGIDPRINT_CORE("ggiClose: closing\n");

	for (vis = _ggiVisuals.visual; vis != NULL; pvis=vis, vis=vis->next) {
		if (vis == visual) break;
	}

	if (vis == NULL) return GGI_EARGINVAL;

	ggLock(_ggiVisuals.mutex);

	if (pvis == NULL) _ggiVisuals.visual = vis->next;
	else pvis->next = vis->next;

	_ggiVisuals.visuals--;
	
	ggUnlock(_ggiVisuals.mutex);

	_ggiDestroyVisual(vis);

	GGIDPRINT_CORE("ggiClose: done!\n");

	return 0;
}

/*
  Extension handling.
*/

/*
  Register an Extension for usage. It will return an extension ID 
  (ggi_extid) that can be used to address this extension.
*/
ggi_extid
ggiExtensionRegister(char *name, int size, int (*change)(ggi_visual_t, int))
{
	ggi_extension *tmp, *ext;

	GGIDPRINT_CORE("ggiExtensionRegister(\"%s\", %d, %p) called\n",
		       name, size, change);
	if (_ggiExtension) {
		for (tmp = _ggiExtension; tmp == NULL; tmp=tmp->next) {
			if (strcmp(tmp->name, name) == 0) {
				tmp->initcount++;
				GGIDPRINT_CORE("ggiExtensionRegister: accepting copy #%d of extension %s\n",
					       tmp->initcount,tmp->name);
				return tmp->id;
			}
		}
	}

	ext = malloc(sizeof(ggi_extension));
	if (ext == NULL) return GGI_ENOMEM;

	ext->size = size;
	ext->paramchange = change;
	ext->next = NULL;
	ext->initcount = 1;
	strncpy(ext->name, name, sizeof(ext->name));
	ext->name[sizeof(ext->name)-1] = '\0';	/* Make sure it terminates */

	if (_ggiExtension) {
		for (tmp = _ggiExtension; tmp->next; tmp=tmp->next) ;
		tmp->next = ext;
		ext->prev = tmp;
	} else {
		_ggiExtension = ext;
		ext->prev = NULL;
	}

	GGIDPRINT_CORE("ggiExtensionRegister: installing first copy of extension %s\n", name);

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

	GGIDPRINT_CORE("ggiExtensionUnregister(%d) called\n", id);
	if (!_ggiExtension) return GGI_ENOTALLOC;

	for (tmp = _ggiExtension; tmp != NULL; tmp=tmp->next) {
		if (tmp->id != id) continue;
		if (--tmp->initcount) {
			GGIDPRINT_CORE("ggiExtensionUnregister: removing #%d copy of extension %s\n", tmp->initcount+1, tmp->name);
			/* Removed copy */
			return 0;	
		}

		if (tmp->prev == NULL) _ggiExtension = tmp->next;
		else tmp->prev->next = tmp->next;

		if (tmp->next != NULL) tmp->next->prev = tmp->prev;

		GGIDPRINT_CORE("ggiExtensionUnregister: removing last copy of extension %s\n",
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
       0 	means you installed the extension as the first one. Note that
     >=0 	should be regarded as "success". It is legal to attach an 
         	extension multiple times. You might want to set up private
         	data if RC==0.
*/
int ggiExtensionAttach(ggi_visual *vis, ggi_extid id)
{
	ggi_extension *tmp = NULL;

	GGIDPRINT_CORE("ggiExtensionAttach(%p, %d) called\n", vis, id);

	if (_ggiExtension) {
		for (tmp = _ggiExtension; tmp != NULL; tmp = tmp->next) {
			if (tmp->id == id) break;
		}
	}
	if (! tmp) return GGI_EARGINVAL;

	if (vis->numknownext <= id) {
		ggi_extlist *newlist;
		int extsize = sizeof(*vis->extlist);
		
		newlist = realloc(vis->extlist, extsize*(id+1));
		if (newlist == NULL) return GGI_ENOMEM;

		vis->extlist = newlist;
		memset(&vis->extlist[vis->numknownext], 0,
		       extsize*(id+1-vis->numknownext));
		vis->numknownext = id+1;
		GGIDPRINT_CORE("ggiExtensionAttach: ExtList now at %p (%d)\n",
			       vis->extlist, vis->numknownext);
	}

	if (vis->extlist[id].attachcount == 0) {
		vis->extlist[id].priv = malloc(tmp->size);
		if (vis->extlist[id].priv == NULL) return GGI_ENOMEM;
	}

	return vis->extlist[id].attachcount++;
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
int ggiExtensionDetach(ggi_visual *vis, ggi_extid id)
{
	GGIDPRINT_CORE("ggiExtensionDetach(%p, %d) called\n", vis, id);

	if (vis->numknownext <= id || vis->extlist[id].attachcount == 0) {
	     	return GGI_EARGINVAL;
	}

	if (--vis->extlist[id].attachcount) {
		return vis->extlist[id].attachcount;
	}
	
	free(vis->extlist[id].priv);
	vis->extlist[id].priv = NULL;	/* Make sure ... */

	return 0;
}

int ggiIndicateChange(ggi_visual_t vis, int whatchanged)
{
	ggi_extension *tmp = NULL;

	GGIDPRINT_CORE("ggiIndicateChange(%p, 0x%x) called\n",
		       vis, whatchanged);

	/* Tell all attached extensions on this visual */
	GGIDPRINT_CORE("ggiIndicateChange: %i changed for %p.\n",
		       whatchanged, vis);

	if (_ggiExtension) {
		for (tmp = _ggiExtension; tmp != NULL; tmp=tmp->next) {
			if (tmp->id < vis->numknownext &&
			    vis->extlist[tmp->id].attachcount) {
				tmp->paramchange(vis, whatchanged);
			}
		}
	}

	return 0;
}

#include <ggi/internal/ggilibinit.h>
