INCLUDES = -I$(top_srcdir)/include $(extra_includes)
radeon_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
radeon_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = radeon.la

radeon_la_SOURCES = visual.c box.c line.c gc.c gtext.c crossblit.c

sublibdir = $(libdir)/$(ggi_subdir)/default/kgi

EXTRA_DIST = EXPSYMS