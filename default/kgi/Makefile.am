INCLUDES = -I$(top_srcdir)/include $(extra_includes)
genkgi_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
genkgi_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = genkgi.la

genkgi_la_SOURCES = box.c copybox.c line.c visual.c

sublibdir = $(libdir)/$(ggi_subdir)/default/fbdev/kgi

EXTRA_DIST = EXPSYMS
