dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.


AC_INIT(ClusteringSuite, 1.99, tools@bsc.es)

AC_CONFIG_AUX_DIR(config)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

dnl ***************************************************************************
dnl Subversion Revision
dnl ***************************************************************************

SVNREV=`svnversion $srcdir | sed 's/:.*//'`
which svnversion > /dev/null
if test $? -ne 0  -o  "x$SVNREV" = "xexported";
  then SVNREV=`cat $srcdir/SVNREV`
  else echo -n $SVNREV>$srcdir/SVNREV
fi
AC_SUBST(SVNREV)


dnl Check if CFLAGS/CXXFLAGS have been defined by user
AC_MSG_CHECKING([whether configure should try to set CFLAGS])
if test "x$CFLAGS" = "x"; then
  enable_cflags_setting=yes
else
  enable_cflags_setting=no
fi
AC_MSG_RESULT($enable_cflags_setting)

AC_MSG_CHECKING([whether configure should try to set CXXFLAGS])
if test "x$CXXFLAGS" = "x"; then
  enable_cxxflags_setting=yes
else
  enable_cxxflags_setting=no
fi
AC_MSG_RESULT($enable_cxxflags_setting)

dnl =========================================================================
dnl Set default xxxFLAGS if not defined
dnl =========================================================================

if test "x$enable_cflags_setting" = "xyes"; then
  CFLAGS="-O3 -Os"
fi

if test "x$enable_cxxflags_setting" = "xyes"; then
  CXXFLAGS="-O3 -Os"
fi

dnl Check the compiler binary type

AX_SELECT_BINARY_TYPE
AX_OFF_T_64BIT


dnl Check the existence of the next data types and its size
AC_CHECK_TYPES([long, long long, char, int, float, long float, double, long double])
AC_CHECK_SIZEOF(long)
AC_CHECK_SIZEOF(long long)
AC_CHECK_SIZEOF(char)
AC_CHECK_SIZEOF(int)
AC_CHECK_SIZEOF(float)
AC_CHECK_SIZEOF(long float)
AC_CHECK_SIZEOF(double)
AC_CHECK_SIZEOF(long double)
AC_CHECK_TYPES([int64_t, uint64_t, int8_t, uint8_t, int32_t, uint32_t, size_t])

case "x$host_os" in
  xlinux*  ) Operating_System="linux" ;;
  xaix*    ) Operating_System="aix"   ;;
  xosf*    ) Operating_System="dec"   ;;
  xirix*   ) Operating_System="irix"  ;;
  xdarwin* ) Operating_System="macosx"   ;;
  xcygwin* ) Operating_System="cygwin";;
esac

AC_SUBST(Operating_System)

dnl ***********************************#
dnl ********Check for libxml***********#
dnl ***********************************#

AM_PATH_XML2(
  [2.5.0],
  [AC_DEFINE([HAVE_XML2], [1], [Defined if libxml2 exists])],
  [AC_MSG_ERROR(Cannot find xml2-config of libXML 2.5.0 (or above))]
)
AX_PROG_XML2
AM_PROG_LIBTOOL

CFLAGS="${CFLAGS} ${XML2_CFLAGS}"
CXXFLAGS="${CXXFLAGS} ${XML2_CXXFLAGS}"

dnl COMMON_FILES_DIR: point the compiling system where the tools 'common-files'
dnl directory is located

AC_ARG_WITH(
  [common-files-dir],
  AS_HELP_STRING(
    [--with-common-files-dir=LIB_DIR],
    [force given directory as BSC Performance Tools common-files location]
  ),
  [
  if test -d $withval; then
    common_filesdir="$withval"
  else
    AC_MSG_ERROR(--with-common-files-dir expected directory name)
  fi
  ],
  [
    if test -d ../common-files; then
      common_filesdir="`pwd`/../common-files"
    else
      AC_MSG_ERROR(
        Unable to find 'common-files' directory. Please point it using
        '--with-common-files-dir' option or copying it to the parent directory)
    fi
  ]
)
AC_SUBST(common_filesdir)


dnl src/BurstClustering/Makefile
dnl src/libANN/Makefile
dnl src/libClusteringAlgorithms/Makefile
dnl src/libClustering/Makefile
dnl src/libParaverTraceParser/Makefile

AC_OUTPUT([
Makefile
src/Makefile
src/BasicClasses/Makefile
src/libParaverTraceParser/Makefile
src/SharedComponents/Makefile
src/libANN/Makefile
src/libClustering/Makefile
src/DataExtraction/Makefile
src/libTraceClustering/Makefile
src/ClusteringDataExtractor/Makefile
src/BurstClustering/Makefile
])

echo "
  $PACKAGE_NAME version $PACKAGE_VERSION.$SVNREV

  Prefix.................: $prefix
  Debug Build............: $debug
  C Compiler Flags.......: $CFLAGS
  C++ Compiler Flags.....: $CXXFLAGS $CPPFLAGS
  Linker flags...........: $LDFLAGS $LIBS
  Common files directory.: ${common_filesdir:-default}
"

