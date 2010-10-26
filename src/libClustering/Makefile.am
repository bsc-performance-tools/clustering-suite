#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libClustering.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
    -I$(top_srcdir)/src/SharedComponents 

libClustering_la_LIBADD= \
	$(top_srcdir)/src/libANN/libANN.la 
	

libClustering_la_LDFLAGS= @XML2_LIBS@ 

libClustering_la_CXXFLAGS = @XML2_CXXFLAGS@

libClustering_la_SOURCES = \
	Classifier.hpp \
	ClusteringAlgorithm.hpp \
	ClusteringAlgorithmsFactory.cpp \
	ClusteringAlgorithmsFactory.hpp \
	DBSCAN.cpp \
	DBSCAN.hpp \
	GMEANS.cpp \
	GMEANS.hpp \
	libClustering.cpp \
	libClustering.hpp \
	libClusteringImplementation.cpp \
	libClusteringImplementation.hpp \
	NearestNeighbourClassifier.cpp \
	NearestNeighbourClassifier.hpp \
	Point.cpp \
	Point.hpp \
	clustering_types.h \
	Partition.hpp \
	Partition.cpp