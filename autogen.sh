#!/bin/sh

version_check=yes

for opt in $@; do
  case "$opt" in
  --force)
    version_check=no
    ;;
  esac
done

if test $version_check = yes; then
  AC_VERSION="2.59"
  AM_VERSION="1.9.6"

  printf "Checking for required autoconf ${AC_VERSION}..."
  ./checkversion.sh autoconf $AC_VERSION
  test $? -eq 0 || exit $? && printf "ok\n"
  printf "Checking for required automake ${AM_VERSION}..."
  ./checkversion.sh automake $AM_VERSION
  test $? -eq 0 || exit $? && printf "ok\n"
fi

touch ChangeLog

echo "Running aclocal..."
aclocal -I m4
echo "Running autoheader..."
autoheader
echo "Running automake..."
automake --add-missing
echo "Running autoconf..."
autoconf
