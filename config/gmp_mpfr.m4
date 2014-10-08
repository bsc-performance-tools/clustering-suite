# SYNOPSIS
#
#   AX_LIB_MPFR([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the MPFR library
#
#   By using the "--with-mpfr=" option, define a special installation
#   directory. If MPFR is not found there, the script will fail immediately.
#   Otherwise, $MPFR_HOME is searched, then standard system locations.
#
#   This macro calls:
#
#     AC_SUBST(MPFR_CPPFLAGS)
#     AC_SUBST(MPFR_LIBSDIR)
#     AC_SUBST(MPFR_LIBS)
#
#   And sets:
#
#     HAVE_MPFR
#
AC_DEFUN([AX_LIB_MPFR],[

AC_ARG_WITH(
  [mpfr],
  AS_HELP_STRING([--with-mpfr=PATH],
                 [specify prefix directory for installed MPFR package.]),
  [ac_mpfr_dirs="$withval"],
  [ac_mpfr_dirs="$MPFR_HOME"' /usr /usr/local /opt /opt/local'])

AC_MSG_CHECKING([whether MPFR is available...])

MPFR_CPPFLAGS=
MPFR_LIBSDIR=
MPFR_LIBS="-lmpfr"

ac_mpfr="no"

for ac_mpfr_iterate in $ac_mpfr_dirs; do

  if test -e $ac_mpfr_iterate/include/mpfr.h -a -e $ac_mpfr_iterate/lib/libmpfr.so; then
    MPFR_CPPFLAGS=-I$ac_mpfr_iterate/include
    MPFR_LIBSDIR=$ac_mpfr_iterate/lib
    ac_mpfr="yes"
    break
  fi

  if test -e $ac_mpfr_iterate/include/mpfr.h -a -e $ac_mpfr_iterate/lib/i386-linux-gnu/libmpfr.so; then
    MPFR_CPPFLAGS=-I$ac_mpfr_iterate/include
    MPFR_LIBSDIR=$ac_mpfr_iteratee/lib/i386-linux-gnu
    ac_mpfr="yes"
    break
  fi
done

if test "$ac_mpfr" = "yes"; then
  AC_MSG_RESULT([yes])
  AC_DEFINE(HAVE_MPFR, [1], [Indicates the presence of MPFR])
  AC_SUBST(MPFR_CPPFLAGS)
  AC_SUBST(MPFR_LIBSDIR)
  AC_SUBST(MPFR_LIBS)
  # execute ACTION-IF-FOUND
  ifelse([$1], , :, [$1])
else
  AC_MSG_RESULT([no])
  # execute ACTION-IF-NOT-FOUND
  ifelse([$2], , :, [$2])
fi

])

# SYNOPSIS
#
#   AX_LIB_GMP([ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
#
# DESCRIPTION
#
#   Test for the GMP library
#
#   By using the "--with-mpfr=" option, define a special installation
#   directory. If MPFR is not found there, the script will fail immediately.
#   Otherwise, $MPFR_HOME is searched, then standard system locations.
#   This macro calls:
#
#     AC_SUBST(GMP_CPPFLAGS)
#     AC_SUBST(GMP_LIBSDIR)
#     AC_SUBST(GMP_LIBS)
#
#   And sets:
#
#     HAVE_GMP

AC_DEFUN([AX_LIB_GMP],[

# Specify a location for gmp
AC_ARG_WITH(
  [gmp],
  AS_HELP_STRING([--with-gmp=PATH],
                 [specify prefix directory for the installed GMP package.]),
  [ac_gmp_dirs="$withval"],
  [ac_gmp_dirs="$GMP_HOME"' /usr /usr/local /opt /opt/local']
)

AC_MSG_CHECKING([whether GMP is available...])

GMP_CPPFLAGS=
GMP_LIBSDIR=
GMP_LIBS="-lgmp"
ac_gmp="no"

for ac_gmp_iterate in $ac_gmp_dirs; do

  if test -e $ac_gmp_iterate/include/gmp.h -a -e $ac_gmp_iterate/lib/libgmp.so; then
    GMP_CPPFLAGS=-I$ac_gmp_iterate/include
    GMP_LIBSDIR=$ac_gmp_iterate/lib
    ac_gmp="yes"
    break
  fi

  if test -e $ac_gmp_iterate/include/gmp.h -a -e $ac_gmp_iterate/lib/i386-linux-gnu/libgmp.so; then
    GMP_CPPFLAGS=-I$ac_gmp_iterate/include
    GMP_LIBSDIR=$ac_gmp_iterate/lib/i386-linux-gnu
    ac_gmp="yes"
    break
  fi
done


if test "$ac_gmp" = "yes"; then
  AC_MSG_RESULT([yes])
  AC_DEFINE(HAVE_GMP, [1], [Indicates the presence of GMP])
  AC_SUBST(GMP_CPPFLAGS)
  AC_SUBST(GMP_LIBSDIR)
  AC_SUBST(GMP_LIBS)
  # execute ACTION-IF-FOUND
  ifelse([$1], , :, [$1])
else
  AC_MSG_RESULT([no])
  # execute ACTION-IF-NOT-FOUND
  ifelse([$2], , :, [$2])
fi

])




