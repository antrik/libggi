AM_CFLAGS = -DPKG_NAME='"GGI:display:X"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
helper_x_dbe_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)
helper_x_dbe_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = helper_x_dbe.la

helper_x_dbe_la_SOURCES = dbe.c

sublibdir = $(libdir)/$(ggi_subdir)/helper

EXTRA_DIST = EXPSYMS
