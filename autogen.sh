#!/bin/sh

AC_VERSION="2.13"
AM_VERSION="1.5"

echo -n "Checking for required autoconf ${AC_VERSION}..."
./checkversion.sh autoconf $AC_VERSION
test $? -eq 0 || exit $? && echo "ok"
echo -n "Checking for required automake ${AM_VERSION}..."
./checkversion.sh automake $AM_VERSION
test $? -eq 0 || exit $? && echo "ok"


touch ChangeLog

cat m4/*.m4 > acinclude.m4
echo "Running aclocal..."
aclocal
echo "Running autoheader..."
autoheader
echo "Running automake..."
automake --add-missing
echo "Running autoconf..."
autoconf
