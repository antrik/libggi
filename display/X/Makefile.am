AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.X"'
SUBDIRS = helper
DIST_SUBDIRS = helper
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
X_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 $(X_EXTRA_LIBS)
X_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = X.la

X_la_SOURCES = pixel.c hline.c vline.c box.c line.c fillscreen.c gtext.c \
	       visual.c mode.c buffer.c color.c misc.c 

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

