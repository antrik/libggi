INCLUDES = -DBUILDING_LIBGGI -I$(top_srcdir)/include $(extra_includes)
libggi_la_LIBADD = -lgii -lgg

lib_LTLIBRARIES = libggi.la

libggi_la_SOURCES = colormap.c db.c dl.c events.c gc.c init.c internal.c \
	mode.c probe.c stubs.c unix.c visual.c
libggi_la_LDFLAGS = $(extra_libraries) -version-info 2:3:0 -no-undefined \
	-export-symbols $(srcdir)/EXPSYMS

EXTRA_DIST = EXPSYMS
