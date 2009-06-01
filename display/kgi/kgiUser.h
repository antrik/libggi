#ifndef	_kgi_user_
#define	_kgi_user_

/* Module kgi */

#include <mach/kern_return.h>
#include <mach/port.h>
#include <mach/message.h>

#include <mach/std_types.h>
#include <mach/mach_types.h>
#include <device/device_types.h>
#include <device/net_status.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/resource.h>
#include <sys/utsname.h>
#include <hurd/hurd_types.h>
#include <hurd/trivfs.h>
#include "kgi/kgi.h"

/* Routine kgi_set_images */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_set_images
#if	defined(LINTLIBRARY)
    (io_object, images)
	io_t io_object;
	int images;
{ return kgi_set_images(io_object, images); }
#else
(
	io_t io_object,
	int images
);
#endif

/* Routine kgi_set_image_mode */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_set_image_mode
#if	defined(LINTLIBRARY)
    (io_object, image, mode)
	io_t io_object;
	int image;
	kgi_image_mode_t mode;
{ return kgi_set_image_mode(io_object, image, mode); }
#else
(
	io_t io_object,
	int image,
	kgi_image_mode_t mode
);
#endif

/* Routine kgi_get_image_mode */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_get_image_mode
#if	defined(LINTLIBRARY)
    (io_object, image, mode)
	io_t io_object;
	int image;
	kgi_image_mode_t *mode;
{ return kgi_get_image_mode(io_object, image, mode); }
#else
(
	io_t io_object,
	int image,
	kgi_image_mode_t *mode
);
#endif

/* Routine kgi_check_mode */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_check_mode
#if	defined(LINTLIBRARY)
    (io_object)
	io_t io_object;
{ return kgi_check_mode(io_object); }
#else
(
	io_t io_object
);
#endif

/* Routine kgi_set_mode */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_set_mode
#if	defined(LINTLIBRARY)
    (io_object)
	io_t io_object;
{ return kgi_set_mode(io_object); }
#else
(
	io_t io_object
);
#endif

/* Routine kgi_unset_mode */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_unset_mode
#if	defined(LINTLIBRARY)
    (io_object)
	io_t io_object;
{ return kgi_unset_mode(io_object); }
#else
(
	io_t io_object
);
#endif

/* Routine kgi_get_fb_resource */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_get_fb_resource
#if	defined(LINTLIBRARY)
    (io_object, fb)
	io_t io_object;
	kgic_mapper_resource_info_result_t *fb;
{ return kgi_get_fb_resource(io_object, fb); }
#else
(
	io_t io_object,
	kgic_mapper_resource_info_result_t *fb
);
#endif

/* Routine kgi_setup_mmap_fb */
#ifdef	mig_external
mig_external
#else
extern
#endif
kern_return_t kgi_setup_mmap_fb
#if	defined(LINTLIBRARY)
    (io_object)
	io_t io_object;
{ return kgi_setup_mmap_fb(io_object); }
#else
(
	io_t io_object
);
#endif

#endif	/* not defined(_kgi_user_) */
