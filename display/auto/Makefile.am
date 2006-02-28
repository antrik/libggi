AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.auto"'

EXTRA_DIST =

INCLUDES = -I$(top_srcdir)/include
libauto_la_LIBADD =

noinst_LTLIBRARIES = libauto.la

include $(srcdir)/os/os.am
include $(srcdir)/target/target.am

libauto_la_SOURCES = probe.c visual.c $(os_src) $(target_src)

EXTRA_DIST += EXPSYMS target/target.h

MAINTAINERCLEANFILES = Makefile.in

