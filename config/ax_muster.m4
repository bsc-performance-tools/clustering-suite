AC_DEFUN([AX_MUSTER],
[
  AC_ARG_WITH(
    [muster],
    AS_HELP_STRING(
      [--with-muster=MUSTER_DIR],
      [sets the given directory as location of muster includes and libs (needs Boost libraries)]
    ),
    [muster_dir="$withval"]
    [muster_dir="/usr)"]
  )

  ac_muster_installed="yes"

  if test ! -f $muster_dir/include/partition.h; then
    ac_muster_installed="no"
  fi

  if test ! -f $muster_dir/lib/libmuster.so; then
    ac_muster_installed="no"
  fi

  if test "x${ac_muster_enabled}" = "xyes"; then

    MUSTER_CPPFLAGS="-I$muster_dir/include"
    MUSTER_LDFLAGS="-L$muster_dir/lib -R$muster_dir/lib"
    MUSTER_LIBS="-lmuster"
    MUSTER_LIBSDIR="$muster_dir/lib"

    AC_SUBST(MUSTER_CPPFLAGS)
    AC_SUBST(MUSTER_LDFLAGS)
    AC_SUBST(MUSTER_LIBS)
    AC_SUBST(MUSTER_LIBSDIR)

    AC_DEFINE(HAVE_MUSTER, 1, [Defined if MUSTER library is enabled])

    # execute ACTION-IF-FOUND
    ifelse([$1], , :, [$1])
  else
    # execute ACTION-IF-NOT-FOUND
    ifelse([$2], , :, [$2])
  fi

])
