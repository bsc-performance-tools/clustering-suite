AC_DEFUN([AX_LIBTOOLS],
[
  AC_ARG_WITH(
    [libtools],
    AS_HELP_STRING(
      [--with-libtools=LIBTOOLS_DIR],
      [sets the given directory as location of libtools includes and libs]
    ),

    [
	  libtools_dir="$withval"

    if test ! -d $libtools_dir/include; then
      AC_MSG_ERROR([libtools include directory not found, check --with-libtools])
    fi
	
	  if test ! -f $libtools_dir/lib/libparavertraceconfig.so; then
		  AC_MSG_ERROR([libtools dynamic library file not found, check --with-libtools])
	  fi

    LIBTOOLS_CXXFLAGS="-I$libtools_dir/include $BOOST_CPPFLAGS"
    LIBTOOLS_CFLAGS="-I$libtools_dir/include"
    LIBTOOLS_LDFLAGS="-L$libtools_dir/lib -R $libtools_dir/lib"
    LIBTOOLS_LIBS="-lparavertraceconfig -lparavertraceparser"
    LIBTOOLS_LIBSDIR="$libtools_dir/lib"

    AC_SUBST(LIBTOOLS_CXXFLAGS)
    AC_SUBST(LIBTOOLS_CFLAGS)
    AC_SUBST(LIBTOOLS_LDFLAGS)
    AC_SUBST(LIBTOOLS_LIBS)
    AC_SUBST(LIBTOOLS_LIBSDIR)

	  libtools_enabled=yes
	  AC_DEFINE(HAVE_LIBTOOLS, 1, [Defined if LIBTOOLS library is enabled])
    ],

    [
      AC_MSG_ERROR([libtools installation directory not set, please use --with-libtools])
    ]
  )
])


dnl AM_CONDITIONAL(HAVE_LIBTOOLS, test "x$libtools_enabled" = "xyes")


