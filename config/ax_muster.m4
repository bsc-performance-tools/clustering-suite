AC_DEFUN([AX_MUSTER],
[
  AC_ARG_WITH(
    [muster],
    AS_HELP_STRING(
      [--with-muster=MUSTER_DIR],
      [sets the given directory as location of muster includes and libs (needs Boost libraries)]
    ),
    [
	  muster_dir="$withval"

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
    MUSTER_LIBDIR="$muster_dir/lib"

    AC_SUBST(MUSTER_CXXFLAGS)
    AC_SUBST(MUSTER_CFLAGS)
    AC_SUBST(MUSTER_LDFLAGS)
    AC_SUBST(MUSTER_LIBS)
    AC_SUBST(MUSTER_LIBDIR)

	  muster_enabled=yes
	  AC_DEFINE(HAVE_MUSTER, 1, [Defined if MUSTER library is enabled])
    ],
    [
	  muster_enabled=no
    ]
  )
])
