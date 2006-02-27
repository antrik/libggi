AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.terminfo"'
INCLUDES = -I$(top_srcdir)/include $(extra_includes)
libterminfo_la_LIBADD = $(terminfolibs)

noinst_LTLIBRARIES = libterminfo.la

libterminfo_la_SOURCES = color.c input.c mode.c visual.c ncurses.c

EXTRA_DIST = EXPSYMS TIvisual.h

MAINTAINERCLEANFILES = Makefile.in

