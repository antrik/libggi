/* $Id: fileio.c,v 1.3 2003/07/06 10:25:22 cegger Exp $
******************************************************************************

   Display-file: file primitives

   Copyright (C) 1998 Andrew Apted    [andrew@ggi-project.org]

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"
#include <ggi/internal/ggi-dl.h>
#include <ggi/display/file.h>


int _ggi_file_create_file(ggi_visual *vis, char *filename)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	LIBGGI_FD(vis) = open(filename, O_CREAT | O_TRUNC | O_RDWR, 0600);

	if (LIBGGI_FD(vis) < 0) {
		perror("display-file: Unable to create file");
		return -1;
	}

	priv->buf_len = 0;

	return 0;
}

void _ggi_file_close_file(ggi_visual *vis)
{
	_ggi_file_flush(vis);

	close(LIBGGI_FD(vis));

	LIBGGI_FD(vis) = -1;
}

void _ggi_file_rewind(ggi_visual *vis)
{
	_ggi_file_flush(vis);

	lseek(LIBGGI_FD(vis),0L,SEEK_SET);
}

void _ggi_file_flush(ggi_visual *vis)
{
	ggi_file_priv *priv = FILE_PRIV(vis);

	if (priv->buf_len <= 0) {
		return;
	}

	if (write(LIBGGI_FD(vis), priv->buffer,
		(unsigned)priv->buf_len) < 0)
	{
		perror("display-file: write error");
	}

	priv->buf_len = 0;
}

void _ggi_file_write_byte(ggi_visual *vis, unsigned int val)
{
	ggi_file_priv *priv = FILE_PRIV(vis);
	
	if (priv->buf_len >= FILE_BUFFER_SIZE) {
		_ggi_file_flush(vis);
	}

	priv->buffer[priv->buf_len] = (uint8) val;
	priv->buf_len++;
}

void _ggi_file_write_word(ggi_visual *vis, unsigned int val)
{
#ifdef GGI_BIG_ENDIAN
	val = GGI_BYTEREV16(val);
#endif
	_ggi_file_write_byte(vis, val >> 8);
	_ggi_file_write_byte(vis, val & 0xff);
}

void _ggi_file_write_string(ggi_visual *vis, unsigned char *str)
{
	for (; *str; str++) {
		_ggi_file_write_byte(vis, (unsigned char) *str);
	}
}

void _ggi_file_write_zeros(ggi_visual *vis, int count)
{
	while(count--) {
		_ggi_file_write_byte(vis, 0);
	}
}
