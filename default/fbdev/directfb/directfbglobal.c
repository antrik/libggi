
/* Get the global variables needed to make the card drivers happy */

#define _FBDEV_DIRECTFB_GLOBALS
#include "ggidirectfb.h"

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
  struct fbdev_directfb_global *globals;


  globals = argptr;
  globals->dfb_card_ptr =      &card;
  globals->dfb_config_ptr =    &dfb_config;
  globals->dfb_major_version = &directfb_major_version;
  globals->dfb_minor_version = &directfb_minor_version;
  globals->dfb_micro_version = &directfb_micro_version;
  globals->dfb_binary_age =    &directfb_binary_age;
  globals->dfb_interface_age = &directfb_interface_age;

  fprintf(stderr,"Globals %p %p %p %p %p %p %p\n",
	  globals->dfb_card_ptr, globals->dfb_config_ptr,
	  globals->dfb_major_version, globals->dfb_minor_version,
	  globals->dfb_micro_version, globals->dfb_interface_age,
	  globals->dfb_binary_age);

  return 0;
}

static int GGIclose(ggi_visual *vis, struct ggi_dlhandle *dlh)
{
  return 0;
}


int GGIdl_directfbglobal(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = GGIclose;
		return 0;
	default:
		*funcptr = NULL;
	}

	return GGI_ENOTFOUND;
}

#include <ggi/internal/ggidlinit.h>
