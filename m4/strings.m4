dnl Check for safe and secure string functions

AC_DEFUN([GGI_CHECK_STRING_FUNCS],
[

dnl strings.h exists on SYSV systems
dnl and contains strcasecmp() and strncasecmp()
AC_CHECK_HEADERS(string.h strings.h)

AC_CHECK_FUNCS([strcasecmp strncasecmp \
	strncat strncpy snprintf \
	strlcpy strlcat asprintf \
	strtol strtoul strtoll strtoull])

])
