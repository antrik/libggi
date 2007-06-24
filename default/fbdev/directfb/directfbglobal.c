/* $Id: directfbglobal.c,v 1.17 2007/06/24 13:37:14 aldot Exp $ */
/* Get the global variables needed to make the card drivers happy */

#define _FBDEV_DIRECTFB_GLOBALS
#include "ggidirectfb.h"

/* Needed for memory_virtual/memory_physical functions */
static void *ggi_fbdev_dfb_framebuffer_base; 

static int GGIopen(struct ggi_visual *vis, struct ggi_dlhandle *dlh,
		   const char *args, void *argptr, uint32_t *dlret)
{
  struct fbdev_directfb_global *globals;

  globals = argptr;
  globals->dfb_config_ptr =    &dfb_config;
  globals->dfb_fbdev_ptr  =    &dfb_fbdev;
  ggi_fbdev_dfb_framebuffer_base = FBDEV_PRIV(vis)->fb_ptr;

  return 0;
}

static int GGIclose(struct ggi_visual *vis, struct ggi_dlhandle *dlh)
{
  return 0;
}


EXPORTFUNC
int GGIdl_fbdev_directfbglobal(int func, void **funcptr);

int GGIdl_fbdev_directfbglobal(int func, void **funcptr)
{
	switch (func) {
	case GGIFUNC_open:
		*funcptr = (void *)GGIopen;
		return 0;
	case GGIFUNC_exit:
		*funcptr = NULL;
		return 0;
	case GGIFUNC_close:
		*funcptr = (void *)GGIclose;
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
    DPRINT("default-fbdev-directfb: Could not mmap MMIO region "
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
    DPRINT( "default-fbdev-directfb: Could not unmap MMIO region "
	       "at %p (length %d)!\n", addr, length );
}

ModuleDirectory dfb_graphics_drivers;

void dfb_modules_register(ModuleDirectory *directory,
			  unsigned int     abi_version,
                          const char      *name,
                          const void      *funcs )
{
	/* Abuse unused field in a global to retrieve
	   driver-library-local pointer */
	(GraphicsDriverFuncs *)(dfb_config->mouse_protocol) = funcs;
}

void dfb_graphics_register_module( GraphicsDriverFuncs *funcs )
{
	/* Abuse unused field in a global to retrieve
	   driver-library-local pointer */
	(GraphicsDriverFuncs *)(dfb_config->mouse_protocol) = funcs;
}

unsigned long
dfb_gfxcard_memory_physical(GraphicsDevice *device, unsigned int offset )
{
  /* Only used by ATI for blended rectangles, won't need until libbuf/libblt,
   * but must provide valid pointer as it's initialized in the ATI DFB driver's
   * init function. 
   */
	return (unsigned long)((__u8*)(dfb_fbdev->shared->fix.smem_start) + 
			       offset);
}

void *dfb_gfxcard_memory_virtual( GraphicsDevice *device, unsigned int offset )
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

CoreSurface       *dfb_layer_surface( const DisplayLayer *layer ) 
{ 
	return NULL; /* Used for overlays only */ 
}

DFBResult errno2dfb( int erno ) {
	return DFB_FAILURE;
}

void dfb_surface_flip_buffers( CoreSurface *surface ) {
	/* Used for special features. */ 
}

VideoMode *
dfb_system_current_mode()
{
	return NULL; /* Used only in Matrox BES */
}

DFBResult          dfb_layer_flip_buffers( DisplayLayer *layer,
					   DFBSurfaceFlipFlags flags )
{ 
	return DFB_OK; /* Used for overlays only (Matrox BES) */ 
}

typedef void * DisplayLayerFuncs;
void dfb_layers_register( GraphicsDevice    *device,
                          void              *driver_data,
                          DisplayLayerFuncs *funcs ) 
{ 
  /* Used for overlays only */ 
}

DFBResult dfb_fbdev_wait_vsync() {
  /* Only works with a DFB-specific kernel patch anyway. */
  return 0;
}

DFBResult dfb_system_wait_vsync() {
  /* Only works with a DFB-specific kernel patch anyway. */
  return 0;
}

void dfb_primary_layer_rectangle( float x, float y, float w, float h,
				  DFBRectangle *rect )
{ 
  /* Used for overlays only */ 
}

DFBSurfacePixelFormat dfb_primary_layer_pixelformat() 
{
  /* Used for overlays only */ 
  return 0;
}

/* I think this one is old and no longer used. */
void dfb_layers_add( DisplayLayer *layer )
{
  /* Used for overlays only */
}

/* I think this one is old and no longer used. */
DFBResult dfb_surface_create( int                      width,
                              int                      height,
                              DFBSurfacePixelFormat    format,
                              CoreSurfacePolicy        policy,
                              DFBSurfaceCapabilities   caps,
			      CorePalette             *palette,
                              CoreSurface            **surface )
{
  /* Used for overlays only */
  return GGI_ENOFUNC;
}

CoreWindowStack* dfb_windowstack_new( DisplayLayer *layer )
{
  
  /* Used for overlays only. */
  return NULL;
}

FusionResult reactor_attach (FusionReactor *reactor,
                     React          react,
                     void          *ctx, 
		     Reaction      *reaction)
{
  /* Only Matrox BES uses this.  Will find out more someday. */
  return FUSION_SUCCESS;
}
