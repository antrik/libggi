INCLUDES = -I$(top_srcdir)/include $(extra_includes) $(X_CFLAGS)
xf86dga_la_LIBADD = $(top_builddir)/ggi/libggi.la -lgii -lgg \
	$(X_LIBS) -lX11 -lXext $(X_EXTRA_LIBS) $(x_helper_xf86dga_libs) $(x_helper_xf86vm_libs)
xf86dga_la_LDFLAGS = $(extra_libraries) -module -no-undefined -avoid-version \
	-export-symbols $(srcdir)/EXPSYMS

sublib_LTLIBRARIES = xf86dga.la

if INTERNAL_XF86DGA
dga_src = XF86DGA.c XF86VMode.c
else
dga_src = 
endif

xf86dga_la_SOURCES = accel.c color.c mode.c visual.c $(dga_src)

EXTRA_xf86dga_la_SOURCES = XF86DGA.c XF86VMode.c

sublibdir = $(libdir)/$(ggi_subdir)/display

EXTRA_DIST = EXPSYMS extutil.h xf86dga.h xf86dgastr.h
