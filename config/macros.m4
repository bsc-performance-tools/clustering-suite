# AX_FLAGS_SAVE
# -------------
AC_DEFUN([AX_FLAGS_SAVE],
[
   saved_LIBS="${LIBS}"
   saved_CC="${CC}"
   saved_CFLAGS="${CFLAGS}"
   saved_CXXFLAGS="${CXXFLAGS}"
   saved_CPPFLAGS="${CPPFLAGS}"
   saved_LDFLAGS="${LDFLAGS}"
])


# AX_FLAGS_RESTORE
# ----------------
AC_DEFUN([AX_FLAGS_RESTORE],
[
   LIBS="${saved_LIBS}"
   CC="${saved_CC}"
   CFLAGS="${saved_CFLAGS}"
   CXXFLAGS="${saved_CXXFLAGS}"
   CPPFLAGS="${saved_CPPFLAGS}"
   LDFLAGS="${saved_LDFLAGS}"
])


# AX_FIND_INSTALLATION
# --------------------
AC_DEFUN([AX_FIND_INSTALLATION],
[
	AC_REQUIRE([AX_SELECT_BINARY_TYPE])

	dnl Search for home directory
	AC_MSG_CHECKING([for $1 installation])
    for home_dir in [$2 "not found"]; do
        if test -d "$home_dir/$BITS" ; then
            home_dir="$home_dir/$BITS"
            break
        elif test -d "$home_dir" ; then
            break
        fi
    done
	AC_MSG_RESULT([$home_dir])
	$1_HOME="$home_dir"
	if test "$$1_HOME" = "not found" ; then
		$1_HOME=""
	else
		dnl Search for includes directory
		AC_MSG_CHECKING([for $1 includes directory])
		for incs_dir in [$$1_HOME/include$BITS $$1_HOME/include "not found"] ; do
			if test -d "$incs_dir" ; then
				break
			fi
		done
		AC_MSG_RESULT([$incs_dir])
		$1_INCLUDES="$incs_dir"
		if test "$$1_INCLUDES" = "not found" ; then
       $1_INCLUDES=""
       $1_CFLAGS=""
       $1_CXXFLAGS=""
       $1_CPPFLAGS=""
		else
       $1_CFLAGS="-I$$1_INCLUDES"
       $1_CXXFLAGS="-I$$1_INCLUDES"
       $1_CPPFLAGS="-I$$1_INCLUDES"
		fi

		dnl Search for libs directory
		AC_MSG_CHECKING([for $1 libraries directory])
		for libs_dir in [$$1_HOME/lib$BITS $$1_HOME/lib "not found"] ; do
			if test -d "$libs_dir" ; then
				break
			fi
		done
		AC_MSG_RESULT([$libs_dir])
		$1_LIBSDIR="$libs_dir"
		if test "$$1_LIBSDIR" = "not found" ; then
       $1_LIBSDIR=""
       $1_LDFLAGS=""
       $1_SHAREDLIBSDIR=""
		else
       $1_LDFLAGS="-L$$1_LIBSDIR"
       if test -d "$$1_LIBSDIR/shared" ; then
          $1_SHAREDLIBSDIR="$$1_LIBSDIR/shared"
       else
          $1_SHAREDLIBSDIR=$$1_LIBSDIR
       fi
		fi
	fi

	dnl Everything went OK?
	if test "$$1_HOME" != "" -a "$$1_INCLUDES" != "" -a "$$1_LIBSDIR" != "" ; then
		$1_INSTALLED="yes"

		AC_SUBST($1_HOME)
		AC_SUBST($1_INCLUDES)

    AC_SUBST($1_CFLAGS)
    AC_SUBST($1_CXXFLAGS)
    AC_SUBST($1_CPPFLAGS)

    AC_SUBST($1_LDFLAGS)
    AC_SUBST($1_SHAREDLIBSDIR)
    AC_SUBST($1_LIBSDIR)

    dnl Update the default variables so the automatic checks will take into account the new directories
    CFLAGS="$CFLAGS $$1_CFLAGS"
    CXXFLAGS="$CXXFLAGS $$1_CXXFLAGS"
    CPPFLAGS="$CPPFLAGS $$1_CPPFLAGS"
    LDFLAGS="$LDFLAGS $$1_LDFLAGS"
	else	
		$1_INSTALLED="no"
	fi
])


# AX_CHECK_POINTER_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_POINTER_SIZE],
[
   AC_TRY_RUN(
      [
         int main()
         {
            return sizeof(void *)*8;
         }
      ],
      [ POINTER_SIZE="0" ],
      [ POINTER_SIZE="$?"]
   )
])


