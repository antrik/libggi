AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.trueemu"'
INCLUDES = -I$(top_srcdir)/include
libtrueemu_la_LIBADD =

noinst_LTLIBRARIES = libtrueemu.la

libtrueemu_la_SOURCES = blit24.c blit32.c frames.c mode.c stubs.c trueemu.c \
	visual.c

EXTRA_DIST = EXPSYMS genblit.c

MAINTAINERCLEANFILES = Makefile.in

