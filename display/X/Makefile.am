INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
X_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)
X_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = X.la

X_la_SOURCES = visual.c mode.c color.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS color.inc mode.inc
