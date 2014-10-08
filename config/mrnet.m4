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


  # CGAL has been checked before 
  # AX_CHECK_CGAL()

  CFLAGS="-frounding-math $CFLAGS"
  CXXFLAGS="-frounding-math $CXXFLAGS"
  #CGAL_DYNAMIC_PATH="${MPFR_LIBDIR}:${GMP_LIBDIR}:${CGAL_LIBDIR}:${BOOST_THREAD_LDFLAGS}"
  #CLUSTERING_RPATH="${CLUSTERING_RPATH} ${CGAL_RPATH}"

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

  AX_FLAGS_RESTORE()
  AC_LANG_RESTORE()

  if test "${MRNETAPP_INSTALLED}" = "yes" ; then
    MRNETAPP_CONFIG="${MRNETAPP_HOME}/bin/mrnapp-config"
    MRNAPP_CONFIG="${MRNETAPP_CONFIG}"
    
    MRNETAPP_CP_CPPFLAGS=`${MRNETAPP_HOME}/bin/mrnapp-config --cp-cflags`
    MRNETAPP_FE_CPPFLAGS=`${MRNETAPP_HOME}/bin/mrnapp-config --fe-cflags`
    MRNETAPP_BE_CPPFLAGS=`${MRNETAPP_HOME}/bin/mrnapp-config --be-cflags`
    MRNETAPP_LIBSDIR=`${MRNETAPP_HOME}/bin/mrnapp-config --libdir`
    MRNETAPP_FE_LIBS=`${MRNETAPP_HOME}/bin/mrnapp-config --fe-libs`
    MRNETAPP_BE_LIBS=`${MRNETAPP_HOME}/bin/mrnapp-config --be-libs`
    MRNETAPP_LIBTOOL_RPATH=`${MRNETAPP_HOME}/bin/mrnapp-config --libtool-rpath`
    
    AC_SUBST(MRNETAPP_CONFIG)
    AC_SUBST(MRNAPP_CONFIG)
    AC_SUBST(MRNETAPP_CP_CPPFLAGS)
    AC_SUBST(MRNETAPP_FE_CPPFLAGS)
    AC_SUBST(MRNETAPP_BE_CPPFLAGS)
    AC_SUBST(MRNETAPP_LIBSDIR)
    AC_SUBST(MRNETAPP_FE_LIBS)
    AC_SUBST(MRNETAPP_BE_LIBS)
    AC_SUBST(MRNETAPP_LIBTOOL_RPATH)
    
    
    # execute ACTION-IF-FOUND
    ifelse([$1], , :, [$1])
  else
    # execute ACTION-IF-NOT-FOUND
    ifelse([$2], , :, [$2])
  fi

])


