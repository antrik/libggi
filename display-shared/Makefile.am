displaydir=$(top_builddir)/display
displaylibdir = ${libdir}/${ggi_subdir}/display

EXTRA_LTLIBRARIES = aa.la auto.la directx.la fbdev.la file.la \
	glide.la helper_x_dbe.la helper_x_dga.la helper_x_evi.la \
	helper_x_shm.la helper_x_vidmode.la ipc.la kgi.la lcd823.la \
	kgilib.la linvtsw.la mansync.la memory.la monotext.la multi.la \
	palemu.la sub.la svgalib.la tele.la terminfo.la tile.la trueemu.la \
	suidkgi.la vcsa.la vgagl.la vgl.la wsfb.la quartz.la X.la

displaylib_LTLIBRARIES = @DISPLAYMODULES@

libggi = ${top_builddir}/ggi/libggi.la

aa_la_LIBADD = $(top_builddir)/display/aa/libaa.la ${libggi}
aa_la_SOURCES =
aa_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/aa/EXPSYMS

auto_la_LIBADD = $(top_builddir)/display/auto/libauto.la ${libggi}
auto_la_SOURCES =
auto_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/auto/EXPSYMS

directx_la_LIBADD = $(top_builddir)/display/directx/libdirectx.la ${libggi}
directx_la_SOURCES =
directx_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/directx/EXPSYMS

fbdev_la_LIBADD = $(top_builddir)/display/fbdev/libfbdev.la ${libggi}
fbdev_la_SOURCES =
fbdev_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/fbdev/EXPSYMS

file_la_LIBADD = $(top_builddir)/display/file/libfile.la ${libggi}
file_la_SOURCES =
file_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/file/EXPSYMS

glide_la_LIBADD = $(top_builddir)/display/glide/libglide.la ${libggi}
glide_la_SOURCES =
glide_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/glide/EXPSYMS

ipc_la_LIBADD = $(top_builddir)/display/ipc/libipc.la ${libggi}
ipc_la_SOURCES =
ipc_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/ipc/EXPSYMS

kgi_la_LIBADD = $(top_builddir)/display/kgi/libkgi.la ${libggi}
kgi_la_SOURCES =
kgi_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/kgi/EXPSYMS


lcd823_la_LIBADD = $(top_builddir)/display/lcd823/liblcd823.la ${libggi}
lcd823_la_SOURCES =
lcd823_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/lcd823/EXPSYMS

kgilib_la_LIBADD = $(top_builddir)/display/kgilib/libkgilib.la ${libggi}
kgilib_la_SOURCES =
kgilib_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/kgilib/EXPSYMS

linvtsw_la_LIBADD = $(top_builddir)/display/linvtsw/liblinvtsw.la ${libggi}
linvtsw_la_SOURCES =
linvtsw_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/linvtsw/EXPSYMS

mansync_la_LIBADD = $(top_builddir)/display/mansync/libmansync.la ${libggi}
mansync_la_SOURCES =
mansync_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/mansync/EXPSYMS

memory_la_LIBADD = $(top_builddir)/display/memory/libmemory.la ${libggi}
memory_la_SOURCES =
memory_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/memory/EXPSYMS

monotext_la_LIBADD = $(top_builddir)/display/monotext/libmonotext.la ${libggi}
monotext_la_SOURCES =
monotext_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/monotext/EXPSYMS

multi_la_LIBADD = $(top_builddir)/display/multi/libmulti.la ${libggi}
multi_la_SOURCES =
multi_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/multi/EXPSYMS

palemu_la_LIBADD = $(top_builddir)/display/palemu/libpalemu.la ${libggi}
palemu_la_SOURCES =
palemu_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/palemu/EXPSYMS

quartz_la_LIBADD = $(top_builddir)/display/quartz/libquartz.la ${libggi}
quartz_la_SOURCES =
quartz_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/quartz/EXPSYMS


sub_la_LIBADD = $(top_builddir)/display/sub/libsub.la ${libggi}
sub_la_SOURCES =
sub_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/sub/EXPSYMS

suidkgi_la_LIBADD = $(top_builddir)/display/suidkgi/libsuidkgi.la ${libggi}
suidkgi_la_SOURCES =
suidkgi_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/suidkgi/EXPSYMS

svgalib_la_LIBADD = $(top_builddir)/display/svgalib/libsvgalib.la ${libggi}
svgalib_la_SOURCES =
svgalib_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/svgalib/EXPSYMS

tele_la_LIBADD = $(top_builddir)/display/tele/libtele.la ${libggi}
tele_la_SOURCES =
tele_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/tele/EXPSYMS

terminfo_la_LIBADD = $(top_builddir)/display/terminfo/libterminfo.la ${libggi}
terminfo_la_SOURCES =
terminfo_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/terminfo/EXPSYMS

tile_la_LIBADD = $(top_builddir)/display/tile/libtile.la ${libggi}
tile_la_SOURCES =
tile_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/tile/EXPSYMS

trueemu_la_LIBADD = $(top_builddir)/display/trueemu/libtrueemu.la ${libggi}
trueemu_la_SOURCES =
trueemu_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/trueemu/EXPSYMS

vcsa_la_LIBADD = $(top_builddir)/display/vcsa/libvcsa.la ${libggi}
vcsa_la_SOURCES =
vcsa_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/vcsa/EXPSYMS

vgagl_la_LIBADD = $(top_builddir)/display/vgagl/libvgagl.la ${libggi}
vgagl_la_SOURCES =
vgagl_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/vgagl/EXPSYMS

vgl_la_LIBADD = $(top_builddir)/display/vgl/libvgl.la ${libggi}
vgl_la_SOURCES =
vgl_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/vgl/EXPSYMS

wsfb_la_LIBADD = $(top_builddir)/display/wsfb/libwsfb.la ${libggi}
wsfb_la_SOURCES =
wsfb_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/wsfb/EXPSYMS

helper_x_dbe_la_LIBADD = $(top_builddir)/display/X/helper/dbe/libhelper_x_dbe.la ${libggi}
helper_x_dbe_la_SOURCES =
helper_x_dbe_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/helper/dbe/EXPSYMS

helper_x_dga_la_LIBADD = $(top_builddir)/display/X/helper/dga/libhelper_x_dga.la ${libggi}
helper_x_dga_la_SOURCES =
helper_x_dga_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/helper/dga/EXPSYMS

helper_x_evi_la_LIBADD = $(top_builddir)/display/X/helper/evi/libhelper_x_evi.la ${libggi}
helper_x_evi_la_SOURCES =
helper_x_evi_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/helper/evi/EXPSYMS

helper_x_shm_la_LIBADD = $(top_builddir)/display/X/helper/shm/libhelper_x_shm.la ${libggi}
helper_x_shm_la_SOURCES =
helper_x_shm_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/helper/shm/EXPSYMS

helper_x_vidmode_la_LIBADD = $(top_builddir)/display/X/helper/vidmode/libhelper_x_vidmode.la ${libggi}
helper_x_vidmode_la_SOURCES =
helper_x_vidmode_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/helper/vidmode/EXPSYMS

X_la_LIBADD = $(top_builddir)/display/X/libx.la ${libggi}
X_la_SOURCES =
X_la_LDFLAGS = -rpath $(displaylibdir) ${extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/display/X/EXPSYMS


DISTCLEANFILES = Makefile .deps
MAINTAINERCLEANFILES = Makefile.in
