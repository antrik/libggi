#!/bin/sh

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
