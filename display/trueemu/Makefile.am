AM_CFLAGS = -DPKG_NAME='"GGI:display:trueemu"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
trueemu_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
trueemu_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = trueemu.la

trueemu_la_SOURCES = blit24.c blit32.c frames.c mode.c stubs.c trueemu.c \
	visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS genblit.c
