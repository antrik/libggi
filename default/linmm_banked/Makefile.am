AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
linmm_banked_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
linmm_banked_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = linmm_banked.la

linmm_banked_la_SOURCES = hline.c pixel.c visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS linmm_banked.h
