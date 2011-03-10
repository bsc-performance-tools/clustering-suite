INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libTraceClustering

###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = ClusteringDataExtractor

ClusteringDataExtractor_SOURCES = \
	ClusteringDataExtractorMain.cpp 

ClusteringDataExtractor_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

ClusteringDataExtractor_CXXFLAGS = ${CXXFLAGS}
ClusteringDataExtractor_CFLAGS   = ${CFLAGS}
ClusteringDataExtractor_LDFLAGS  = ${LDFLAGS} -static
ClusteringDataExtractor_LDADD   +=  @XML2_LIBS@ ${LIBS}

if HAVE_MUSTER

ClusteringDataExtractor_CXXFLAGS += @MUSTER_CXXFLAGS@
ClusteringDataExtractor_LDFLAGS += @MUSTER_LDFLAGS@
ClusteringDataExtractor_LDADD   += @MUSTER_LIBS@

if HAVE_MPI
ClusteringDataExtractor_CXXFLAGS += @MPI_CFLAGS@
ClusteringDataExtractor_LDFLAGS += @MPI_LDFLAGS@
ClusteringDataExtractor_LDADD   += @MPI_LIBS@
endif


endif
