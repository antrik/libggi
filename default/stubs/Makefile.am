AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
stubs_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
stubs_la_LDFLAGS = $(extra_libraries) -module -no-undefined \
		-avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = stubs.la

stubs_la_SOURCES = box.c copybox.c crossblit.c fillscreen.c gtext.c hline.c \
	 line.c pixel.c visual.c vline.c 

sublibdir = $(libdir)/$(ggi_subdir)/default

EXTRA_DIST = EXPSYMS stublib.h
