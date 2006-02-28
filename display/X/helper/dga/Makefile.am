AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.X.dga"'
INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
libhelper_x_dga_la_LIBADD = $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS) $(x_helper_xf86dga_libs)

noinst_LTLIBRARIES = libhelper_x_dga.la

libhelper_x_dga_la_SOURCES = dga.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

