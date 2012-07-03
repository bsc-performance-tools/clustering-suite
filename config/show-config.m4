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
	echo LDFLAGS:             ${LDFLAGS}
	echo LIBS:                ${LIBS}
	echo 

	echo
	echo Optional features:
	echo ------------------

	echo Muster clustering library: ${muster_enabled}
	if test "${muster_enabled}" = "yes"; then
		echo -e \\\tMuster home:             ${muster_dir}
	fi

	echo MPI support: ${MPI_INSTALLED}
	if test "${MPI_INSTALLED}" = "yes" ; then
		echo -e \\\tMPI home:                ${MPI_HOME}
		echo -e \\\tshared libraries?        ${MPI_SHARED_LIB_FOUND}
	fi

	echo CGAL support: ${cgal_enabled}
	if test "x$cgal_enabled" = "xyes"; then
		echo -e \\\tCGAL_CPPFLAGS:           ${CGAL_CPPFLAGS}
		echo -e \\\tCGAL_LDFLAGS:            ${CGAL_LDFLAGS}
	fi

	echo MRNetApp support: ${MRNETAPP_INSTALLED}
	if test "x${MRNETAPP_INSTALLED}" = "xyes"; then
    	echo -e \\\tMRNetApp config script: "${MRNETAPP_HOME}/bin/mrnapp-config"
	fi

])
