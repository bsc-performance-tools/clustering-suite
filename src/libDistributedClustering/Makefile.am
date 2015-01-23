#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libDistributedClustering.la

instdir = $(includedir)

inst_HEADERS = \
	libDistributedClustering.hpp \
	HullModel.hpp \
	ClusterStatistics.hpp

libDistributedClustering_la_SOURCES = \
	libDistributedClustering.hpp \
	libDistributedClustering.cpp \
	libDistributedClusteringImplementation.hpp \
	libDistributedClusteringImplementation.cpp \
	HullModel.hpp \
	HullModel.cpp \
	ClusterStatistics.hpp \
	ClusterStatistics.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libTraceClustering \
	-I$(top_srcdir)/common_utilities
	

libDistributedClustering_la_CPPFLAGS = @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libSharedComponents\
 -I$(top_srcdir)/pcfparser_svn1816\
 -I$(top_srcdir)/src/libParaverTraceParser\
 -I$(top_srcdir)/src/libANN
 
libDistributedClustering_la_LDFLAGS  = @CLUSTERING_LDFLAGS@ @CLUSTERING_CPPFLAGS@
libDistributedClustering_la_LIBADD= \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	$(top_srcdir)/src/libSharedComponents/libSharedComponents.la \
	@CLUSTERING_LIBS@




