AM_CPPFLAGS = -DDEBUG_NAMESPACE='"libggi.display.X.shm"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
libhelper_x_shm_la_LIBADD = $(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS)

noinst_LTLIBRARIES = libhelper_x_shm.la

libhelper_x_shm_la_SOURCES = shm.c

EXTRA_DIST = EXPSYMS

MAINTAINERCLEANFILES = Makefile.in

