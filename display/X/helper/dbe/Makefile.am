AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.X.dbe"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
libhelper_x_dbe_la_LIBADD = $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)

noinst_LTLIBRARIES = libhelper_x_dbe.la

libhelper_x_dbe_la_SOURCES = dbe.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

