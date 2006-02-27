AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.directx"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libdirectx_la_LIBADD = -lddraw -lgdi32 -luser32

noinst_LTLIBRARIES = libdirectx.la

libdirectx_la_SOURCES = ddinit.c mode.c visual.c dxguid.c

EXTRA_DIST = EXPSYMS cursor.inc ddinit.h

MAINTAINERCLEANFILES = Makefile.in

