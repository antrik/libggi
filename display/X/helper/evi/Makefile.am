AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.X.evi"'
INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
libhelper_x_evi_la_LIBADD = $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)

noinst_LTLIBRARIES = libhelper_x_evi.la

libhelper_x_evi_la_SOURCES = evi.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

