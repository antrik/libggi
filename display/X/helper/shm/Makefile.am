AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.X"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
helper_x_shm_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)
helper_x_shm_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = helper_x_shm.la

helper_x_shm_la_SOURCES = shm.c

sublibdir = $(libdir)/$(ggi_subdir)/helper

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