# AX_SELECT_BINARY_TYPE
# ---------------------
# Check the binary type the user wants to build and verify whether it can be successfully built
AC_DEFUN([AX_SELECT_BINARY_TYPE],
[
	AC_ARG_WITH(binary-type,
		AC_HELP_STRING(
			[--with-binary-type@<:@=ARG@:>@],
			[choose the binary type between: 32, 64, default @<:@default=default@:>@]
		),
		[Selected_Binary_Type="$withval"],
		[Selected_Binary_Type="default"]
	)

	if test "$Selected_Binary_Type" != "default" -a "$Selected_Binary_Type" != "32" -a "$Selected_Binary_Type" != "64" ; then
		AC_MSG_ERROR([--with-binary-type: Invalid argument '$Selected_Binary_Type'. Valid options are: 32, 64, default.])
	fi

	C_compiler="$CC"
	CXX_compiler="$CXX"

	AC_LANG_SAVE([])
	m4_foreach([language], [[C], [C++]], [
		AC_LANG_PUSH(language)

		AC_CACHE_CHECK(
			[for $_AC_LANG_PREFIX[]_compiler compiler default binary type], 
			[[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type],
			[
				AX_CHECK_POINTER_SIZE
				Default_Binary_Type="$POINTER_SIZE"
				[]_AC_LANG_PREFIX[]_ac_cv_compiler_default_binary_type="$Default_Binary_Type""-bit"
			]
		)

		if test "$Default_Binary_Type" != "32" -a "$Default_Binary_Type" != 64 ; then
			AC_MSG_ERROR([Unknown default binary type (pointer size is $POINTER_SIZE!?)])
		fi

		if test "$Selected_Binary_Type" = "default" ; then
			Selected_Binary_Type="$Default_Binary_Type"
		fi

		if test "$Selected_Binary_Type" != "$Default_Binary_Type" ; then

			force_bit_flags="-m32 -q32 -32 -m64 -q64 -64 -maix64 none"

			AC_MSG_CHECKING([for $_AC_LANG_PREFIX[]_compiler compiler flags to build a $Selected_Binary_Type-bit binary])
			for flag in [$force_bit_flags]; do
				old_[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS"
				[]_AC_LANG_PREFIX[]FLAGS="$[]_AC_LANG_PREFIX[]FLAGS $flag"

				AX_CHECK_POINTER_SIZE()
				if test "$POINTER_SIZE" = "$Selected_Binary_Type" ; then
					BINARY_TYPE_FLAGS="$flag"
					AC_MSG_RESULT([$flag])
					break
				else
					[]_AC_LANG_PREFIX[]FLAGS="$old_[]_AC_LANG_PREFIX[]FLAGS"
					if test "$flag" = "none" ; then
						AC_MSG_RESULT([unknown])
						AC_MSG_NOTICE([$Selected_Binary_Type-bit binaries not supported])
						AC_MSG_ERROR([Please use '--with-binary-type' to select an appropriate binary type.])

					fi
				fi
			done
		fi
		AC_LANG_POP(language)
	])
	AC_LANG_RESTORE([])
	BITS="$Selected_Binary_Type"
])


# AX_CHECK_ENDIANNESS
# -------------------
# Test if the architecture is little or big endian
AC_DEFUN([AX_CHECK_ENDIANNESS],
[
   AC_CACHE_CHECK([for the architecture endianness], [ac_cv_endianness],
   [
      AC_LANG_SAVE()
      AC_LANG([C])
      AC_TRY_RUN(
      [
         int main()
         {
            short s = 1;
            short * ptr = &s;
            unsigned char c = *((char *)ptr);
            return c;
         }
      ],
      [ac_cv_endianness="big endian" ],
      [ac_cv_endianness="little endian" ]
      )
      AC_LANG_RESTORE()
   ])
   if test "$ac_cv_endianness" = "big endian" ; then
      AC_DEFINE(IS_BIG_ENDIAN, 1, [Define to 1 if architecture is big endian])
   fi
   if test "$ac_cv_endianness" = "little endian" ; then
      AC_DEFINE(IS_LITTLE_ENDIAN, 1, [Define to 1 if architecture is little endian])
   fi
])


# AX_CHECK__FUNCTION__MACRO
# -------------------------
# Check whether the compiler defines the __FUNCTION__ macro
AC_DEFUN([AX_CHECK__FUNCTION__MACRO],
[
   AC_CACHE_CHECK([whether the compiler defines the __FUNCTION__ macro], [ac_cv_have__function__],
      [
         AC_LANG_SAVE()
         AC_LANG([C])
         AC_TRY_COMPILE(
            [#include <stdio.h>],
            [
               char *s = __FUNCTION__;
               return 0;
            ],
            [ac_cv_have__function__="yes"],
            [ac_cv_have__function__="no"]
         )
         AC_LANG_RESTORE()
      ]
   )
   if test "$ac_cv_have__function__" = "yes" ; then
      AC_DEFINE([HAVE__FUNCTION__], 1, [Define to 1 if __FUNCTION__ macro is supported])
   fi
])

# AX_PROG_XML2
# -----------
AC_DEFUN([AX_PROG_XML2],
[
   XML2_HOME_BIN="`dirname ${XML2_CONFIG}`"
   XML2_HOME="`dirname ${XML2_HOME_BIN}`"

   XML2_INCLUDES1="${XML2_HOME}/include/libxml2"
   XML2_INCLUDES2="${XML2_HOME}/include"
   XML2_CFLAGS="-I${XML2_INCLUDES1} -I${XML2_INCLUDES2}"
   XML2_CPPFLAGS=${XML2_CFLAGS}
   XML2_CXXFLAGS=${XML2_CFLAGS}

   XML2_LIBS="-lxml2"
   if test -f ${XML2_HOME}/lib${BITS}/libxml2.so -o -f ${XML2_HOME}/lib${BITS}/libxml2.a ; then
      XML2_LIBSDIR="${XML2_HOME}/lib${BITS}"
   else
      XML2_LIBSDIR="${XML2_HOME}/lib"
   fi
   XML2_LDFLAGS="-L${XML2_LIBSDIR}"

   if test -d ${XML2_LIBSDIR}/shared ; then 
      XML2_SHAREDLIBSDIR="${XML2_LIBSDIR}/shared"
   else
      XML2_SHAREDLIBSDIR=${XML2_LIBSDIR}
   fi

   XML_LIBS="${XML2_LDFLAGS} -lxml2 -lz -lpthread -lm"

   AC_SUBST(XML2_HOME)
   AC_SUBST(XML2_CFLAGS)
   AC_SUBST(XML2_CPPFLAGS)
   AC_SUBST(XML2_CXXFLAGS)
   AC_SUBST(XML2_INCLUDES)
   AC_SUBST(XML2_LIBSDIR)
   AC_SUBST(XML2_SHAREDLIBSDIR)
   AC_SUBST(XML2_LIBS)
   AC_SUBST(XML2_LDFLAGS)
])


# AX_PROG_MPI
# -----------
AC_DEFUN([AX_PROG_MPI],
[
   AX_FLAGS_SAVE()

   if test "${IS_BGL_MACHINE}" = "yes" ; then
      mpi_default_paths="${BGL_HOME}/bglsys"
   else
      mpi_default_paths="none"
   fi

   AC_ARG_WITH(mpi,
      AC_HELP_STRING(
         [--with-mpi@<:@=DIR@:>@],
         [specify where to find MPI libraries and includes]
      ),
      [mpi_paths=${withval}],
      [mpi_paths=${mpi_default_paths}] dnl List of possible default paths
   )

   dnl Search for MPI installation
   AX_FIND_INSTALLATION([MPI], [$mpi_paths])

   if test "${MPI_INSTALLED}" = "yes" ; then

      if test -d "$MPI_INCLUDES/mpi" ; then
         MPI_INCLUDES="$MPI_INCLUDES/mpi"
         MPI_CFLAGS="-I$MPI_INCLUDES"
         CFLAGS="$MPI_CFLAGS $CFLAGS"
      fi

      dnl Check for the MPI header files.
      AC_CHECK_HEADERS([mpi.h], [], [MPI_INSTALLED="no"])

      dnl Check for the MPI library.
      dnl We won't use neither AC_CHECK_LIB nor AC_TRY_LINK because this library may have unresolved references to other libs (i.e: libgm).
      AC_MSG_CHECKING([for MPI library])
      if test -f "${MPI_LIBSDIR}/libmpi.a" ; then
         MPI_LIBS="-lmpi"
      elif test -f "${MPI_LIBSDIR}/libmpi.so" ; then
         MPI_LIBS="-lmpi"
      elif test -f "${MPI_LIBSDIR}/libmpich.a" ; then
         MPI_LIBS="-lmpich"
      elif test -f "${MPI_LIBSDIR}/libmpich.so" ; then
         MPI_LIBS="-lmpich"
      else
         MPI_LIBS="not found"
      fi
      if test -f "${MPI_LIBSDIR}/libmpi.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/libmpich.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/shared/libmpi.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      elif test -f "${MPI_LIBSDIR}/shared/libmpich.so"; then
         MPI_SHARED_LIB_FOUND="yes"
      fi
      AC_MSG_RESULT([${MPI_LIBSDIR}])

      if test "${MPI_LIBSDIR}" = "not found" ; then
         MPI_INSTALLED="no"
      else
         MPI_LDFLAGS="${MPI_LDFLAGS}"
         AC_SUBST(MPI_LDFLAGS)
         AC_SUBST(MPI_LIBS)
      fi

      dnl If $MPICC is not set, check for mpicc under $MPI_HOME/bin. We don't want to mix multiple MPI installations.
      AC_MSG_CHECKING([for MPI C compiler])
      if test "${MPICC}" = "" ; then
         mpicc_compilers="mpicc hcc mpxlc_r mpxlc mpcc cmpicc"
         for mpicc in [$mpicc_compilers]; do
            if test -f "${MPI_HOME}/bin/${mpicc}" ; then
               MPICC="${MPI_HOME}/bin/${mpicc}"
               AC_MSG_RESULT([${MPICC}])
               break
            fi
         done
         if test "${MPICC}" = "" ; then
            AC_MSG_RESULT([not found])
            AC_MSG_NOTICE([Cannot find \${MPI_HOME}/bin/mpicc -or similar- using \${CC} instead])
            MPICC_DOES_NOT_EXIST="yes"
            MPICC=${CC}
         else
            MPICC_DOES_NOT_EXIST="no"
         fi
      else
         AC_MSG_RESULT([${MPICC}])
      fi
   fi
   AC_SUBST(MPICC)

   # If the system do not have MPICC (or similar) be sure to add -lmpi and -Impi
   AM_CONDITIONAL(NEED_MPI_LIB_INCLUDE, test "${CC}" = "${MPICC}" )

   dnl Did the checks pass?
   AM_CONDITIONAL(HAVE_MPI, test "x${MPI_INSTALLED}" = "xyes")

   if test "$MPI_INSTALLED" = "no" ; then
       AC_MSG_WARN([No MPI installed. Distributed version will not be compiled])
   else
       AC_DEFINE([HAVE_MPI], 1, [Determine if MPI in installed])
   fi

   AX_FLAGS_RESTORE()
])

# AX_CHECK_MPI_STATUS_SIZE
# ---------------------
AC_DEFUN([AX_CHECK_MPI_STATUS_SIZE],
[
   AC_MSG_CHECKING([for size of the MPI_Status struct])
   AX_FLAGS_SAVE()
   CFLAGS="${CFLAGS} -I${MPI_INCLUDES}"
   AC_TRY_RUN(
      [
         #include <mpi.h>
         int main()
         {
            return sizeof(MPI_Status)/sizeof(int);
         }
      ],
      [ SIZEOF_MPI_STATUS="0" ],
      [ SIZEOF_MPI_STATUS="$?"]
   )
   AC_MSG_RESULT([${SIZEOF_MPI_STATUS}])
   AC_DEFINE_UNQUOTED([SIZEOF_MPI_STATUS], ${SIZEOF_MPI_STATUS}, [Size of the MPI_Status structure in "sizeof-int" terms])
   AX_FLAGS_RESTORE()
])

# AX_CHECK_MPI_TAG_OFFSET
#------------------------
AC_DEFUN([AX_CHECK_MPI_TAG_OFFSET],
[
   AX_FLAGS_SAVE()
   CFLAGS="${CFLAGS} -I${MPI_INCLUDES}"

   AC_CHECK_MEMBER(MPI_Status.MPI_TAG,,
                [AC_MSG_ERROR([We need MPI_Status.MPI_TAG!])],
                [#include <mpi.h>])

   AC_MSG_CHECKING([for offset of TAG field in MPI_Status])
   AC_TRY_RUN(
      [
         #include <mpi.h>
         int main()
         {
            MPI_Status s;
            long addr1 = (long) &s;
            long addr2 = (long) &(s.MPI_TAG);

            return (addr2 - addr1)/sizeof(int);
         }
      ],
      [ MPI_TAG_OFFSET="0" ],
      [ MPI_TAG_OFFSET="$?"]
   )
   AC_MSG_RESULT([${MPI_TAG_OFFSET}])
   AC_DEFINE_UNQUOTED([MPI_TAG_OFFSET], ${MPI_TAG_OFFSET}, [Offset of the TAG field in MPI_Status in sizeof-int terms])
   AX_FLAGS_RESTORE()
])



# AX_PROG_PAPI
# ------------
AC_DEFUN([AX_PROG_PAPI],
[
   AX_FLAGS_SAVE()

   papi_default_paths="none"

   AC_ARG_WITH(papi,
      AC_HELP_STRING(
         [--with-papi@<:@=DIR@:>@],
         [specify where to find PAPI libraries and includes]
      ),
      [papi_paths="${withval}"],
      [papi_paths=${papi_default_paths}] dnl List of possible default paths
   )
   AC_ARG_ENABLE(sampling,
      AC_HELP_STRING(
         [--enable-sampling],
         [Enable PAPI sampling support]
      ),
      [enable_sampling="${enableval}"],
      [enable_sampling="auto"]
   )
   PAPI_SAMPLING_ENABLED="no"

   dnl Search for PAPI installation
   AX_FIND_INSTALLATION([PAPI], [$papi_paths])

   if test "${PAPI_INSTALLED}" = "yes" ; then
      AC_CHECK_HEADERS([papi.h], [], [PAPI_INSTALLED="no"])

      if test "${IS_BGL_MACHINE}" = "yes" ; then
         LIBS="-static ${LIBS} -L${BGL_HOME}/bglsys/lib -lbgl_perfctr.rts -ldevices.rts -lrts.rts"
      else
         if test "${OperatingSystem}" = "freebsd" ; then
            LIBS="-lpapi -lpmc"
         elif test "${OperatingSystem}" = "linux" -a "${Architecture}" = "powerpc" ; then
            LIBS="-lpapi -lperfctr"
         else
            LIBS="-lpapi"
         fi
      fi

      AC_CHECK_LIB([papi], [PAPI_start],
         [ 
           if test "${IS_BGL_MACHINE}" = "yes" ; then
              PAPI_LIBS="-static -lpapi -L${BGL_HOME}/bglsys/lib -lbgl_perfctr.rts -ldevices.rts -lrts.rts"
           else
              if test "${OperatingSystem}" = "freebsd" ; then
                 PAPI_LIBS="-lpapi -lpmc"
              elif test "${OperatingSystem}" = "linux" -a "${Architecture}" = "powerpc" ; then
                 PAPI_LIBS="-lpapi -lperfctr"
              else
                 PAPI_LIBS="-lpapi"
              fi
           fi
           AC_SUBST(PAPI_LIBS)
         ],
         [PAPI_INSTALLED="no"]
      )
   fi

   AM_CONDITIONAL(HAVE_PAPI, test "${PAPI_INSTALLED}" = "yes")

   if test "${PAPI_INSTALLED}" = "no" ; then
      AC_MSG_WARN([PAPI counters tracing support has been disabled])
   else
      AC_DEFINE([NEW_HWC_SYSTEM], [1], [Enable HWC support])
      AC_DEFINE([USE_HARDWARE_COUNTERS], [1], [Enable HWC support])
      if test "${enable_sampling}" = "yes" ; then
         AC_CHECK_MEMBER([PAPI_substrate_info_t.supports_hw_overflow],[support_hw_overflow="yes"],[support_hw_overflow="no"],[#include <papi.h>])
         if test "${support_hw_overflow}" = "yes" ; then
            AC_DEFINE([HAVE_SUPPORT_HW_OVERFLOW], [1], [Use supports_hw_overflow field])
            AC_DEFINE([SAMPLING_SUPPORT], [1], [Enable PAPI sampling support])
            PAPI_SAMPLING_ENABLED="yes"
         else
            AC_CHECK_MEMBER([PAPI_substrate_info_t.hardware_intr_sig],[hardware_intr_sig="yes"],[hardware_intr_sig="no"],[#include <papi.h>])
            if test "${hardware_intr_sig}" = "yes" ; then
               AC_DEFINE([HAVE_HARDWARE_INTR_SIG], [1], [Use hardware_intr_sig field])
               AC_DEFINE([SAMPLING_SUPPORT], [1], [Enable PAPI sampling support])
               PAPI_SAMPLING_ENABLED="yes"
            else
               AC_MSG_ERROR([Cannot determine how to check whether PAPI supports HW overflows!])
            fi
         fi
      fi
   fi

   AX_FLAGS_RESTORE()
])

# AX_IS_ALTIX_MACHINE
# ----------------
AC_DEFUN([AX_IS_ALTIX_MACHINE],
[
   AC_MSG_CHECKING([if this is an Altix machine])
   if test -r /etc/sgi-release ; then 
      AC_MSG_RESULT([yes])
      IS_ALTIX_MACHINE="yes"
			AC_DEFINE([IS_ALTIX], 1, [Defined if this machine is a SGI Altix])
   else
      AC_MSG_RESULT([no])
      IS_ALTIX_MACHINE="no"
   fi
])


# AX_HAVE_MMTIMER_DEVICE
# ----------------
AC_DEFUN([AX_HAVE_MMTIMER_DEVICE],
[
   AC_REQUIRE([AX_IS_ALTIX_MACHINE])

   if test "${IS_ALTIX_MACHINE}" = "yes" ; then
      AC_MSG_CHECKING([if this is an Altix machine has MMTimer device])
      if test -r /dev/mmtimer ; then 
         AC_MSG_RESULT([yes])
         AC_DEFINE([HAVE_MMTIMER_DEVICE], 1, [Defined if this machine has a MMTimer device and it is readable])
         HAVE_MMTIMER_DEVICE="yes"
      else
         AC_MSG_RESULT([no])
         HAVE_MMTIMER_DEVICE="no"
      fi
   else
      HAVE_MMTIMER_DEVICE="no"
   fi
])



# AX_IS_BGL_MACHINE
# ---------------------
AC_DEFUN([AX_IS_BGL_MACHINE],
[
   AC_MSG_CHECKING([if this is a BG/L machine])
   if test -d /bgl/BlueLight/ppcfloor/bglsys ; then
     IS_BGL_MACHINE="yes"
     BGL_HOME="/bgl/BlueLight/ppcfloor"
     CFLAGS="${CFLAGS} -I${BGL_HOME}/bglsys/include -I${BGL_HOME}/blrts-gnu/include"
     AC_SUBST(BGL_HOME)
     AC_MSG_RESULT([yes])
     AC_DEFINE([IS_BGL_MACHINE], 1, [Defined if this machine is a BG/L machine])
   else
     IS_BGL_MACHINE="no"
     AC_MSG_RESULT([no])
   fi
   AM_CONDITIONAL(IS_BGL_MACHINE, test "${IS_BGL_MACHINE}" = "yes")
])

# AX_IS_MN_MACHINE
#---------------------
AC_DEFUN([AX_IS_MN_MACHINE],
[
   AC_MSG_CHECKING([if this is MN machine])
   grep "Welcome to MareNostrum" /etc/motd 2> /dev/null > /dev/null
   GREP_RESULT=$?
   if test "${GREP_RESULT}" = "0" ; then
      AC_MSG_RESULT([yes])
      AC_DEFINE([IS_MN_MACHINE], 1, [Defined if this machine is MN])
   else
      AC_MSG_RESULT([no])
   fi
])

# AX_OPENMP
#-----------------
AC_DEFUN([AX_OPENMP],
[
   AC_PREREQ(2.59)

   AC_CACHE_CHECK([for OpenMP flag of _AC_LANG compiler],
      ax_cv_[]_AC_LANG_ABBREV[]_openmp,
      [save[]_AC_LANG_PREFIX[]FLAGS=$[]_AC_LANG_PREFIX[]FLAGS ax_cv_[]_AC_LANG_ABBREV[]_openmp=unknown
      # Flags to try:  -fopenmp (gcc), -openmp (icc), -mp (SGI &amp; PGI),
      #                -xopenmp (Sun), -omp (Tru64), -qsmp=omp (AIX), none
      ax_openmp_flags="-fopenmp -openmp -mp -xopenmp -omp -qsmp=omp none"
      if test "x$OPENMP_[]_AC_LANG_PREFIX[]FLAGS" != x; then
         ax_openmp_flags="$OPENMP_[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flags"
      fi
      for ax_openmp_flag in $ax_openmp_flags; do
         case $ax_openmp_flag in
            none) []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[] ;;
            *) []_AC_LANG_PREFIX[]FLAGS="$save[]_AC_LANG_PREFIX[]FLAGS $ax_openmp_flag" ;;
         esac
         AC_TRY_LINK_FUNC(omp_set_num_threads,
   	       [ax_cv_[]_AC_LANG_ABBREV[]_openmp=$ax_openmp_flag; break])
      done
      []_AC_LANG_PREFIX[]FLAGS=$save[]_AC_LANG_PREFIX[]FLAGS])
      if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" = "xunknown"; then
         m4_default([$2],:)
      else
         if test "x$ax_cv_[]_AC_LANG_ABBREV[]_openmp" != "xnone"; then
            OPENMP_[]_AC_LANG_PREFIX[]FLAGS=$ax_cv_[]_AC_LANG_ABBREV[]_openmp
         fi
         m4_default([$1], [AC_DEFINE(HAVE_OPENMP,1,[Define if OpenMP is enabled])])
      fi
])


# AX_CHECK_LIBZ
# ------------
AC_DEFUN([AX_CHECK_LIBZ],
[
   AX_FLAGS_SAVE()

   AC_ARG_WITH(libz,
      AC_HELP_STRING(
         [--with-libz@<:@=DIR@:>@],
         [specify where to find libz libraries and includes]
      ),
      [libz_paths="${withval}"],
      [libz_paths="/usr/local /usr"] dnl List of possible default paths
   )

   for home_dir in [${libz_paths} "not found"]; do
      if test -f "${home_dir}/${BITS}/include/zlib.h" -a \
              -f "${home_dir}/${BITS}/lib/libz.a" ; then
         LIBZ_HOME="${home_dir}/${BITS}"
         LIBZ_LIBSDIR="${home_dir}/${BITS}/lib"
         break
      elif test -f "${home_dir}/include/zlib.h" -a \
                -f "${home_dir}/lib${BITS}/libz.a" ; then
         LIBZ_HOME="${home_dir}"
         LIBZ_LIBSDIR="${home_dir}/lib${BITS}"
      elif test -f "${home_dir}/include/zlib.h" -a \
                -f "${home_dir}/lib/libz.a" ; then
         LIBZ_HOME="${home_dir}"
         LIBZ_LIBSDIR="${home_dir}/lib"
         break
      fi
    done

   LIBZ_INCLUDES="${LIBZ_HOME}/include"
   LIBZ_CFLAGS="-I${LIBZ_INCLUDES}"
   LIBZ_CPPFLAGS=${LIBZ_CFLAGS}
   LIBZ_CXXFLAGS=${LIBZ_CFLAGS}
   LIBZ_LIBS="-lz"
   LIBZ_LDFLAGS="-L${LIBZ_LIBSDIR}"
   if test -d ${LIBZ_LIBSDIR}/shared ; then 
      LIBZ_SHAREDLIBSDIR="${LIBZ_LIBSDIR}/shared"
   else
      LIBZ_SHAREDLIBSDIR=${LIBZ_LIBSDIR}
   fi

   AC_SUBST(LIBZ_HOME)
   AC_SUBST(LIBZ_CFLAGS)
   AC_SUBST(LIBZ_CPPFLAGS)
   AC_SUBST(LIBZ_CXXFLAGS)
   AC_SUBST(LIBZ_INCLUDES)
   AC_SUBST(LIBZ_LIBSDIR)
   AC_SUBST(LIBZ_SHAREDLIBSDIR)
   AC_SUBST(LIBZ_LIBS)
   AC_SUBST(LIBZ_LDFLAGS)

   CFLAGS="${CFLAGS} ${LIBZ_CFLAGS}"
   LIBS="${LIBS} ${LIBZ_LIBS}"
   LDFLAGS="${LDFLAGS} ${LIBZ_LDFLAGS}"

   AC_CHECK_LIB(z, inflateEnd, [zlib_cv_libz=yes], [zlib_cv_libz=no])
   AC_CHECK_HEADER(zlib.h, [zlib_cv_zlib_h=yes], [zlib_cv_zlib_h=no])

   if test "${zlib_cv_libz}" = "yes" -a "${zlib_cv_zlib_h}" = "yes" ; then
      AC_DEFINE([HAVE_ZLIB], [1], [Zlib available])
			ZLIB_INSTALLED="yes"
   else
      ZLIB_INSTALLED="no"
   fi

   AM_CONDITIONAL(HAVE_ZLIB, test "${ZLIB_INSTALLED}" = "yes")

   AX_FLAGS_RESTORE()
])


AC_DEFUN([AX_OFF_T_64BIT],
[
	AC_MSG_CHECKING([how to get 64-bit off_t])
	if test "${OperatingSystem}" = "linux" ; then
		AC_DEFINE([_FILE_OFFSET_BITS],[64],[Define the bits for the off_t structure])
		AC_MSG_RESULT([define _FILE_OFFSET_BITS=64])
	elif test "${OperatingSystem}" = "freebsd" ; then
		AC_MSG_RESULT([nothing required])
	else
		AC_MSG_RESULT([unknown])
	fi
])

dnl Check for CGAL and MPFR (uses AX_LIB_CGAL_CORE)
AC_DEFUN([AX_CHECK_CGAL],
[

cgal_enabled=no
AX_CHECK_MPFR()

if test "x$mpfr_enabled" = "xyes"; then
	
	BOOST_REQUIRE(1.36)
	BOOST_THREADS
  
	dnl AC_MSG_CHECKING([for CGAL installation])

	if test "$BOOST_THREAD_LIBS"; then
	
		AX_FLAGS_SAVE()

		CXXFLAGS="$CXXFLAGS -frounding-math $MPFR_CXXFLAGS $BOOST_CPPFLAGS"
		CFLAGS="$CFLAGS $MPFR_CFLAGS $BOOST_CPPFLAGS"
		LDFLAGS="$LDFLAGS $MPFR_LDFLAGS $BOOST_THREAD_LDFLAGS $BOOST_THREAD_LIBS"
		LIBS="$LIBS $MPFR_LIBS $BOOST_THREAD_LIBS"
    
		AX_LIB_CGAL_CORE(
			CGAL_CPPFLAGS="$CGAL_CPPFLAGS -frounding-math $MPFR_CXXFLAGS $BOOST_CPPFLAGS"
			CGAL_LDFLAGS="$CGAL_LDFLAGS $MPFR_LDFLAGS $MPFR_LIBS $BOOST_THREAD_LDFLAGS $BOOST_THREAD_LIBS"
			AC_SUBST(CGAL_CPPFLAGS)
			AC_SUBST(CGAL_LDFLAGS)
	      	cgal_enabled="yes",
			cgal_enabled="no")

		if test ! "x$cgal_enabled" = "xyes"; then
			AC_MSG_RESULT([CGAL not found, some functionalities will be missing])
		fi

		AX_FLAGS_RESTORE()
	fi
fi

AM_CONDITIONAL(HAVE_CGAL, test "x$cgal_enabled" = "xyes")

])

AC_DEFUN([AX_CHECK_MPFR],
[

AC_MSG_CHECKING([for MPFR installation (needed by CGAL)])

AC_ARG_WITH([mpfr],
	AS_HELP_STRING([--with-mpfr=MPFR_DIR],
	[sets the given directory as location of MPFR includes and libs (needed by CGAL)]),
	[mpfr_paths="$withval"],
	[mpfr_paths="$MPFR_HOME"' /usr /usr/local /opt /opt/local']
)

mpfr_enabled="no"

for mpfr_iterate in $mpfr_paths; do

	if test -e $mpfr_iterate/include/mpfr.h -a -e $mpfr_iterate/lib/libmpfr.so; then
		mpfr_enabled="yes"
	fi

	AC_MSG_CHECKING([whether MPFR is available in $mpfr_iterate])

	if test "$mpfr_enabled" = "yes"; then
		mpfr_dir=$mpfr_iterate
		AC_MSG_RESULT([yes])
		break
	else
		AC_MSG_RESULT([no])
	fi
done

if test "x$mpfr_enabled" = "xyes"; then
    AC_DEFINE(HAVE_MPFR, 1, [Defined if MPFR library is enabled])
   	AC_SUBST(mpfr_dir)

    MPFR_CXXFLAGS="-I${mpfr_dir}/include"
   	MPFR_CFLAGS="-I${mpfr_dir}/include"
	MPFR_LDFLAGS="-L${mpfr_dir}/lib"
	MPFR_LIBS="-lmpfr"

	AC_SUBST(MPFR_CXXFLAGS)
	AC_SUBST(MPFR_CFLAGS)
	AC_SUBST(MPFR_LDFLAGS)
	AC_SUBST(MPFR_LIBS)
fi

AM_CONDITIONAL(HAVE_MPFR, test "x$mpfr_enabled" = "xyes")

])

# AX_CHECK_SEQAN
# ----------------

AC_DEFUN([AX_CHECK_SEQAN],
[
	AC_ARG_WITH([seqan],
	AS_HELP_STRING([--with-seqan=SEQAN_DIR], [sets the given directory as location of Seqan includes]),
	[seqan_paths="$withval"],
	[seqan_paths="$SEQAN_HOME"' /usr /usr/local /opt /opt/local']
)

SEQAN_TEST_PROGRAM='AC_LANG_PROGRAM(
[
[@%:@include <iostream>]
[@%:@include <seqan/align.h>]
[@%:@include <seqan/graph_msa.h>]
[@%:@include <seqan/score.h>]
[using namespace seqan;]
[using std::ostream;]
[typedef String<int>                        TSequence;]
[typedef Align<TSequence, ArrayGaps>        TAlign;]
[typedef Row<TAlign>::Type                  TRow;]
[typedef Iterator<TRow,Rooted>::Type        TIterator;]
[typedef Position<Rows<TAlign>::Type>::Type TRowsPosition;]
[typedef Position<TAlign>::Type             TPosition;]
]
,
[
[TAlign align;
TSequence seq1, seq2, seq3, seq4;
StringSet<TSequence> seq;
Score<int> score(10, -2, -1, -1);

/* Sequence 1 */
appendValue(seq1, 2);
appendValue(seq1, 1);
appendValue(seq1, 1);
appendValue(seq1, 1);
appendValue(seq1, 4);

/* Sequence 2 */
appendValue(seq2, 1);
appendValue(seq2, 3);
appendValue(seq2, 5);
appendValue(seq2, 4);

/* Sequence 3 */
appendValue(seq3, 1);
appendValue(seq3, 3);
appendValue(seq3, 6);
appendValue(seq3, 1);
appendValue(seq3, 4);

/* Sequence 4 */
appendValue(seq4, 1);
appendValue(seq4, 3);
appendValue(seq4, 4);
appendValue(seq4, 1);

resize(rows(align), 4);

assignSource(row(align, 0), seq1);
assignSource(row(align, 1), seq2);
assignSource(row(align, 2), seq3);
assignSource(row(align, 3), seq4);

globalMsaAlignment(align, score);
]
])'

AC_LANG_PUSH([C++])

seqan_enabled="no"
for seqan_iterate in $seqan_paths; do
	
	CPPFLAGS_SAVED="$CPPFLAGS"
	SEQAN_CPPFLAGS="-I$seqan_iterate/include"
	CPPFLAGS="$CPPFLAGS $SEQAN_CPPFLAGS"
	export CPPFLAGS

	AC_MSG_CHECKING([whether Seqan is available in $seqan_iterate])
	AC_RUN_IFELSE($SEQAN_TEST_PROGRAM,[seqan_enabled=yes],[seqan_enabled=no])

	CPPFLAGS="$CPPFLAGS_SAVED"
	export CPPFLAGS

	if test "$seqan_enabled" = "yes"; then
		seqan_dir=$seqan_iterate
		AC_MSG_RESULT([yes])
		break
	else
		AC_MSG_RESULT([no])
	fi
done

AC_LANG_POP([C++])

if test "x$seqan_enabled" = "xyes"; then
	AC_DEFINE(HAVE_SEQAN, 1, [Defined if Seqan library is enabled])
	AC_SUBST(seqan_dir)

	SEQAN_CPPFLAGS="-I${seqan_dir}/include"

	AC_SUBST(SEQAN_CPPFLAGS)
else
	AC_MSG_WARN([SeqAn library not installed. Clustering refinement will not be available])
fi

AM_CONDITIONAL(HAVE_SEQAN, test "x$seqan_enabled" = "xyes")
])
