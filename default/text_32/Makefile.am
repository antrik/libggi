AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
text_32_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
text_32_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = text_32.la

text_32_la_SOURCES = color.c gtext.c pixel.c pixela.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS t32lib.h
