# ===========================================================================
#     http://www.gnu.org/software/autoconf-archive/ax_lib_cgal_core.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_LIB_CGAL_CORE([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the CGAL_Core library.
#
#   By using the "--with-cgal=" option, define a special installation
#   directory. If CGAL is not found there, the script will fail immediately.
#   Otherwise, $CGAL_HOME is searched, then standard system locations.
#
#   NOTE: This script depends on BOOST_CPPFLAGS, so be sure to run
#   AX_BOOST_BASE in advance.
#
#   This macro calls:
#
#     AC_SUBST(CGAL_CPPFLAGS)
#     AC_SUBST(CGAL_LIBSDIR)
#     AC_SUBST(CGAL_LIBS)
#
#   And sets:
#
#     HAVE_CGAL
#
# LICENSE
#
#   Copyright (c) 2010 Sebastian Hegler <sebastian.hegler@tu-dresden.de>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

#serial 2

AC_DEFUN([AX_LIB_CGAL_CORE],[

dnl guess from env, or use given value
AC_ARG_WITH([cgal],
            AS_HELP_STRING([--with-cgal@<:@=DIR@:>@],
                           [location of cgal installation, default $CGAL_HOME]),
            [ac_cgal_dirs="$withval"],
            [ac_cgal_dirs="$CGAL_HOME"' /usr /usr/local /opt /opt/local'])

dnl This test program is taken from:
dnl http://www.cgal.org/Manual/latest/examples/Convex_hull_2/vector_convex_hull_2.cpp
TEST_PROGRAM='
'

AC_LANG_PUSH([C++])

CPPFLAGS_SAVED="$CPPFLAGS"
LDFLAGS_SAVED="$LDFLAGS"
LIBS_SAVED="$LIBS"

for ac_cgal_iterate in $ac_cgal_dirs ; do

  CGAL_CPPFLAGS="-DNDEBUG -I$ac_cgal_iterate/include"
  CGAL_LDFLAGS="-L$ac_cgal_iterate/lib"
  CGAL_LIBS="-lCGAL -lCGAL_Core"

  CPPFLAGS="$CPPFLAGS_SAVED $CGAL_CPPFLAGS"
  LDFLAGS="$LDFLAGS_SAVED $CGAL_LDFLAGS"
  LIBS="$LIBS_SAVED $CGAL_LIBS"

  export CPPFLAGS
  export LDFLAGS
  export LIBS

  AC_MSG_CHECKING([whether CGAL is available in $ac_cgal_iterate])
  AC_LINK_IFELSE(
    [AC_LANG_PROGRAM(
      [
      [@%:@include <vector>]
      [@%:@include <CGAL/Exact_predicates_inexact_constructions_kernel.h>]
      [@%:@include <CGAL/convex_hull_2.h>]
      [typedef CGAL::Exact_predicates_inexact_constructions_kernel K;]
      [typedef K::Point_2 Point_2;]
      [typedef std::vector<Point_2> Points;]
      ],
      [
      [
      Points points, result;
      points.push_back(Point_2(0,0));
      points.push_back(Point_2(10,0));
      points.push_back(Point_2(10,10));
      points.push_back(Point_2(6,5));
      points.push_back(Point_2(4,1));
      CGAL::convex_hull_2(points.begin(),points.end(),std::back_inserter(result));
      //std::cout << result.size() << " points on the convex hull" << std::endl;
      ]
      ]
    )],
    [ac_cgal=yes],
    [ac_cgal=no])

  CPPFLAGS="$CPPFLAGS_SAVED"
  LDFLAGS="$LDFLAGS_SAVED"
  LIBS="$LIBS_SAVED"
  
  export LDFLAGS
  export CPPFLAGS
  export LIBS

  if test $ac_cgal = yes ; then
    CGAL_LIBSDIR="$ac_cgal_iterate/lib"
    AC_MSG_RESULT([yes])
    break
  else
    AC_MSG_RESULT([no])
  fi
done

AC_LANG_POP([C++])

if test "x$ac_cgal" = "xyes" ; then
  AC_DEFINE(HAVE_CGAL,[1],[Indicates presence of CGAL library])
  AC_SUBST(CGAL_CPPFLAGS)
  AC_SUBST(CGAL_LIBSDIR)
  AC_SUBST(CGAL_LIBS)
  # execute ACTION-IF-FOUND
  ifelse([$1], , :, [$1])
else
  # execute ACTION-IF-NOT-FOUND
  ifelse([$2], , :, [$2])
fi

])
