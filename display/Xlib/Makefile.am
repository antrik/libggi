INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
Xlib_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)
Xlib_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = Xlib.la

Xlib_la_SOURCES = box.c color.c fillscreen.c hline.c line.c mode.c pixel.c \
	visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS
