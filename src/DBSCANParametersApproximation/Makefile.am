INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libTraceClustering

###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = DBSCANParametersApproximation

DBSCANParametersApproximation_SOURCES = \
	DBSCANParametersApproximationMain.cpp 

DBSCANParametersApproximation_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

DBSCANParametersApproximation_CXXFLAGS = ${CXXFLAGS}
DBSCANParametersApproximation_CFLAGS   = ${CFLAGS}
DBSCANParametersApproximation_LDFLAGS  = ${LDFLAGS} -static
DBSCANParametersApproximation_LDADD   +=  @XML2_LDFLAGS@ @XML2_LIBS@ ${LIBS}

if HAVE_MUSTER

DBSCANParametersApproximation_CXXFLAGS += @MUSTER_CXXFLAGS@
DBSCANParametersApproximation_LDFLAGS += @MUSTER_LDFLAGS@
DBSCANParametersApproximation_LDADD   += @MUSTER_LIBS@

if HAVE_MPI
DBSCANParametersApproximation_CXXFLAGS += @MPI_CFLAGS@
DBSCANParametersApproximation_LDFLAGS += @MPI_LDFLAGS@
DBSCANParametersApproximation_LDADD   += @MPI_LIBS@
endif


endif
