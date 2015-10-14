# AX_PROG_SYNAPSE
# ----------------
AC_DEFUN([AX_PROG_SYNAPSE],
[
  AX_FLAGS_SAVE()
  AC_LANG_SAVE()
  AC_LANG([C++])

  AC_ARG_WITH(synapse,
    AC_HELP_STRING(
      [--with-synapse@<:@=DIR@:>@],
      [specify where to find Synapse libraries and includes]
    ),
    [synapse_paths="$withval"],
    [synapse_paths="/home/bsc41/bsc41127/apps/synapse"] dnl List of possible default paths
  )

  SYNAPSE_INSTALLED="no"

  dnl Search for Synapse installation
  AX_FIND_INSTALLATION([SYNAPSE], [$synapse_paths], [synapse])


  # CGAL has been checked before 
  # AX_CHECK_CGAL()

  CFLAGS="-frounding-math $CFLAGS"
  CXXFLAGS="-frounding-math $CXXFLAGS"
  #CGAL_DYNAMIC_PATH="${MPFR_LIBDIR}:${GMP_LIBDIR}:${CGAL_LIBDIR}:${BOOST_THREAD_LDFLAGS}"
  #CLUSTERING_RPATH="${CLUSTERING_RPATH} ${CGAL_RPATH}"

  if test "$SYNAPSE_INSTALLED" = "yes" ; then
    dnl Check for headers
    AC_MSG_CHECKING([for MRNetApp.h presence])
    if test -e ${SYNAPSE_INCLUDES}/MRNetApp.h; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      SYNAPSE_INSTALLED="no"
    fi
    
    dnl Check for libraries
    AC_MSG_CHECKING([for libsynapse_frontend])

    if test -f ${SYNAPSE_LIBSDIR}/libsynapse_frontend.a ; then
      SYNAPSE_FE_LIBS="-lsynapse_frontend"
      AC_SUBST(SYNAPSE_FE_LIBS)
      AC_MSG_RESULT([yes])
    else
      SYNAPSE_INSTALLED="no"
      AC_MSG_RESULT([no])
    fi
    
    AC_MSG_CHECKING([for libsynapse_backend])
    
    if test -f ${SYNAPSE_LIBSDIR}/libsynapse_backend.a  ; then
      SYNAPSE_BE_LIBS="-lsynapse_backend"
      AC_SUBST(SYNAPSE_BE_LIBS)
      AC_MSG_RESULT([yes])
    else
      SYNAPSE_INSTALLED="no"
      AC_MSG_RESULT([no])
    fi
  fi

  AX_FLAGS_RESTORE()
  AC_LANG_RESTORE()

  if test "${SYNAPSE_INSTALLED}" = "yes" ; then
    SYNAPSE_CONFIG="${SYNAPSE_HOME}/bin/synapse-config"
    
    SYNAPSE_CP_CPPFLAGS=`${SYNAPSE_HOME}/bin/synapse-config --cp-cflags`
    SYNAPSE_FE_CPPFLAGS=`${SYNAPSE_HOME}/bin/synapse-config --fe-cflags`
    SYNAPSE_BE_CPPFLAGS=`${SYNAPSE_HOME}/bin/synapse-config --be-cflags`
    SYNAPSE_LIBSDIR=`${SYNAPSE_HOME}/bin/synapse-config --libdir`
    SYNAPSE_FE_LIBS=`${SYNAPSE_HOME}/bin/synapse-config --fe-libs`
    SYNAPSE_BE_LIBS=`${SYNAPSE_HOME}/bin/synapse-config --be-libs`
    SYNAPSE_LIBTOOL_RPATH=`${SYNAPSE_HOME}/bin/synapse-config --libtool-rpath`
    
    AC_SUBST(SYNAPSE_CONFIG)
    AC_SUBST(SYNAPSE_CP_CPPFLAGS)
    AC_SUBST(SYNAPSE_FE_CPPFLAGS)
    AC_SUBST(SYNAPSE_BE_CPPFLAGS)
    AC_SUBST(SYNAPSE_LIBSDIR)
    AC_SUBST(SYNAPSE_FE_LIBS)
    AC_SUBST(SYNAPSE_BE_LIBS)
    AC_SUBST(SYNAPSE_LIBTOOL_RPATH)
    
    
    # execute ACTION-IF-FOUND
    ifelse([$1], , :, [$1])
  else
    # execute ACTION-IF-NOT-FOUND
    ifelse([$2], , :, [$2])
  fi

])


