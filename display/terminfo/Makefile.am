AM_CPPFLAGS += -DDEBUG_NAMESPACE='"libggi.display.terminfo"'
INCLUDES = -I$(top_srcdir)/include
libterminfo_la_LIBADD = $(terminfolibs)

noinst_LTLIBRARIES = libterminfo.la

libterminfo_la_SOURCES = color.c mode.c visual.c ncurses.c

EXTRA_DIST = EXPSYMS TIvisual.h

MAINTAINERCLEANFILES = Makefile.in

