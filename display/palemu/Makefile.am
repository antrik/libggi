AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.palemu"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
palemu_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg
palemu_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = palemu.la

palemu_la_SOURCES = color.c draw.c frames.c mode.c palemu.c text.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

