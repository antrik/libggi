AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.X.vidmode"'
INCLUDES = -I$(top_srcdir)/include $(X_CFLAGS)
libhelper_x_vidmode_la_LIBADD = $(X_LIBS) $(x_helper_xf86vm_libs) -lX11 -lXext $(X_EXTRA_LIBS)

noinst_LTLIBRARIES = libhelper_x_vidmode.la

libhelper_x_vidmode_la_SOURCES = vidmode.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

