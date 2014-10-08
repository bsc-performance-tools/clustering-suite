AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses
	
	
	

###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = ClusteringDataExtractor

ClusteringDataExtractor_SOURCES = \
	ClusteringDataExtractorMain.cpp 

ClusteringDataExtractor_CPPFLAGS = @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents
  
ClusteringDataExtractor_LDFLAGS  = @CLUSTERING_LDFLAGS@
ClusteringDataExtractor_LDADD =\
 $(top_srcdir)/src/libTraceClustering/libTraceClustering.la\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@



