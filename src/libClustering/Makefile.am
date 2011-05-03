#include $(top_srcdir)/PATHS
SUBDIRS = gmeans

lib_LTLIBRARIES = libClustering.la

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
	-I$(top_srcdir)/src/SharedComponents 

libClustering_la_LIBADD= \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	$(top_srcdir)/src/libANN/libANN.la \
	./gmeans/libInternalGMeans.la

libClustering_la_CXXFLAGS=@CXXFLAGS@
libClustering_la_LDFLAGS=@LDFLAGS@
libClustering_la_LIBS=@LIBS@

if HAVE_CGAL
libClustering_la_CXXFLAGS += @CGAL_CPPFLAGS@
libClustering_la_LDFLAGS += @CGAL_LDFLAGS@
endif

if HAVE_MUSTER

libClustering_la_CXXFLAGS += @MUSTER_CXXFLAGS@
libClustering_la_LDFLAGS += @MUSTER_LDFLAGS@
libClustering_la_LIBS += @MUSTER_LIBS@

if HAVE_MPI
libClustering_la_CXXFLAGS += @MPI_CFLAGS@
libClustering_la_LDFLAGS += @MPI_LDFLAGS@
libClustering_la_LIBS += @MPI_LIBS@
endif

endif



instdir = $(includedir)

inst_HEADERS = \
	clustering_types.h \
	libClustering.hpp \
	Point.hpp \
	Partition.hpp


libClustering_la_SOURCES = \
	Classifier.hpp \
	ClusteringAlgorithm.hpp \
	ClusteringAlgorithmsFactory.cpp \
	ClusteringAlgorithmsFactory.hpp \
	DBSCAN.cpp \
	DBSCAN.hpp \
	GMEANS.cpp \
	GMEANS.hpp \
	OPTICS.cpp \
	OPTICS.hpp \
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

if HAVE_CGAL
inst_HEADERS += \
	ConvexHullModel.hpp

libClustering_la_SOURCES += \
	ConvexHullModel.hpp \
	ConvexHullModel.cpp \
	ConvexHullClassifier.hpp \
	ConvexHullClassifier.cpp

endif

if HAVE_MUSTER
libClustering_la_SOURCES += \
	MUSTER_DBSCAN.cpp \
	MUSTER_DBSCAN.hpp \
	MUSTER_PAM.cpp \
	MUSTER_PAM.hpp \
	MUSTER_XCLARA.cpp \
	MUSTER_XCLARA.hpp

if HAVE_MPI
libClustering_la_SOURCES += \
	MUSTER_CAPEK.cpp \
	MUSTER_CAPEK.hpp
endif

endif


