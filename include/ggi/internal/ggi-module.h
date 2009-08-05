/*
 * Copyright (c) 2007 Eric Faurot <eric.faurot@gmail.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <ggi/gg-api.h>


struct ggi_visual;
struct ggi_helper;


#define GGI_MODULE_DISPLAY  0
#define GGI_MODULE_HELPER   1


/*
 * A display module is one that provides an implementation for a
 * visual.  All visuals must be opened by one and only one display
 * module to be usable.
 */
struct ggi_module_display {
	struct gg_module module;
	
	int (*open)(struct ggi_visual*, const char*, void*);
	void (*close)(struct ggi_visual*);
};

/*
 * A helper module is one that provides additional features to (or
 * support for) a visual.
 */
struct ggi_module_helper {
	struct gg_module module;

	int (*setup)(struct ggi_helper *, const char *, void *);
	void (*finish)(struct ggi_helper *);
	void (*teardown)(struct ggi_helper *);
};

/*
 * A helper is a plugin that is be bound to a visual.
 */
struct ggi_helper {
	struct gg_plugin plugin;

	struct ggi_visual *visual;

	/* list of helpers on that visual, don't touch. */
	GG_LIST_ENTRY(ggi_helper) h_list;

	/* module-specific data */
	void *priv;
};
