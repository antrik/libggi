INCLUDES = -I$(top_srcdir)/include $(extra_includes)
directfb_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgg
directfb_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

directfbglobal_la_LIBADD = 
directfbglobal_la_LDFLAGS = -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS.global

sublib_LTLIBRARIES = directfb.la directfbglobal.la

directfb_la_SOURCES = box.c copybox.c crossblit.c gtext.c hline.c line.c \
	visual.c vline.c

directfbglobal_la_SOURCES = directfbglobal.c

sublibdir = $(libdir)/ggi/default/fbdev/directfb

EXTRA_DIST = EXPSYMS ggidirecfb.h




