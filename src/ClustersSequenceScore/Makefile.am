## Process this file with automake to produce Makefile.in

INCLUDES =  \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/common_utilities \
    -I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libTraceClustering


bin_PROGRAMS =  \
	ClustersSequenceScore

ClustersSequenceScore_SOURCES =  \
	ClustersSequenceScore.cpp

ClustersSequenceScore_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la

ClustersSequenceScore_CXXFLAGS = ${CXXFLAGS}
ClustersSequenceScore_LDFLAGS  = ${LDFLAGS} -static

if HAVE_SQLITE3
ClustersSequenceScore_CXXFLAGS += @SQLITE3_CFLAGS@
ClustersSequenceScore_LDFLAGS  += @SQLITE3_LDFLAGS@
endif
