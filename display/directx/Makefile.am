AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.directx"'
INCLUDES = -I$(top_srcdir)/include
libdirectx_la_LIBADD = -lgg -lddraw -lgdi32 -luser32

noinst_LTLIBRARIES = libdirectx.la

libdirectx_la_SOURCES = ddinit.c mode.c visual.c dxguid.c

EXTRA_DIST = EXPSYMS cursor.inc ddinit.h

MAINTAINERCLEANFILES = Makefile.in

