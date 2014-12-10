## Process this file with automake to produce Makefile.in
bin_PROGRAMS =  \
	ClustersSequenceScore.bin

ClustersSequenceScore_bin_SOURCES =  \
	ClustersSequenceScore.cpp

ClustersSequenceScore_bin_CPPFLAGS =\
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents\
 -I$(top_srcdir)/pcfparser_svn1816

ClustersSequenceScore_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@

ClustersSequenceScore_bin_LDADD = \
 $(top_srcdir)/src/libTraceClustering/libTraceClustering.la\
 $(top_srcdir)/src/libClustering/libClustering.la\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@

bin_SCRIPTS=ClustersSequenceScore.sh
EXTRA_DIST=ClustersSequenceScore.sh

install-data-hook:
	mv ${prefix}/bin/ClustersSequenceScore.sh ${prefix}/bin/ClustersSequenceScore

