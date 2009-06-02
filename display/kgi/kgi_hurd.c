/* $Id: kgi.c,v 1.20 2009/05/24 18:19:38 antrik Exp $
******************************************************************************


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

#define _GNU_SOURCE

#include <assert.h>
#include <fcntl.h>
#include <hurd/fd.h>
#include <string.h>

#include <ggi/display/kgi.h>
#include <ggi/internal/ggi_debug.h>

#include "kgiUser.h"

kgi_error_t kgiInit(kgi_context_t *ctx, const char *client,
		    const kgi_version_t *version, const gg_option *options)
{
	char fopt[2048], *fname;

	if (NULL == ctx) {
		return -KGI_NOMEM;
	}
	if ((NULL == client) || (NULL == version)) {
		return -KGI_INVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	ggstrlcpy(fopt, options[KGI_OPT_DEVICE].result, sizeof(fopt));
	if (strlen(options[KGI_OPT_DEVICE].result) > 2047) {
		fprintf(stderr, "option string too long for device\n");
	}
	fname = fopt;
	while (strchr(fname, ',') != NULL) {
		*(strchr(fname, ',')) = '\0';
		ctx->mapper.fd = open(fname, O_RDWR);
		if (ctx->mapper.fd >= 0) goto found;
		fprintf(stderr, "failed to open device %s\n", fname);
		fname += strlen(fname) + 1;
	}
	ctx->mapper.fd = open(fname, O_RDWR);
	if (ctx->mapper.fd >= 0) goto found;
	fprintf(stderr, "failed to open device %s.\n", fname);
	return -KGI_INVAL;
 found:
	return KGI_EOK;
}

kgi_error_t kgiSetImages(kgi_context_t *ctx, kgi_u_t images)
{
	if ((NULL == ctx) || (ctx->mapper.fd < 0)) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_set_images(port, images));
}

kgi_error_t kgiSetImageMode(kgi_context_t *ctx, kgi_u_t image,
	const kgi_image_mode_t *mode)
{
	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_set_image_mode(port, image, *mode));
}

kgi_error_t kgiGetImageMode(kgi_context_t *ctx, kgi_u_t image,
	kgi_image_mode_t *mode)
{
	kgi_error_t err;

	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	err = HURD_DPORT_USE(ctx->mapper.fd, kgi_get_image_mode(port, image, mode));
	mode->out = NULL;
	return err;
}

kgi_error_t kgiCheckMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_check_mode(port));
}

kgi_error_t kgiSetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_set_mode(port));
}

kgi_error_t kgiUnsetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_unset_mode(port));
}

kgi_error_t kgiGetResource(kgi_context_t *ctx, kgi_u_t start,
	kgi_resource_type_t type,
	const kgic_mapper_resource_info_result_t **info)
{
	static kgic_mapper_resource_info_result_t result;

	*info = &result;

	if (NULL == ctx) {

		return -KGI_INVAL;
	}

	assert(!start);    /* we assume nobody ever uses that, but make sure */

	switch (type) {

	case KGI_RT_MMIO:
		return HURD_DPORT_USE(ctx->mapper.fd, kgi_get_fb_resource(port, &result));

	default:
		return EOPNOTSUPP;
	}
}

kgi_error_t kgiSetupMmapAccel(kgi_context_t *ctx, kgi_u_t resource,
	kgi_u_t min, kgi_u_t max, kgi_u_t buf, kgi_u_t priority)
{
	return EOPNOTSUPP;
#if 0
	static union {
		kgic_mapper_mmap_setup_request_t	request;
		kgic_mapper_mmap_setup_result_t		result;
	} cb;
    
	cb.request.type = KGI_RT_ACCEL;
	cb.request.image = -1;
	cb.request.resource = resource;
	cb.request.private.accel.min_order = min;
	cb.request.private.accel.max_order = max;
	cb.request.private.accel.buffers = buf;
	cb.request.private.accel.priority = priority;
    
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MMAP_SETUP, &cb)
		? errno : KGI_EOK;
#endif
}

kgi_error_t kgiSetupMmapFB(kgi_context_t *ctx, kgi_u_t resource)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}

	return HURD_DPORT_USE(ctx->mapper.fd, kgi_setup_mmap_fb(port));
}

/*
kgi_error_t kgiSetDisplayOrigin(kgi_context_t *ctx, kgi_u_t x, kgi_u_t y)
{
	kgi_error_t err;
	static union {
		kgic_origin_set_request_t	request;
		kgic_origin_set_result_t	result;
	} cb;

	cb.request.image = -1;
	cb.request.resource = 2;
	cb.request.x = x;
	cb.request.y = y;

	err = ioctl(ctx->mapper.fd, KGIC_RESOURCE_ORIGIN_SET, &cb)
		? errno : KGI_EOK;
	return err;
}
*/

void kgiPrintImageMode(kgi_image_mode_t *mode)
{
	DPRINT_MODE("%ix%i (%ix%i) \n", mode->size.x, mode->size.y, 
		       mode->virt.x, mode->virt.y);
}

kgi_error_t kgiPrintResourceInfo(kgi_context_t *ctx, kgi_u_t resource)
{
	return EOPNOTSUPP;
#if 0
	union {
		kgic_mapper_resource_info_request_t	request;
		kgic_mapper_resource_info_result_t	result;
	} cb;

	cb.request.image = -1;
	cb.request.resource = resource;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_RESOURCE_INFO, &cb)) {

		return errno;
	}

	DPRINT_MISC("resource %i (%s) is ", cb.result.resource, cb.result.name);
	switch (cb.result.type & KGI_RT_MASK) {

	case KGI_RT_MMIO:
		DPRINT_MISC("MMIO: window %i, size %i, align %.8x, "
			"access %.8x\n",
			cb.result.info.mmio.window,
			cb.result.info.mmio.size,
			cb.result.info.mmio.align,
			cb.result.info.mmio.access);
		break;

	case KGI_RT_ACCEL:
		DPRINT_MISC("ACCEL: recommended are %i buffers of size %i\n",
			cb.result.info.accel.buffers,
			cb.result.info.accel.buffer_size);
		break;

	case KGI_RT_SHMEM:
		DPRINT_MISC("SHMEM: (maximum) aperture size %i\n",
			cb.result.info.shmem.aperture_size);
		break;

	default:
		DPRINT_MISC("of unknown type\n");
	}
	return KGI_EOK;
#endif
}

kgi_error_t kgiSetIlut(kgi_context_t *ctx, const kgic_ilut_set_request_t *ilut)
{
	return EOPNOTSUPP;
#if 0
	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == ilut)) {

		return -KGI_INVAL;
	}

	return ioctl(ctx->mapper.fd, KGIC_RESOURCE_CLUT_SET, ilut)
		? errno : KGI_EOK;
#endif
}
