## Process this file with automake to produce Makefile.in

INCLUDES =  \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../common-files \
    -I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libTraceClustering


bin_PROGRAMS =  \
	ClustersSequenceScore

ClustersSequenceScore_SOURCES =  \
	ClustersSequenceScore.cpp

ClustersSequenceScore_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la

ClustersSequenceScore_CXXFLAGS = ${CXXFLAGS}
