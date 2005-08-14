AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.X.vidmode"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
helper_x_vidmode_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) $(x_helper_xf86vm_libs) -lX11 -lXext $(X_EXTRA_LIBS)
helper_x_vidmode_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
	-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = helper_x_vidmode.la

helper_x_vidmode_la_SOURCES = vidmode.c

sublibdir = $(libdir)/$(ggi_subdir)/helper

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

