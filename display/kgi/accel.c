/* $Id: accel.c,v 1.4 2005/01/23 21:57:20 nsouch Exp $
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

#include <unistd.h>
#include <sys/mman.h>
#include "kgi/config.h"
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/kgi.h>

ggi_accel_t *GGI_kgi_map_accelerator(ggi_visual_t vis, kgi_u_t resource,
				     kgi_u_t min, kgi_u_t max,
				     kgi_u_t buffers, kgi_u_t priority)
{
	ggi_accel_t *accel;
	kgi_error_t err;
	
	err = kgiSetupMmapAccel(&KGI_CTX(vis), resource, 
				min, max, buffers, priority);

	DPRINT("setup err: %d\n", err);
	
	if (err != KGI_EOK)
		return NULL;
	
	accel = malloc(sizeof(ggi_accel_t));
	if (! accel)
		return NULL;
	
	accel->u32.current = 0;
	accel->u32.buffer = mmap(0, (0x1000 << max) * buffers, 
				 PROT_READ | PROT_WRITE, MAP_SHARED,
				 KGI_CTX(vis).mapper.fd, 0);

	DPRINT("mmap buffer: %d\n", accel->u32.buffer);
	
	if (accel->u32.buffer == MAP_FAILED) {
	
		free(accel);
		return NULL;
	}
	
	return accel;
}
