dnl Check CC specific characteristics

dnl $1 is the $CC option to check for.
dnl $2 is a boolean value (1 for true or 0 for false),
dnl    whether the option should be activated in
dnl    CFLAGS if available
dnl    Defaults to true, if not specified

AC_DEFUN([GGI_CC_CHECK4_OPTION],
[

AC_MSG_CHECKING([if $CC has -$1 option])
save_CFLAGS="$CFLAGS"

CFLAGS="$CFLAGS -$1"
AC_COMPILE_IFELSE([
	AC_LANG_PROGRAM([[#include <stdio.h>]],
			[[printf("Hello World\n");]])
	], [cc_tmp=yes],
	[cc_tmp=no])

activate=1
if test -n "$2"; then
	activate=$2
fi

if test $activate -eq 0 -o "$cc_tmp" != "yes"; then
	CFLAGS="$save_CFLAGS"
fi


AC_MSG_RESULT($cc_tmp)

AS_VAR_SET(AS_TR_SH(m4_tolower([cc_has_$1])), $cc_tmp)

])



dnl Check if -Werror-implicit-function-declaration kills the build.

dnl AIX 4.2 is missing a prototype for (at least) snprintf.
dnl Cygwin 1.5.11 is missing a prototype for (at least) siginterrupt.
dnl HP/UX 11.11 is missing a prototype for (at least) strtoull.
dnl True64 (version?) is missing a prototype for (at least) strtoull.

AC_DEFUN([GGI_CHECK_IMPLICIT_DECLARATIONS],
[

dnl First check, if we _have_ -Werror-implicit-function-declaration
GGI_CC_CHECK4_OPTION([Werror-implicit-function-declaration], 0)

if test "$cc_has_werror_implicit_function_declaration" = "yes"; then
	ggi_test_funcs="strncasecmp snprintf siginterrupt strtoull"

	dnl Now check for some headers ...
	AC_CHECK_HEADERS([string.h strings.h signal.h])

	dnl ... for the declarations
	for func in $ggi_test_funcs; do
		AC_CHECK_DECLS($func,[],[],
			[$ac_includes_default
			 #ifdef HAVE_SIGNAL_H
			 #include <signal.h>
			 #endif])
	done

	dnl ... and for the symbols.
	AC_CHECK_FUNCS($ggi_test_funcs)

	AC_MSG_CHECKING([if the system is missing declarations])

	for func in $ggi_test_funcs; do
		decl_var=AS_TR_SH([ac_cv_have_decl_$func])
		func_var=AS_TR_SH([ac_cv_func_$func])

		if eval "test \"x\${$decl_var}\" = \"xno\" -a \
			      \"x\${$func_var}\" = \"xyes\""; then
		  cc_has_werror_implicit_function_declaration="no"
		fi
	done

	dnl When we can use -Werror-implicit-function-declaration, then
	dnl there are no declarations missing. Thus the result output
	dnl must be inverted!
	if test $cc_has_werror_implicit_function_declaration = "yes"; then
		AC_MSG_RESULT(no)
	else
		AC_MSG_RESULT(yes)
	fi
fi

])
