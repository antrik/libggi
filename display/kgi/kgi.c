#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <ggi/display/kgi.h>

kgi_error_t kgiInit(kgi_context_t *ctx, const char *client,
	const kgi_version_t *version, const gg_option *options)
{
	union {
		kgic_mapper_identify_request_t	request;
		kgic_mapper_identify_result_t	result;
	} cb;

	if (NULL == ctx) {

		return -KGI_NOMEM;
	}
	if ((NULL == client) || (NULL == version)) {

		return -KGI_INVAL;
	}

	memset(ctx, 0, sizeof(*ctx));

	ctx->mapper.fd = open(options[KGI_OPT_DEVICE].result, O_RDWR);
	if (ctx->mapper.fd < 0) {

		fprintf(stderr, "failed to open device %s\n",
			options[KGI_OPT_DEVICE].result);
		return -KGI_INVAL;
	}

	memset(&cb, 0, sizeof(cb));
	strncpy(cb.request.client, client,
		sizeof(cb.request.client));
	cb.request.client[sizeof(cb.request.client) - 1] = 0;
	cb.request.client_version = *version;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_IDENTIFY, &cb)) {

		fprintf(stderr, "failed to identify to mapper\n");
		return errno;
	}
	printf("identified to mapper %s-%i.%i.%i-%i\n",
		cb.result.mapper,
		cb.result.mapper_version.major,
		cb.result.mapper_version.minor,
		cb.result.mapper_version.patch,
		cb.result.mapper_version.extra);
	ctx->mapper.resources =
		cb.result.resources;

	return KGI_EOK;
}

kgi_error_t kgiSetImages(kgi_context_t *ctx, kgi_u_t images)
{
	if ((NULL == ctx) || (ctx->mapper.fd < 0)) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGES, &images) 
		? errno : KGI_EOK; 
}

kgi_error_t kgiSetImageMode(kgi_context_t *ctx, kgi_u_t image,
	const kgi_image_mode_t *mode)
{
	kgic_mapper_set_image_mode_request_t	cb;

	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	cb.image = image;
	memcpy(&cb.mode, mode, sizeof(cb.mode));
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_SET_IMAGE_MODE, &cb)
		? errno : KGI_EOK;
}

kgi_error_t kgiGetImageMode(kgi_context_t *ctx, kgi_u_t image,
	kgi_image_mode_t *mode)
{
	union {
		kgic_mapper_get_image_mode_request_t	request;
		kgic_mapper_get_image_mode_result_t	result;
	} cb;

	if ((NULL == ctx) || (ctx->mapper.fd < 0) || (NULL == mode)) {

		return -KGI_INVAL;
	}

	cb.request.image = image;
	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_GET_IMAGE_MODE, &cb)) {

		return errno;
	}
	memcpy(mode, &cb.result, sizeof(*mode));
	mode->out = NULL;
	return KGI_EOK;
}

kgi_error_t kgiCheckMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_CHECK, 0)
		? errno : KGI_EOK;
}

kgi_error_t kgiSetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_SET, 0)
		? errno : KGI_EOK;
}

kgi_error_t kgiUnsetMode(kgi_context_t *ctx)
{
	if (NULL == ctx) {

		return -KGI_INVAL;
	}
	return ioctl(ctx->mapper.fd, KGIC_MAPPER_MODE_DONE, 0)
		? errno : KGI_EOK;
}

const kgic_mapper_resource_info_result_t *
kgiGetResource(kgi_context_t *ctx, kgi_u_t start, kgi_resource_type_t type)
{
	kgi_error_t err;
	static union {
		kgic_mapper_resource_info_request_t	request;
		kgic_mapper_resource_info_result_t	result;
	} cb;
    
	cb.request.image = -1;
	cb.request.resource = start;
    
	do {
		err = ioctl(ctx->mapper.fd, KGIC_MAPPER_RESOURCE_INFO, &cb);

		if (!err && (cb.result.type & KGI_RT_MASK) == type)
			return &cb.result;

		++cb.request.resource;
		
		
	} while (!err);
	
	return NULL;
}

kgi_error_t kgiSetupMmapAccel(kgi_context_t *ctx, kgi_u_t resource,
	kgi_u_t min, kgi_u_t max, kgi_u_t buf, kgi_u_t priority)
{
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
}

kgi_error_t kgiSetupMmapFB(kgi_context_t *ctx, kgi_u_t resource)
{
	kgi_error_t err;
	static union {
		kgic_mapper_mmap_setup_request_t	request;
		kgic_mapper_mmap_setup_result_t		result;
	} cb;
    
	cb.request.type = KGI_RT_MMIO;
	cb.request.image = -1;
	cb.request.resource = resource;
    
	err = ioctl(ctx->mapper.fd, KGIC_MAPPER_MMAP_SETUP, &cb)
		? errno : KGI_EOK;
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
}
*/	

void kgiPrintImageMode(kgi_image_mode_t *mode)
{
	printf("%ix%i (%ix%i) \n", mode->size.x, mode->size.y, 
		mode->virt.x, mode->virt.y);
}

kgi_error_t kgiPrintResourceInfo(kgi_context_t *ctx, kgi_u_t resource)
{
	union {
		kgic_mapper_resource_info_request_t	request;
		kgic_mapper_resource_info_result_t	result;
	} cb;

	cb.request.image = -1;
	cb.request.resource = resource;

	if (ioctl(ctx->mapper.fd, KGIC_MAPPER_RESOURCE_INFO, &cb)) {

		return errno;
	}

	printf("resource %i (%s) is ", cb.result.resource, cb.result.name);
	switch (cb.result.type & KGI_RT_MASK) {

	case KGI_RT_MMIO:
		printf("MMIO: window %i, size %i, align %.8x, "
			"access %.8x\n",
			cb.result.info.mmio.window,
			cb.result.info.mmio.size,
			cb.result.info.mmio.align,
			cb.result.info.mmio.access);
		break;

	case KGI_RT_ACCEL:
		printf("ACCEL: recommended are %i buffers of size %i\n",
			cb.result.info.accel.buffers,
			cb.result.info.accel.buffer_size);
		break;

	case KGI_RT_SHMEM:
		printf("SHMEM: (maximum) aperture size %i\n",
			cb.result.info.shmem.aperture_size);
		break;

	default:
		printf("of unknown type\n");
	}
	return KGI_EOK;
}