dnl Process this file with autoconf to produce a configure script.
dnl Created by Anjuta application wizard.

define([svnversion], "r"esyscmd([sh -c "svnversion -n . | cut -d: -f2 | cut -dS -f1 | tr -d '\n'"]))
AC_INIT(ClusteringSuite, [2.1-svnversion], tools@bsc.es)

AC_CONFIG_AUX_DIR(config)
AC_CANONICAL_SYSTEM

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)
AC_CONFIG_HEADERS([clustering_suite_config.h])
AM_MAINTAINER_MODE

AC_ISC_POSIX
AC_PROG_CXX
AM_PROG_CC_STDC
AC_HEADER_STDC

echo -n $VERSION>$srcdir/VERSION

AC_SUBST(PACKAGE)
AC_SUBST(VERSION)


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

dnl =========================================================================
dnl Check for libxml
dnl =========================================================================

AM_PATH_XML2(
  [2.5.0],
  [AC_DEFINE([HAVE_XML2], [1], [Defined if libxml2 exists])],
  [AC_MSG_ERROR(Cannot find xml2-config of libXML 2.5.0 (or above))]
)
AX_PROG_XML2()
AM_PROG_LIBTOOL

dnl =========================================================================
dnl COMMON_FILES_DIR: point the compiling system where the tools 'common-files'
dnl directory is located
dnl =========================================================================

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

dnl =========================================================================
dnl MUSTER_DIR: point the compiling system where the tools 'muster' includes
dnl an libs are located
dnl =========================================================================
AC_ARG_WITH(
  [muster],
  AS_HELP_STRING(
    [--with-muster=MUSTER_DIR],
    [sets the given directory as location of muster includes and libs (needs Boost libraries)]
  ),
  [
	muster_dir="$withval"

	dnl Check boost existence
	BOOST_REQUIRE(1.36)

  if test ! -d $muster_dir/include; then
    AC_MSG_ERROR([muster include directory not found, check --with-muster])
  fi
	
	if test ! -f $muster_dir/lib/libmuster.so; then
		AC_MSG_ERROR([muster dynamic library file not found, check --with-muster])
	fi

  MUSTER_CXXFLAGS="-I$muster_dir/include $BOOST_CPPFLAGS"
  MUSTER_CFLAGS="-I$muster_dir/include"
  MUSTER_LDFLAGS="-L$muster_dir/lib -R$muster_dir/lib"
  MUSTER_LIBS="-lmuster"

  AC_SUBST(MUSTER_CXXFLAGS)
  AC_SUBST(MUSTER_CFLAGS)
  AC_SUBST(MUSTER_LDFLAGS)
  AC_SUBST(MUSTER_LIBS)

	muster_enabled=yes
	AC_DEFINE(HAVE_MUSTER, 1, [Defined if MUSTER library is enabled])
  ],
  [
	muster_enabled=no
  ]
)
AM_CONDITIONAL(HAVE_MUSTER, test "x$muster_enabled" = "xyes")

dnl =========================================================================
dnl AX_PROG_MPI: set if distributed version must be compiled
dnl =========================================================================
AX_PROG_MPI()

dnl =========================================================================
dnl AX_CHECK_CGAL: set if hull models and mrnet version can be compiled
dnl =========================================================================
AX_CHECK_CGAL()

dnl =========================================================================
dnl AX_CHECK_SEQAN: check for Seqan installation to perform the iterative
dnl refinement
dnl =========================================================================
AX_CHECK_SEQAN()
if test "x${seqan_enabled}" = "xyes"; then
	CXXFLAGS="${CXXFLAGS} ${SEQAN_CPPFLAGS}"
	CFLAGS="${CFLAGS} ${SEQAN_CPPFLAGS}"
fi

dnl =========================================================================
dnl Enable the generation of the ClusteringSuite manual
dnl =========================================================================
AC_ARG_ENABLE(doc,
   AC_HELP_STRING(
      [--enable-doc],
      [Generates the documentation of this instrumentation package (disabled by default)]
   ),
   [enable_doc="${enableval}"],
   [enable_doc="no"]
)
AM_CONDITIONAL(GENERATE_DOCUMENTATION, test "${enable_doc}" = "yes" )

AC_CHECK_PROGS(latex,[latex elatex lambda],no)
AC_CHECK_PROGS(dvips,[dvips],no)
AC_CHECK_PROGS(dvipdf,[dvipdf],no)
AC_CHECK_PROGS(latex2html,[latex2html],no)

AM_CONDITIONAL(BUILD_DOCS_PS, test "${latex}" != "no" -a "${dvips}" != "no")
AM_CONDITIONAL(BUILD_DOCS_PDF, test "${latex}" != "no" -a "${dvipdf}" != "no")
AM_CONDITIONAL(BUILD_HTML_DOCS, test "${latex}" != "no" -a "${latex2html}" != "no")


AC_CONFIG_FILES([
Makefile
include/Makefile
src/Makefile
src/BasicClasses/Makefile
src/libParaverTraceParser/Makefile
src/libSharedComponents/Makefile
src/libANN/Makefile
src/libClustering/gmeans/Makefile
src/libClustering/Makefile
src/libTraceClustering/Makefile
src/ClusteringDataExtractor/Makefile
src/DBSCANParametersApproximation/Makefile
src/BurstClustering/Makefile
doc/Makefile
])

if test "${MPI_INSTALLED}" = "yes" -a "x$muster_enabled" = "xyes"; then
AC_CONFIG_FILES([src/MusterDistributedClustering/Makefile])
fi

if test "${cgal_enabled}" = "yes"; then
AC_CONFIG_FILES([
src/libDistributedClustering/Makefile
src/MRNetStub/Makefile
])
fi

CFLAGS="-DCLUSTERING_SUITE $CFLAGS"
CXXFLAGS="-DCLUSTERING_SUITE $CXXFLAGS"

AC_OUTPUT
AX_SHOW_CONFIGURATION

dnl echo "
dnl   $PACKAGE_NAME version $VERSION

dnl   Prefix....................: $prefix
dnl   Debug Build...............: $debug
dnl   C Compiler Flags..........: $CFLAGS
dnl   C++ Compiler Flags........: $CXXFLAGS $CPPFLAGS
dnl   Linker flags..............: $LDFLAGS $LIBS
dnl   'common-files' directory..: ${common_filesdir:-default}
dnl   MUSTER Clustering enabled.: ${muster_enabled}
dnl   MUSTER Library directory..: ${muster_dir:-not_available}
dnl   Boost includes............: ${BOOST_CPPFLAGS:-default}
dnl   Distributed version (MPI).: ${MPI_INSTALLED}
dnl"

