/* $Id: patchlib.c,v 1.7 2007/11/07 10:26:31 pekberg Exp $
******************************************************************************

   Alter the path to the config file in binary libggi

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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif
#include <sys/stat.h>


static void print_usage(const char *fname)
{
        fprintf(stderr, "Usage: %s libggi.so-filename [new_path_to_patch_in]\n",
		fname);
        fprintf(stderr,
                "    Output will be located in a file named as the original\n"
                "    with \".patched\" appended, e.g. libggi.so.patched\n");
}

static char tag[] = GGIPATHTAG;

#define PATH_SPACE (256)

int main(int argc, const char *argv[])
{
        int size;
        int fd;
        struct stat st;
        char *solib;
        char *path;
        char *newname;
        int   offset=0;
        int   i;

        if (argc<2) {
                print_usage(argv[0]);
                exit(0);
        }
        fd=open(argv[1], O_RDWR);
        if ((fd=open(argv[1], O_RDWR)) < 0 ||
            fstat(fd, &st) != 0 ||
            (solib=malloc(size=st.st_size)) == NULL ||
            read(fd, solib, size) < size) {
                perror(argv[1]);
                exit(1);
        }
        close(fd);
        path=solib;
        while ((path=memchr(path, tag[0], size-offset)) != NULL) {
                if (memcmp(path, tag, strlen(tag)) == 0) {
                        path += GGITAGLEN;
                        offset = path-solib;
                        break;
                }
                path++;
                offset = path-solib;
        }

        if (path == NULL) {
                fprintf(stderr,
                        "Could not locate the path tag, bailing out...\n");
                exit(1);
       	}

        printf("Currently compiled in path (at %d) is :\n%s\n", offset, path);

        if (offset > size - PATH_SPACE) {
                fprintf(stderr,
                        "Not enough space after tag, bailing out...\n");
                exit(1);
        }

        for (i = 0; i < PATH_SPACE; ++i) {
                if (!path[i])
                        /* first nul found */
                        break;
        }
        for (; i < PATH_SPACE; ++i) {
                if (path[i]) {
                        /* ouch, found non-nul character */
                        fprintf(stderr,
                                "Non-empty tail after the existing path, "
                                "bailing out...\n");
                        exit(1);
                }
        }

        if (argc == 2) {
                fprintf(stderr, "No new path given. No new file generated.\n");
                exit(1);
        }
        strncpy(path, argv[2], PATH_SPACE);

        newname=malloc(strlen(argv[1]) + strlen(".patched") + 1);
        if (newname == NULL) {
                fprintf(stderr, "Memory allocation error\n");
                exit(1);
        }
        strcpy(newname, argv[1]);
        strcat(newname, ".patched");

        if ((fd=creat(newname, st.st_mode)) < 0 ||
             write(fd, solib, size) < size) {
                perror(newname);
                exit(1);
        }
        printf("New path (at %d) in %s is :\n%s\n", offset, newname, argv[2]);
        return 0;
}
