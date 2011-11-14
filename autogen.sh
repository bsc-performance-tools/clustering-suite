#!/bin/sh
MAJOR=2
MINOR=1
sh -c "svnversion -n . | cut -d: -f2 | cut -dS -f1 | tr -d '\n'" > ./SVN_VERSION
SVN_VERSION=`cat ./SVN_VERSION`
VERSION="$MAJOR.$MINOR-$SVN_VERSION"
echo "Current version is $VERSION"
sed s/@@VERSION_NUMBER@@/$VERSION/ < configure.ac.template > configure.ac
set -x
libtoolize --automake --force --copy \
&& aclocal -I config \
&& autoheader \
&& automake --gnu --add-missing --copy \
&& autoconf

