INCLUDES = -I${top_srcdir}/include ${extra_includes}
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = warp-ggi

# cube3d is nice - we install it
EXTRA_PROGRAMS = cube3d wrap
bin_PROGRAMS = ${INSTDEMOS}

# findleaks.c showaccel2.c tunemode.c
noinst_PROGRAMS = demo flying_ggis pageflip saver slimy stars \
	textdemo $(OPTDEMOS)

# -lgg is needed for ggUSleep() and ggCurTime()
demo_LDADD =  ${top_builddir}/ggi/libggi.la -lgg
flying_ggis_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm
pageflip_LDADD =  ${top_builddir}/ggi/libggi.la -lgg -lm
saver_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm
slimy_LDADD = ${top_builddir}/ggi/libggi.la -lm
stars_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm
wrap_LDADD =  ${top_builddir}/ggi/libggi.la -lgg $(bsdsock_libs)
cube3d_LDADD = ${top_builddir}/ggi/libggi.la -lgg -lm
textdemo_LDADD =  ${top_builddir}/ggi/libggi.la -lgg

EXTRA_DIST =
