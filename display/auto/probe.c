/* $Id: probe.c,v 1.4 2004/10/31 14:25:00 cegger Exp $
******************************************************************************

   Auto target for GGI.

   Copyright (C) 2004 Christoph Egger

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

#include <stdlib.h>
#include <string.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/internal/ggi_debug.h>
#include <ggi/display/auto.h>

static inline void _ggi_setup_display(char *display,
				struct ggi_auto_Target *target, size_t display_size)
{
	ggstrlcpy(display, target->display, display_size);

	if (target->os_options != NULL) {
		ggstrlcat(display, ":", display_size);
		ggstrlcat(display, target->os_options,
				display_size);
	}

	return;
}



ggi_visual_t _GGI_auto_findOptimalTarget(ggi_auto_priv *priv)
{
	int i, j;
	ggi_visual_t vis = NULL;
	char display[1024];

	if (OS_createTargetInfo(priv) != GGI_OK) return NULL;

	for (i = 0; i < priv->num_targets; i++) {
		struct ggi_auto_Target *target = &priv->target[i];

		if (target->probe != NULL) {
			if (target->probe->checkDisplay != NULL) {
				int checkTarget = target->probe->checkDisplay();
				if (!checkTarget) continue;
			}

			if (target->probe->num_options == 0)
				goto probe_default;

			for (j = 0; j < target->probe->num_options; j++) {
				_ggi_setup_display(display, target, sizeof(display));

				if (target->probe->option[j] != NULL) {
					GGIDPRINT("display-auto: option = %s\n",
						target->probe->option[j]);

					ggstrlcat(display, ":", sizeof(display));
					ggstrlcat(display, target->probe->option[j],
						sizeof(display));
				}

				ggDPrintf(1, "LibGGI", "Try to use %s...\n", display);
				vis = ggiOpen(display, NULL);
				if (vis != NULL) goto found_target;
			}

			continue;
		}

	probe_default:
		_ggi_setup_display(display, target, sizeof(display));

		ggDPrintf(1, "LibGGI", "Try to use %s...\n", display);
		vis = ggiOpen(display, NULL);
		if (vis != NULL) goto found_target;
	}

found_target:
	OS_freeTargetInfo(priv);

	return vis;
}	/* _GGI_auto_findOptimalTarget */

