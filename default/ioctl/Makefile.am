INCLUDES = -I$(top_srcdir)/include $(extra_includes)
ioctl_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
ioctl_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version -export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = ioctl.la

ioctl_la_SOURCES = box.c copybox.c line.c visual.c

sublibdir = $(libdir)/ggi/default

EXTRA_DIST = EXPSYMS ioctllib.h
