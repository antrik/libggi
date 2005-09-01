AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.default"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libmga_g400_la_LIBADD =

noinst_LTLIBRARIES = libmga_g400.la

libmga_g400_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

EXTRA_DIST = EXPSYMS mga_g400.h mmio.h regs.h

MAINTAINERCLEANFILES = Makefile.in

