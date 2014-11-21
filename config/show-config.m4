ge# AX_SHOW_CONFIGURATION
# --------------------
AC_DEFUN([AX_SHOW_CONFIGURATION],
[

	echo
	echo Package configuration :
	echo -----------------------
	echo Installation prefix: ${prefix}
	echo CC:                  ${CC}
	echo CXX:                 ${CXX}
	echo Binary type:         ${BITS} bits
	echo common-files:        ${common_filesdir:-default}
	echo CXXFLAGS:            ${CXXFLAGS}
	echo CFLAGS:              ${CFLAGS}
	echo CLUSTERING_CPPFLAGS: ${CLUSTERING_CPPFLAGS}
	echo CLUSTERING_LDFLAGS:  ${CLUSTERING_LDFLAGS}
	echo CLUSTERING_LIBS:     ${CLUSTERING_LIBS}
	echo 

  echo
  echo Optional features:
  echo ------------------

  echo Muster clustering library: ${muster_installed}
  if test "${muster_enabled}" = "yes"; then
    echo -e \\\tMUSTER_CPPFLAGS:         ${MUSTER_CPPFLAGS}
    echo -e \\\tMUSTER_LIBSDIR:          ${MUSTER_LIBSDIR}
    echo -e \\\tMUSTER_LIBS:             ${MUSTER_LIBS}
  fi

  echo MPI support: ${MPI_INSTALLED}
  if test "${MPI_INSTALLED}" = "yes" ; then
    echo -e \\\tMPI home:                ${MPI_HOME}
    echo -e \\\tshared libraries?        ${MPI_SHARED_LIB_FOUND}
  fi
  
  # Tree DSBCAN is available is 'MRNETApp' (Synapse) is available
  echo TREEDBSCAN enabled: ${mrnetapp_enabled:-no}
  if test "x${TREEDBSCAN_ENABLED}" = "xyes"; then
    echo ""
    echo -e -- MPFR flags --
    echo -e \\\tMPFR_CPPFLAGS:           ${MPFR_CPPFLAGS}
    echo -e \\\tMPFR_LIBSDIR:            ${MPFR_LIBSDIR}
    echo -e \\\tMPFR_LIBS:               ${MPFR_LIBS}
    echo -e -- GMP  flags --
    echo -e \\\tGMP_CPPFLAGS:            ${GMP_CPPFLAGS}
    echo -e \\\tGMP_LIBSDIR:             ${GMP_LIBSDIR}
    echo -e \\\tGMP_LIBS:                ${GMP_LIBS}
    echo -e -- CGAL flags --
    echo -e \\\tCGAL_CPPFLAGS:           ${CGAL_CPPFLAGS}
    echo -e \\\tCGAL_LIBSDIR:            ${CGAL_LIBSDIR}
    echo -e \\\tCGAL_LIBS:               ${CGAL_LIBS}
    echo -e -- MRNetApp flags --
    echo -e \\\tMRNetApp config script: "${MRNETAPP_HOME}/bin/mrnapp-config"
    echo ""
  fi

  echo SQLite3 support: ${sqlite3_installed}
  if test "x${SQLITE3_INSTALLED}" = "xyes"; then
    echo -e \\\tSQLITE3_CFLAGS:          ${SQLITE3_CFLAGS}
    echo -e \\\tSQLITE3_LDFLAGS:         ${SQLITE3_LDFLAGS}
  fi

  if test "x${TREEDBSCAN_ENABLED}" = "xyes" -a "x${SQLITE3_INSTALLED}"  = "xno"; then
    echo ""
    echo "WARNING: compiling the TreeDBSCAN packages without SQLite3 availability"
    echo "WARNING: results on a memory bound implementation of the algorithm"
    echo ""
  fi
])
