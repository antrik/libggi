INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la $(THREADLIBS)

SUBDIRS = warp-ggi

# cube3d is nice - we install it
EXTRA_PROGRAMS = cube3d wrap
bin_PROGRAMS = ${INSTDEMOS}

# findleaks.c showaccel2.c tunemode.c
noinst_PROGRAMS = demo flying_ggis pageflip saver slimy stars \
	textdemo $(OPTDEMOS)

# -lgg is needed for ggUSleep() and ggCurTime()
demo_LDADD =  ${top_builddir}/ggi/libggi.la -lgg ${THREADLIBS}
flying_ggis_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm ${THREADLIBS}
pageflip_LDADD =  ${top_builddir}/ggi/libggi.la -lgg -lm ${THREADLIBS}
saver_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm ${THREADLIBS}
slimy_LDADD = ${top_builddir}/ggi/libggi.la -lm ${THREADLIBS}
stars_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm ${THREADLIBS}
wrap_LDADD =  ${top_builddir}/ggi/libggi.la -lgg $(bsdsock_libs) ${THREADLIBS}
cube3d_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm ${THREADLIBS}
textdemo_LDADD =  ${top_builddir}/ggi/libggi.la -lgg ${THREADLIBS}

EXTRA_DIST =
