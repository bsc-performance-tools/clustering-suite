#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libTraceClustering.la

common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(top_srcdir)/src/SharedComponents \
	-I$(common_filesdir)

libTraceClustering_la_LIBADD= \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/SharedComponents/libClusteringUtils.la \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la


libTraceClustering_la_LDFLAGS= @XML2_LIBS@ 

libTraceClustering_la_CXXFLAGS = @XML2_CXXFLAGS@

libTraceClustering_la_SOURCES = \
	ClusteredPRVGenerator.cpp \
	ClusteredPRVGenerator.hpp \
	ClusteredTraceGenerator.cpp \
	ClusteredTraceGenerator.hpp \
	ClusteredTRFGenerator.cpp \
	ClusteredTRFGenerator.hpp \
	CPUBurst.cpp \
	CPUBurst.hpp \
	DataExtractor.cpp \
	DataExtractorFactory.cpp \
	DataExtractorFactory.hpp \
	DataExtractor.hpp \
	PRVDataExtractor.cpp \
	PRVDataExtractor.hpp \
	TraceData.cpp \
	TraceData.hpp \
	TRFDataExtractor.cpp \
	TRFDataExtractor.hpp \
	libTraceClustering.cpp \
	libTraceClustering.hpp \
	libTraceClusteringImplementation.cpp \
	libTraceClusteringImplementation.hpp \
	trace_clustering_types.h
