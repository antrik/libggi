#! /bin/sh

#*****************************************************************************
#
#  Generator script for builtin configuration
#
#  Copyright (C) 2006      Peter Ekberg          [peda@lysator.liu.se]
#
#  Permission is hereby granted, free of charge, to any person obtaining a
#  copy of this software and associated documentation files (the "Software"),
#  to deal in the Software without restriction, including without limitation
#  the rights to use, copy, modify, merge, publish, distribute, sublicense,
#  and/or sell copies of the Software, and to permit persons to whom the
#  Software is furnished to do so, subject to the following conditions:
#
#  The above copyright notice and this permission notice shall be included in
#  all copies or substantial portions of the Software.
#
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
#  THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
#  IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
#  CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
#*****************************************************************************

if test $# != 2; then
  echo "$0: Usage $0 root file" 1>&2
  exit 1
fi

if test -z $SED; then
  echo "$0: You need to set and export the SED variable" 1>&2
  exit 1
fi

quit=no

# assume that conf files do not have any "^" character in them
NL2ESC="tr \n ^"
ESC2NL="tr ^ \n"

sedroot=`echo "$1" | $SED -e 's,\\\\,/,g'`

conf=`cat "$1/$2" | $NL2ESC`

# support 20 levels of .includes
for count in 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20; do

  # generate sed script that includes the next level
  echo "$conf" | $ESC2NL | $SED -n -e '
# only care about .include lines
/^[.]include /{
h
# work out which file to include
s/^[.]include \([^ ]*\)/\1/
s^@ggi_sysconfdir@^'"$sedroot"'^
# assume location of target-specific confs and how they are named
s,/targets/\([^. ]*\)\.,/display/\1/\1.,
s/\.conf$/.conf.in/
x
# now work out how to find the correct line to replace
s,\([/.]\),\\\1,g
# now pattern is regexp to find line, and hold is file to include, so join
G
# print sed expr to remove the .include line and instead include the file
s/\(.*\)\n\(.*\)/\/^\1$\/{r \2\nd}/
p}
' > ./tmpgenconf.sed

  test -s "./tmpgenconf.sed" || quit=yes

  # include next level, if there is anything to include
  test $quit = yes || \
    conf=`echo "$conf" | $ESC2NL | $SED -f ./tmpgenconf.sed | $NL2ESC`

  rm -f ./tmpgenconf.sed
  test $quit = yes && break
done

# remove unwanted lines, condense whitespace, and format as C array of strings
echo "$conf" | $ESC2NL | $SED -e '
/^$/d
/^#/d
/^[.]/d
/@DLLEXT@/d
s/[ 	][ 	]*/ /g
s/\(["\\]\)/\\\1/g
s/\(.*\)/"\1",/
'
