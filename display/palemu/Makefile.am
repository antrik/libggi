AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.palemu"'
INCLUDES = -I$(top_srcdir)/include
libpalemu_la_LIBADD = -lgii -lgg

noinst_LTLIBRARIES = libpalemu.la

libpalemu_la_SOURCES = color.c draw.c frames.c mode.c palemu.c text.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

