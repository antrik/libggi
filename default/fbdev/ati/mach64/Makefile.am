INCLUDES = -I$(top_srcdir)/include $(extra_includes)
mach64_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
mach64_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = mach64.la

mach64_la_SOURCES = box.c copybox.c crossblit.c gc.c gtext.c hline.c line.c \
	visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default/fbdev/ati

EXTRA_DIST = EXPSYMS mach64.h ati_mach64.h
