INCLUDES = -I$(top_srcdir)/include $(extra_includes)
terminfo_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg $(terminfolibs)
terminfo_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = terminfo.la

terminfo_la_SOURCES = color.c input.c mode.c visual.c ncurses.c

sublibdir = $(libdir)/ggi/display

EXTRA_DIST = EXPSYMS TIvisual.h
