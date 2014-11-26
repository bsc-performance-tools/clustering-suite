###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = ClusteringDataExtractor.bin

ClusteringDataExtractor_bin_SOURCES = \
	ClusteringDataExtractorMain.cpp 

ClusteringDataExtractor_bin_CPPFLAGS = \
  @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents
  
ClusteringDataExtractor_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@
ClusteringDataExtractor_bin_LDADD =\
 $(top_srcdir)/src/libTraceClustering/libTraceClustering.la\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@

EXTRA_DIST=ClusteringDataExtractor
bin_SCRIPTS=ClusteringDataExtractor

