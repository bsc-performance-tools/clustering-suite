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
 -I$(top_srcdir)/pcfparser_svn3942

ClustersSequenceScore_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@

ClustersSequenceScore_bin_LDADD = \
 $(top_builddir)/src/libTraceClustering/libTraceClustering.la\
 $(top_builddir)/src/libClustering/libClustering.la\
 $(top_builddir)/src/BasicClasses/libBasicClasses.la\
 @CLUSTERING_LIBS@

bin_SCRIPTS=ClustersSequenceScore.sh
EXTRA_DIST=ClustersSequenceScore.sh

install-data-hook:
	mv $(DESTDIR)${bindir}/ClustersSequenceScore.sh $(DESTDIR)${bindir}/ClustersSequenceScore

