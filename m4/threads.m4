dnl Optional: Predefine $use_threads
dnl Return: $use_threads and $THREADLIB

AC_DEFUN([GGI_THREADLIBS],
[

dnl Set use_threads to autodetect (yes) if not already
dnl specified via configure
if test -z "$use_threads"; then
	use_threads="yes"
fi
THREADLIBS=""


dnl Check for pthread library

if test "$use_threads" = "yes" -o \
	"$use_threads" = "pthread"; then

AC_CHECK_HEADERS([pthread.h])
have_pthread=no
if test "$ac_cv_header_pthread_h" = "yes"; then
	TMP_SAVE_LIBS=$LIBS
	TMP_SAVE_CC=$CC
	CC="$SHELL ./libtool $CC"

	for pthreadlib in -lpthread -lc_r -pthread; do
		AC_MSG_CHECKING(for pthread_mutex_init with $pthreadlib)
		LIBS="$TMP_SAVE_LIBS $pthreadlib"

		AC_TRY_RUN([
			#define __C_ASM_H /* fix for retarded Digital Unix headers */
			#include <pthread.h>
			pthread_mutex_t mtex;
			int main(void)
			{
				if (pthread_mutex_init(&mtex, NULL) == 0) return 0;
				return 1;
			}
		],[
			AC_MSG_RESULT(yes)
			have_pthread=yes
			THREADLIBS="$pthreadlib"
			break
		],[
			AC_MSG_RESULT(no)
		])
	done

	LIBS=$TMP_SAVE_LIBS
	CC="$TMP_SAVE_CC"
fi

dnl Use pthread library if present
if test "$have_pthread" = "yes"; then
	use_threads="pthread"
elif test "$use_threads" = "pthread"; then
	use_threads="no"
fi

fi


dnl Check for OS specific thread support

if test "$use_threads" = "yes" -o \
	"$use_threads" = "win32"; then

dnl Win32 (after pthread since pthread is prefered on cygwin)
AC_CHECK_HEADERS([windows.h])
AC_MSG_CHECKING(for win32 threads)
if test "x$ac_cv_header_windows_h" = "xyes"; then
	AC_MSG_RESULT(yes)
	use_threads="win32"
	THREADLIBS=""
else
	AC_MSG_RESULT(no)
	if test "$use_threads" = "win32"; then
		use_threads="no"
	fi
fi

fi


dnl Fall back to not use threads, if threads were not found

if test "$use_threads" = "yes"; then
	use_threads="no"
fi

AC_SUBST(THREADLIBS)

])
