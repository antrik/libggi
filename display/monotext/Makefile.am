AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.monotext"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libmonotext_la_LIBADD = -lm

noinst_LTLIBRARIES = libmonotext.la

libmonotext_la_SOURCES = color.c draw.c mode.c monotext.c text.c visual.c

EXTRA_DIST = EXPSYMS font_data.h

MAINTAINERCLEANFILES = Makefile.in

