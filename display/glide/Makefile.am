INCLUDES = -I$(top_srcdir)/include $(extra_includes)
glide_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg -lglide2x -lm
glide_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = glide.la

glide_la_SOURCES = box.c fb.c gtext.c line.c mode.c pixel.c visual.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS
