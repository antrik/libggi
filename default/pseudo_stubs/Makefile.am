AM_CFLAGS = -DPKG_NAME='"LibGGI:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
pseudo_stubs_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = pseudo_stubs.la

pseudo_stubs_la_SOURCES = visual.c

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS
