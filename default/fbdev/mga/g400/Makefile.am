AM_CFLAGS = -DPKG_NAME='"LibGGI:default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
mga_g400_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
mga_g400_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = mga_g400.la

mga_g400_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

sublibdir = $(libdir)/$(ggi_subdir)/default/fbdev/mga

EXTRA_DIST = EXPSYMS mga_g400.h mmio.h regs.h
