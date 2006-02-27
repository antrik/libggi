AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.default.fbdev.3dlabs.pm2"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libpm2_la_LIBADD =

noinst_LTLIBRARIES = libpm2.la

libpm2_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

EXTRA_DIST = EXPSYMS 3dlabs_pm2.h mmio.h regs.h

MAINTAINERCLEANFILES = Makefile.in

