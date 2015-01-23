#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libTraceClustering.la

## common_filesdir = @common_filesdir@

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/pcfparser_svn1816

instdir = $(includedir)

inst_HEADERS = \
	libTraceClustering.hpp

libTraceClustering_la_SOURCES = \
	libTraceClustering.cpp \
	libTraceClustering.hpp \
	libTraceClusteringImplementation.cpp \
	libTraceClusteringImplementation.hpp 

libTraceClustering_la_CPPFLAGS = @CLUSTERING_CPPFLAGS@\
  -I$(top_srcdir)/src/libClustering\
  -I$(top_srcdir)/src/libSharedComponents\
  -I$(top_srcdir)/pcfparser_svn1816\
  -I$(top_srcdir)/src/libANN\
  -I$(top_srcdir)/src/libParaverTraceParser

libTraceClustering_la_LDFLAGS  = @CLUSTERING_CPPFLAGS@ @CLUSTERING_LDFLAGS@
libTraceClustering_la_LIBADD =  \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/libSharedComponents/libSharedComponents.la \
	@CLUSTERING_LIBS@

##	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la




