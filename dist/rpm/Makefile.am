# Process this file with automake to create Makefile.in

# Normally a file to be included in the distribution would be
# added to EXTRA_DIST.  libgii.spec, however, is a generated
# file that must be in the distribution already generated.
# This fails when building with VPATH because the generated
# file is put in $(top_builddir)/$(subdir) and EXTRA_DIST
# files are only copied from $(srcdir).
DIST_GENERATED = libggi.spec

dist-hook: $(DIST_GENERATED)
	@for f in $(DIST_GENERATED); do \
		cp $$f $(distdir) || exit 1 ; \
	done

MAINTAINERCLEANFILES = Makefile.in

