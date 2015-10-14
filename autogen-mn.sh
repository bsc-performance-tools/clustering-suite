#!/bin/sh
export PATH=/gpfs/apps/AUTOMAKE/1.10.2/bin/:/gpfs/apps/LIBTOOL/2.2.6a/bin:/gpfs/apps/AUTOCONF/2.63/bin:${PATH}

MAJOR=2
MINOR=6
#sh -c "svnversion -n . | cut -d: -f2 | cut -dS -f1 | tr -d '\n'" > ./SVN_VERSION
#SVN_VERSION=`cat ./SVN_VERSION`
VERSION="$MAJOR.$MINOR.MN-`date "+%Y%m%d"`"

if test -e ./configure.ac; then
  rm ./configure.ac
fi

echo "Current version is $VERSION"

echo "#*********************************************************************" >> configure.ac
echo "#'configure.ac.' automatically generated by 'autogen.sh' do not modify" >> configure.ac
echo "#*********************************************************************" >> configure.ac
echo "" >> configure.ac

sed s/@@VERSION_NUMBER@@/$VERSION/ < configure.ac.template >> configure.ac

LIBTOOL_M4=/gpfs/apps/LIBTOOL/2.2.6a/share/aclocal
XML_M4=/usr/share/aclocal

set -x
libtoolize --automake --force --copy \
&& aclocal -I config -I ${LIBTOOL_M4} -I ${XML_M4} \
&& autoheader \
&& automake --gnu --add-missing --copy \
&& autoconf

