INCLUDES = -I${top_srcdir}/include
LDADD = ${top_builddir}/ggi/libggi.la

SUBDIRS = warp-ggi

# cube3d is nice - we install it
EXTRA_PROGRAMS = cube3d wrap
bin_PROGRAMS = ${INSTDEMOS}

# findleaks.c showaccel2.c tunemode.c
noinst_PROGRAMS = demo flying_ggis pageflip saver slimy stars \
	textdemo $(OPTDEMOS)

libggi = ${top_builddir}/ggi/libggi.la -lgii -lgg

# -lgg is needed for ggUSleep() and ggCurTime()
demo_LDADD =  ${libggi}
flying_ggis_LDADD = ${libggi} -lm
pageflip_LDADD = ${libggi} -lm
saver_LDADD = ${libggi} -lm
slimy_LDADD = ${libggi} -lm
stars_LDADD = ${libggi} -lm
wrap_LDADD = ${libggi} $(bsdsock_libs)
cube3d_LDADD = ${libggi} -lm
textdemo_LDADD = ${libggi}

EXTRA_DIST = cube3d.txt demo.txt banner.inc.c

MAINTAINERCLEANFILES = Makefile.in

