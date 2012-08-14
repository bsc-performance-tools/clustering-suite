# AX_PROG_MRNETAPP
# ----------------
AC_DEFUN([AX_PROG_MRNETAPP],
[
  AX_FLAGS_SAVE()
  AC_LANG_SAVE()
  AC_LANG([C++])

  AC_ARG_WITH(mrnetapp,
    AC_HELP_STRING(
      [--with-mrnetapp@<:@=DIR@:>@],
      [specify where to find MRNetApp libraries and includes]
    ),
    [mrnetapp_paths="$withval"],
    [mrnetapp_paths="/home/bsc41/bsc41127/apps/MRNetApp"] dnl List of possible default paths
  )

  MRNETAPP_INSTALLED="no"

  dnl Search for MRNet installation
  AX_FIND_INSTALLATION([MRNETAPP], [$mrnetapp_paths], [mrnetapp])


  dnl Check first for CGAL
  AX_CHECK_CGAL()

  if test "x$cgal_enabled" = "xyes"; then
    CFLAGS="-frounding-math $CFLAGS"
    CXXFLAGS="-frounding-math $CXXFLAGS"
    CGAL_DYNAMIC_PATH="${gmp_libdir}:${mpfr_libdir}:${cgal_libdir}:${BOOST_LDPATH}"
    CLUSTERING_RPATH="${CLUSTERING_RPATH} ${CGAL_RPATH}"

    if test "$MRNETAPP_INSTALLED" = "yes" ; then
      dnl Check for headers
      AC_MSG_CHECKING([for MRNetApp.h presence])
      if test -e ${MRNETAPP_INCLUDES}/MRNetApp.h; then
        AC_MSG_RESULT([yes])
      else
        AC_MSG_RESULT([no])
        MRNETAPP_INSTALLED="no"
      fi
      
      dnl Check for libraries
      AC_MSG_CHECKING([for libmrnapp_frontend])

      if test -f ${MRNETAPP_LIBSDIR}/libmrnapp_frontend.a ; then
        MRNETAPP_FE_LIBS="-lmrnapp_frontend"
        AC_SUBST(MRNETAPP_FE_LIBS)
        AC_MSG_RESULT([yes])
      else
        MRNETAPP_INSTALLED="no"
        AC_MSG_RESULT([no])
      fi
      
      AC_MSG_CHECKING([for libmrnapp_backend])
      
      if test -f ${MRNETAPP_LIBSDIR}/libmrnapp_backend.a  ; then
        MRNETAPP_BE_LIBS="-lmrnapp_backend"
        AC_SUBST(MRNETAPP_BE_LIBS)
        AC_MSG_RESULT([yes])
      else
        MRNETAPP_INSTALLED="no"
        AC_MSG_RESULT([no])
      fi
    fi
	
    if test "${MRNETAPP_INSTALLED}" = "no" ; then
      AC_MSG_WARN()
      AC_MSG_WARN([MRNetApp was not found. TreeDBSCAN will not be built
  Please specify where to find MRNetApp libraries and includes
  with --with-mrnetapp
      ])
    else
      MRNETAPP_CONFIG="${MRNETAPP_HOME}/bin/mrnapp-config"
      MRNAPP_CONFIG="${MRNETAPP_CONFIG}"
		  AC_SUBST(MRNETAPP_CONFIG)
		  AC_SUBST(MRNAPP_CONFIG)
    fi
  fi

  AX_FLAGS_RESTORE()
  AC_LANG_RESTORE()

  AM_CONDITIONAL(HAVE_MRNETAPP, test "x$MRNETAPP_INSTALLED" = "xyes")
])


