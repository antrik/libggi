#!/bin/sh

AC_VERSION="2.59"
AM_VERSION="1.9.2"

printf "Checking for required autoconf ${AC_VERSION}..."
./checkversion.sh autoconf $AC_VERSION
test $? -eq 0 || exit $? && printf "ok\n"
printf "Checking for required automake ${AM_VERSION}..."
./checkversion.sh automake $AM_VERSION
test $? -eq 0 || exit $? && printf "ok\n"


touch ChangeLog

cat m4/*.m4 > acinclude.m4
echo "Running aclocal..."
aclocal
echo "Running autoheader..."
autoheader
echo "Running automake..."
automake --add-missing
echo "Running autoconf - generating genlibtool..."
autoconf -o genlibtool genlibtool.in
echo "Running autoconf - generating configure..."
autoconf
