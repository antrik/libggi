dnl Find a relative path that takes us from static_sysconfdir/ggi_subdir
dnl to static_libdir/ggi_subdir.

dnl This macro is easily fooled with //, /./ and /../ combinations.
dnl Also, I'm very far from a script guru. This macro can probably
dnl be written in a couple of lines if you know the right script
dnl knobs.

AC_DEFUN([GGI_SYSCONF_TO_LIB],
[
rel_ggisysconfdir="$static_sysconfdir/$ggi_subdir"
rel_ggilibdir="$static_libdir/$ggi_subdir"
again="yes"
until test "$again" = "no"; do
	again=""
	newcdir=""
	newldir=""
	as_save_IFS=$IFS
	IFS='/'
	for cdir in $rel_ggisysconfdir; do
		IFS=$as_save_IFS
		if test -n "$again"; then
			if test -n "$newcdir"; then
				newcdir="$newcdir/$cdir"
			else
				newcdir="$cdir"
			fi
			continue
		fi
		as_save_IFS=$IFS
		IFS='/'
		for ldir in $rel_ggilibdir; do
			IFS=$as_save_IFS
			if test -z "$again"; then
				if test "$ldir" = "$cdir"; then
					again="yes"
					newldir=""
					newcdir=""
				else
					again="no"
					newldir="$ldir"
					newcdir="$cdir"
				fi
				continue
			fi
			if test -n "$newldir"; then
				newldir="$newldir/$ldir"
			else
				newldir="$ldir"
			fi
		done
	done
	rel_ggisysconfdir="$newcdir"
	rel_ggilibdir="$newldir"
done
as_save_IFS=$IFS
IFS='/'
for cdir in $rel_ggisysconfdir; do
	rel_ggilibdir="../$rel_ggilibdir"
done
IFS=$as_save_IFS

ggi_sysconfdir_to_libdir="$rel_ggilibdir"
])
