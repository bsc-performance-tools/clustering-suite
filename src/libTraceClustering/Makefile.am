#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libTraceClustering.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/DataExtraction \
	-I$(top_srcdir)/src/libClustering \
    -I$(top_srcdir)/src/SharedComponents 

libTraceClustering_la_LIBADD= \
	$(top_srcdir)/src/DataExtraction/libDataExtraction.la \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/SharedComponents/libClusteringUtils.la


libTraceClustering_la_LDFLAGS= @XML2_LIBS@ 

libTraceClustering_la_CXXFLAGS = @XML2_CXXFLAGS@

libTraceClustering_la_SOURCES = \
	libTraceClustering.cpp \
	libTraceClustering.hpp \
	libTraceClusteringImplementation.cpp \
	libTraceClusteringImplementation.hpp \
	trace_clustering_types.h
