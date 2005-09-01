defaultdir=$(top_builddir)/default
defaultlibdir = ${libdir}/${ggi_subdir}/default

EXTRA_LTLIBRARIES = color.la fbdev_directfb.la fbdev_directfbglobal.la \
	fbdev_m2164w.la fbdev_mach64.la fbdev_mga_g400.la \
	ilbm.la ioctl.la iplanar_2p.la kgi_Gx00.la kgi_mach64.la \
	kgi_radeon.la linear_1.la linear_16.la linear_1_r.la \
	linear_2.la linear_24.la linear_32.la linear_4.la \
	linear_4_r.la linear_8.la linmm_banked.la planar.la \
	pseudo_stubs.la ramdac.la stubs.la text_16.la text_32.la

defaultlib_LTLIBRARIES = @DEFAULTMODULES@

libggi = ${top_builddir}/ggi/libggi.la


color_la_LIBADD = $(top_builddir)/default/color/libcolor.la ${libggi}
color_la_SOURCES =
color_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/color/EXPSYMS

fbdev_mach64_la_LIBADD = $(top_builddir)/default/fbdev/ati/mach64/libmach64.la ${libggi}
fbdev_mach64_la_SOURCES =
fbdev_mach64_la_LDFLAGS = -rpath $(defaultlibdir)/fbdev/ati {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/fbdev/ati/mach64/EXPSYMS

fbdev_directfb_la_LIBADD = $(top_builddir)/default/fbdev/directfb/libdirectfb.la ${libggi}
fbdev_directfb_la_SOURCES =
fbdev_directfb_la_LDFLAGS = -rpath $(defaultlibdir)/fbdev/directfb {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/fbdev/directfb/EXPSYMS

fbdev_directfbglobal_la_LIBADD = $(top_builddir)/default/fbdev/directfbglobal/libdirectfbglobal.la ${libggi}
fbdev_directfbglobal_la_SOURCES =
fbdev_directfbglobal_la_LDFLAGS = -rpath $(defaultlibdir)/fbdev/directfbglobal {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/fbdev/directfbglobal/EXPSYMS

fbdev_m2164w_la_LIBADD = $(top_builddir)/default/fbdev/mga/m2164w/libm2164w.la ${libggi}
fbdev_m2164w_la_SOURCES =
fbdev_m2164w_la_LDFLAGS = -rpath $(defaultlibdir)/fbdev/mga {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/fbdev/mga/m2164w/EXPSYMS

fbdev_mga_g400_la_LIBADD = $(top_builddir)/default/fbdev/mga/g400/libmga_g400.la ${libggi}
fbdev_mga_g400_la_SOURCES =
fbdev_mga_g400_la_LDFLAGS = -rpath $(defaultlibdir)/fbdev/mga {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/fbdev/mga/g400/EXPSYMS

ilbm_la_LIBADD = $(top_builddir)/default/ilbm/libilbm.la ${libggi}
ilbm_la_SOURCES =
ilbm_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/ilbm/EXPSYMS

ioctl_la_LIBADD = $(top_builddir)/default/ioctl/libioctl.la ${libggi}
ioctl_la_SOURCES =
ioctl_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/ioctl/EXPSYMS

iplanar_2p_la_LIBADD = $(top_builddir)/default/iplanar_2p/libiplanar_2p.la ${libggi}
iplanar_2p_la_SOURCES =
iplanar_2p_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/iplanar_2p/EXPSYMS

kgi_mach64_la_LIBADD = $(top_builddir)/default/kgi/ATI/Mach64/libmach64.la ${libggi}
kgi_mach64_la_SOURCES =
kgi_mach64_la_LDFLAGS = -rpath $(defaultlibdir)/kgi {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/kgi/ATI/Mach64/EXPSYMS

kgi_radeon_la_LIBADD = $(top_builddir)/default/kgi/ATI/Radeon/libradeon.la ${libggi}
kgi_radeon_la_SOURCES =
kgi_radeon_la_LDFLAGS = -rpath $(defaultlibdir)/kgi {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/kgi/ATI/Radeon/EXPSYMS

kgi_Gx00_la_LIBADD = $(top_builddir)/default/kgi/Matrox/Gx00/libGx00.la ${libggi}
kgi_Gx00_la_SOURCES =
kgi_Gx00_la_LDFLAGS = -rpath $(defaultlibdir)/kgi {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/kgi/Matrox/Gx00/EXPSYMS

linear_1_la_LIBADD = $(top_builddir)/default/linear_1/liblinear_1.la ${libggi}
linear_1_la_SOURCES =
linear_1_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_1/EXPSYMS

linear_16_la_LIBADD = $(top_builddir)/default/linear_16/liblinear_16.la ${libggi}
linear_16_la_SOURCES =
linear_16_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_16/EXPSYMS

linear_1_r_la_LIBADD = $(top_builddir)/default/linear_1_r/liblinear_1_r.la ${libggi}
linear_1_r_la_SOURCES =
linear_1_r_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_1_r/EXPSYMS

linear_2_la_LIBADD = $(top_builddir)/default/linear_2/liblinear_2.la ${libggi}
linear_2_la_SOURCES =
linear_2_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_2/EXPSYMS

linear_24_la_LIBADD = $(top_builddir)/default/linear_24/liblinear_24.la ${libggi}
linear_24_la_SOURCES =
linear_24_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_24/EXPSYMS

linear_32_la_LIBADD = $(top_builddir)/default/linear_32/liblinear_32.la ${libggi}
linear_32_la_SOURCES =
linear_32_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_32/EXPSYMS

linear_4_la_LIBADD = $(top_builddir)/default/linear_4/liblinear_4.la ${libggi}
linear_4_la_SOURCES =
linear_4_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_4/EXPSYMS

linear_4_r_la_LIBADD = $(top_builddir)/default/linear_4_r/liblinear_4_r.la ${libggi}
linear_4_r_la_SOURCES =
linear_4_r_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_4_r/EXPSYMS

linear_8_la_LIBADD = $(top_builddir)/default/linear_8/liblinear_8.la ${libggi}
linear_8_la_SOURCES =
linear_8_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linear_8/EXPSYMS

linmm_banked_la_LIBADD = $(top_builddir)/default/linmm_banked/liblinmm_banked.la ${libggi}
linmm_banked_la_SOURCES =
linmm_banked_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/linmm_banked/EXPSYMS

planar_la_LIBADD = $(top_builddir)/default/planar/libplanar.la ${libggi}
planar_la_SOURCES =
planar_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/planar/EXPSYMS

pseudo_stubs_la_LIBADD = $(top_builddir)/default/pseudo_stubs/libpseudo_stubs.la ${libggi}
pseudo_stubs_la_SOURCES =
pseudo_stubs_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/pseudo_stubs/EXPSYMS

ramdac_la_LIBADD = $(top_builddir)/default/ramdac/libramdac.la ${libggi}
ramdac_la_SOURCES =
ramdac_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/ramdac/EXPSYMS

stubs_la_LIBADD = $(top_builddir)/default/stubs/libstubs.la ${libggi}
stubs_la_SOURCES =
stubs_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/stubs/EXPSYMS

text_16_la_LIBADD = $(top_builddir)/default/text_16/libtext_16.la ${libggi}
text_16_la_SOURCES =
text_16_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/text_16/EXPSYMS

text_32_la_LIBADD = $(top_builddir)/default/text_32/libtext_32.la ${libggi}
text_32_la_SOURCES =
text_32_la_LDFLAGS = -rpath $(defaultlibdir) {extra_libraries} \
	-shared -module -no-undefined -avoid-version \
	-export-symbols ${top_srcdir}/default/text_32/EXPSYMS


DISTCLEANFILES = Makefile .deps
MAINTAINERCLEANFILES = Makefile.in
