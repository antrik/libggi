INCLUDES = -I$(top_srcdir)/include $(extra_includes)
m2164w_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
m2164w_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = m2164w.la

m2164w_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

sublibdir = $(libdir)/ggi/default/fbdev/mga

EXTRA_DIST = EXPSYMS m2164w.h mmio.h regs.h
