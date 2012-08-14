AC_DEFUN([AX_CHECK_MPFR_GMP],
[

# Check for GMP and MPFR
gmplibs="-lmpfr -lgmp"
gmpinc=
have_gmp=no
have_mpfr=no

# Specify a location for mpfr
# check for this first so it ends up on the link line before gmp.

AC_ARG_WITH(
  mpfr,
  [AS_HELP_STRING(
    [--with-mpfr=PATH],
		[specify prefix directory for installed MPFR package.])],
  [mpfr_paths="$withval"],
	[mpfr_paths="$MPFR_HOME"' /usr /usr/local /opt /opt/local']
)

for mpfr_iterate in $mpfr_paths; do

  if test -e $mpfr_iterate/include/mpfr.h -a -e $mpfr_iterate/lib/libmpfr.so; then
    mpfr_enabled="yes"
  fi

  if test -e $mpfr_iterate/include/mpfr.h -a -e $mpfr_iterate/lib/i386-linux-gnu/libmpfr.so; then
    mpfr_enabled="yes"
  fi

	AC_MSG_CHECKING([whether MPFR is available in $mpfr_iterate])

  if test "$mpfr_enabled" = "yes"; then
    mpfr_dir=$mpfr_iterate

    mpfr_libdir="$mpfr_dir/lib"
    AC_SUBST(mpfr_libdir)

    gmplibs="-L$mpfr_dir/lib $gmplibs"
    gmpinc="-I$mpfr_dir/include $gmpinc"

    have_mpfr=yes
    AC_MSG_RESULT([yes])
    break
  else
    AC_MSG_RESULT([no])
  fi

done

# Specify a location for gmp
AC_ARG_WITH(
  gmp,
  [AS_HELP_STRING(
    [--with-gmp=PATH],
    [specify prefix directory for the installed GMP package.]
  )],
  [gmp_paths="$withval"],
	[gmp_paths="$GMP_HOME"' /usr /usr/local /opt /opt/local']
)


for gmp_iterate in $gmp_paths; do

  if test -e $gmp_iterate/include/gmp.h -a -e $gmp_iterate/lib/libgmp.so; then
    gmp_enabled="yes"
  fi

  if test -e $gmp_iterate/include/gmp.h -a -e $gmp_iterate/lib/i386-linux-gnu/libgmp.so; then
    gmp_enabled="yes"
  fi

	AC_MSG_CHECKING([whether GMP is available in $gmp_iterate])

  if test "$gmp_enabled" = "yes"; then
    gmp_dir=$gmp_iterate
    
    gmp_libdir="$gmp_dir/lib"
    AC_SUBST(gmp_libdir)

    gmplibs="-L$gmp_dir/lib $gmplibs"
    gmpinc="-I$gmp_dir/include $gmpinc"

    have_gmp=yes
    AC_MSG_RESULT([yes])
    break
  else
    AC_MSG_RESULT([no])
  fi

done

if test x$have_mpfr != xyes -o x$have_gmp != xyes; then
    AC_MSG_WARN([GMP and MPFR libraries are needed to include CGAL.
Try the --with-gmp, --with-mpfr options to specify their locations. Source code
for these libraries can be found at their respective hosting sites. If you
obtained GMP/MPFR from a vendor distribution package, make sure that you
have installed both the libraries and the headerfiles.
They may be located in separate packages.])
fi

# Flags needed for both GMP, MPFR and/or MPC.
AC_SUBST(gmplibs)
AC_SUBST(gmpinc)

])

