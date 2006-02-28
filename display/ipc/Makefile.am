AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.ipc"'
INCLUDES = -I$(top_srcdir)/include
libipc_la_LIBADD = $(bsdsock_libs)

noinst_LTLIBRARIES = libipc.la

libipc_la_SOURCES = mode.c visual.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

