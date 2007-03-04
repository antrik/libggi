/* $Id: db.c,v 1.7 2007/03/04 14:44:53 soyt Exp $
******************************************************************************

   DirectBuffer handling.

   Copyright (C) 1998 Marcus Sundberg	[marcus@ggi-project.org]
  
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

#include <string.h>
#include <stdlib.h>
#include "config.h"
#include <ggi/internal/ggi.h>
#include <ggi/internal/ggi_debug.h>


/* Internal functions */

/* _ggi_db_get_new
   Returns a pointer to a newly allocated ggi_directbuffer structure.
*/
ggi_directbuffer *_ggi_db_get_new(void)
{
	ggi_directbuffer *db = malloc(sizeof(ggi_directbuffer));

	if (db == NULL) return NULL;
	
	db->type = GGI_DB_NORMAL;
	db->frame = 0;
	db->resource = NULL;
	db->read = db->write = NULL;
	db->page_size = 0;
	db->noaccess = 0;
	db->align = 0;
	
	return db;
}


/* _ggi_db_free
   Frees a ggi_directbuffer obtained from _ggi_db_get_new.
*/
void _ggi_db_free(ggi_directbuffer *db)
{
	LIB_ASSERT(db != NULL, "_ggi_db_free: db is NULL");
	
	free(db);
}


/* _ggi_db_add_buffer
   Adds a buffer at the end of the list.
   Returns the position of the buffer (starting with 0).
*/
int _ggi_db_add_buffer(ggi_db_list *dbl, ggi_directbuffer *buf)
{
	LIB_ASSERT(dbl != NULL, "_ggi_db_add_buffer: list is NULL");
	LIB_ASSERT(buf != NULL, "_ggi_db_add_buffer: buffer is NULL");

	dbl->num++;
	dbl->bufs = _ggi_realloc(dbl->bufs, sizeof(ggi_directbuffer *) * dbl->num);
	dbl->bufs[dbl->num-1] = buf;

	return dbl->num-1;
}


/* _ggi_db_del_buffer
   Deletes the buffer at position idx from the list.
   Returns the number of buffers left in the list.
*/
int _ggi_db_del_buffer(ggi_db_list *dbl, int idx)
{
	LIB_ASSERT(dbl != NULL, "_ggi_db_del_buffer: list is NULL");
	LIB_ASSERT(dbl->num > 0, "_ggi_db_del_buffer: called for empty list");

	dbl->num--;
	memmove(dbl->bufs+idx, dbl->bufs+idx+1, (dbl->num-idx)*sizeof(ggi_directbuffer));
	if (dbl->num == 0) {
		free(dbl->bufs);
		dbl->bufs = NULL;
	} else {
		dbl->bufs = _ggi_realloc(dbl->bufs, sizeof(ggi_directbuffer *) * dbl->num);
	}
	
	return dbl->num;
}

/* _ggi_db_move_buffer
   Moves the buffer at position idx in src to the end of dst.
   Returns the position of the buffer in dst.
*/
int _ggi_db_move_buffer(ggi_db_list *dst, ggi_db_list *src, int idx)
{
	int pos = _ggi_db_add_buffer(dst, src->bufs[idx]);
	_ggi_db_del_buffer(src, idx);

	return pos;
}


ggi_directbuffer *_ggi_db_find_frame(struct ggi_visual *vis, int frameno)
{
	int i;

	for (i=0; i < LIBGGI_APPLIST(vis)->num; i++) {
		ggi_directbuffer *cur = LIBGGI_APPBUFS(vis)[i];

		if ((cur->type & GGI_DB_NORMAL) && (cur->frame == frameno)) {
			return cur;
		}
	}

	for (i=0; i < LIBGGI_PRIVLIST(vis)->num; i++) {
		ggi_directbuffer *cur = LIBGGI_PRIVBUFS(vis)[i];

		if ((cur->type & GGI_DB_NORMAL) && (cur->frame == frameno)) {
			return cur;
		}
	}

	return NULL;  /* not found */
}


/************** DirectBuffer calls ****************/

int  ggiDBGetNumBuffers(ggi_visual_t v)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	return LIBGGI_APPLIST(vis)->num;
}

const ggi_directbuffer *ggiDBGetBuffer(ggi_visual_t v, int bufnum)
{
	struct ggi_visual *vis = GGI_VISUAL(v);
	if (bufnum >= 0 && bufnum < LIBGGI_APPLIST(vis)->num)
		return LIBGGI_APPLIST(vis)->bufs[bufnum];
	return NULL;
}
