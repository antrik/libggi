INCLUDES = -I$(top_srcdir)/include $(extra_includes)
directfb_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
directfb_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = directfb.la

directfb_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

sublibdir = $(libdir)/ggi/default/fbdev/directfb

EXTRA_DIST = EXPSYMS directfb.h mmio.h regs.h
