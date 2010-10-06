INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/libTraceClustering

###############################################################################
# 'DataExtractor' binary
###############################################################################

bin_PROGRAMS = ClusteringDataExtractor

ClusteringDataExtractor_SOURCES = \
	ClusteringDataExtractorMain.cpp 

#ClusteringDataExtractor_CXXFLAGS = @XML2_CFLAGS@

#ClusteringDataExtractor_CFLAGS= @XML2_CFLAGS@

ClusteringDataExtractor_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la


ClusteringDataExtractor_LDFLAGS = \
	-static \
	@XML2_LIBS@

