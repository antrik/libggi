#!/bin/sh

touch ChangeLog

cat m4/*.m4 > acinclude.m4
aclocal
autoheader
automake --add-missing --verbose
autoconf
