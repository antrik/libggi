#!/bin/sh

TOOL=$1

have_version=`$TOOL --version | head -1 | tr -s '[A-Za-z() ]' ' '`
wanted_version=$2

have_version=`echo $have_version | tr '[A-Za-z]' '.'`
wanted_version=`echo $wanted_version | tr '[A-Za-z]' '.'`

# debug
##echo "have version: $have_version"
##echo "wanted version: $wanted_version"

# Test major version
have_major_version=`echo $have_version | cut -d"." -f1`
wanted_major_version=`echo $wanted_version | cut -d"." -f1`

# debug
##echo "have_major_version: $have_major_version"
##echo "wanted_major_version: $wanted_major_version"

if test $have_major_version -lt $wanted_major_version; then
	echo "$TOOL $wanted_version is required"
	exit 63
fi

# Test minor version
have_minor_version=`echo $have_version | cut -d"." -f2`
wanted_minor_version=`echo $wanted_version | cut -d"." -f2`

# debug
##echo "have_minor_version: $have_minor_version"
##echo "wanted_minor_version: $wanted_minor_version"

if test $have_minor_version -lt $wanted_minor_version; then
	echo "$TOOL $wanted_version is required"
	exit 63
fi

# Test patch version
have_patch_version=`echo $have_version | cut -d"." -f3`
wanted_patch_version=`echo $wanted_version | cut -d"." -f3`

# debug
##echo "have_patch_version: $have_patch_version"
##echo "wanted_patch_version: $wanted_patch_version"

test -z $have_patch_version && exit 0
test -z $wanted_patch_version && exit 0

if test $have_patch_version -lt $wanted_patch_version; then
	echo "$TOOL $wanted_version is required"
	exit 63
fi

exit 0
