
/* Get the global variables needed to make the card drivers happy */

#define _FBDEV_DIRECTFB_GLOBALS
#include "ggidirectfb.h"

/* Needed for crusty memory_virtual function */
static void *ggi_fbdev_dfb_framebuffer_base; 

static int GGIopen(ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32 *dlret)
{
  struct fbdev_directfb_global *globals;

  globals = argptr;
  globals->dfb_config_ptr =    &dfb_config;
  globals->dfb_fbdev_ptr  =    &dfb_fbdev;
  ggi_fbdev_dfb_framebuffer_base = FBDEV_PRIV(vis)->fb_ptr;

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

/* Warning -- hackery follows and DFB changes too quick, so don't bother
 * trying to make this pretty.  We must provide symbols and in some cases
 * working functions for some callbacks DFB drivers make into their generic 
 * codebase.
 */

int dfb_gfxcard_get_accelerator( GraphicsDevice *device )
{
  return device->shared->fix.accel;
}

volatile void *dfb_gfxcard_map_mmio( GraphicsDevice *device,
				     unsigned int    offset,
				     int             length )
{
  void *addr;

  if (length < 0)
    length = device->shared->fix.mmio_len;

  addr = mmap( NULL, length, PROT_READ | PROT_WRITE, MAP_SHARED,
	       dfb_fbdev->fd, device->shared->fix.smem_len + offset );
  if ((int)(addr) == -1) {
    GGIDPRINT("default-fbdev-directfb: Could not mmap MMIO region "
	      "(offset %d, length %d)!\n", offset, length);
    return NULL;
  }

  return (volatile void*) addr;
}

void dfb_gfxcard_unmap_mmio( GraphicsDevice *device,
			     volatile void  *addr,
			     int             length )
{
  if (length < 0)
    length = device->shared->fix.mmio_len;

  if (munmap( (void*) addr, length ) < 0)
    GGIDPRINT( "default-fbdev-directfb: Could not unmap MMIO region "
	       "at %p (length %d)!\n", addr, length );
}

void dfb_graphics_register_module( GraphicsDriverFuncs *funcs )
{
	/* Abuse unused field in a global to retreive 
	   driver-library-local pointer */
	(GraphicsDriverFuncs *)(dfb_config->mouse_protocol) = funcs;
}

void *dfb_gfxcard_memory_virtual( unsigned int offset )
{
  /* Only used by ATI for blended rectangles, won't need until libbuf/libblt,
   * but must provide valid pointer as it's initialized in the ATI DFB driver's
   * init function. 
   */
  return (void*)((__u8*)(ggi_fbdev_dfb_framebuffer_base) + offset);
}

int dfb_gfxcard_reserve_memory( GraphicsDevice *device, unsigned int size )
{
  /* Only used by ATI for blended rectangles, won't need until libbuf/libblt */
  /* TODO: clean up handling of this resource. */
  return(device->framebuffer.length - size);
}

void dfb_sort_triangle( DFBTriangle *tri )
{
  /*    We don't actually use this yet, but if/when we do,
   *	get the code from DirectFB's src/gfx/util.c file.
   */
}

void
dfb_layers_add( DisplayLayer *layer )
{
  /* Used for overlays only */
}


DFBResult dfb_surface_create(int width, int height, 
			     DFBSurfacePixelFormat format,
			     CoreSurfacePolicy policy,
			     DFBSurfaceCapabilities caps, 
			     CoreSurface **surface
			     )
{
  /* Used for overlays only */
  return -1;
}

CoreWindowStack*
dfb_windowstack_new( DisplayLayer *layer )
{
  
  /* Used for overlays only. */
  return NULL;
}

void reactor_attach (FusionReactor *reactor,
                     React          react,
                     void          *ctx)
{
  /* Only Matrox BES uses this.  Will find out more someday. */
}

#include <ggi/internal/ggidlinit.h>
