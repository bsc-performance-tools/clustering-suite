#include $(top_srcdir)/PATHS
SUBDIRS = gmeans

lib_LTLIBRARIES = libClustering.la

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

if TREEDBSCAN_ENABLED
inst_HEADERS += \
	ConvexHullModel.hpp 

libClustering_la_SOURCES += \
	ConvexHullModel.hpp \
	ConvexHullModel.cpp \
	ConvexHullClassifier.hpp \
	ConvexHullClassifier.cpp \
	CGAL_Kernel/MyConstruct_bbox_2.h \
	CGAL_Kernel/MyConstruct_coord_iterator.h \
	CGAL_Kernel/MyConstruct_point_2.h \
	CGAL_Kernel/MyKernel.h \
	CGAL_Kernel/MyPointC2.h \
	CGAL_Kernel/MyPointC2_iostream.h \
	CGAL_Kernel/MySegmentC2.h
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

libClustering_la_CPPFLAGS=@CLUSTERING_CPPFLAGS@ -I$(top_srcdir)/src/libANN
libClustering_la_LDFLAGS=@CLUSTERING_LDFLAGS@
libClustering_la_LIBADD= \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	$(top_srcdir)/src/libANN/libANN.la \
	./gmeans/libInternalGMeans.la \
	@CLUSTERING_LIBS@




