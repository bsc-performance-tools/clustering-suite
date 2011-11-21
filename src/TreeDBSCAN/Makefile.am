SUBDIRS = scripts

bin_PROGRAMS = Tree_DBSCAN_FE.bin Tree_DBSCAN_BE.bin

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../common-files \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libDistributedClustering \
	-I$(top_srcdir)/src/libSharedComponents

CLUSTERING_LIBS = \
  $(top_srcdir)/src/BasicClasses/libBasicClasses.la \
  $(top_srcdir)/src/libDistributedClustering/libDistributedClustering.la 

Tree_DBSCAN_FE_bin_SOURCES  = \
  Tree_DBSCAN_FE.cpp \
  Tree_DBSCAN_FE.h \
  NoiseManager.cpp \
  NoiseManager.h \
  ClusteringOffline_FE.cpp \
  ClusteringOffline_FE.h \
  tags.h \
  utils.h

Tree_DBSCAN_FE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
Tree_DBSCAN_FE_bin_LDFLAGS  = `@MRNETAPP_CONFIG@ --fe-libs` -static $(CLUSTERING_LIBS)
# Tree_DBSCAN_FE_bin_LDADD    = 

Tree_DBSCAN_BE_bin_SOURCES  = \
  Tree_DBSCAN_BE.cpp \
  NoiseManager.cpp NoiseManager.h \
  ClusteringOffline_BE.cpp \
  ClusteringOffline_BE.h \
  tags.h \
  utils.h

Tree_DBSCAN_BE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
Tree_DBSCAN_BE_bin_LDFLAGS  = `@MRNETAPP_CONFIG@ --be-libs` -static $(CLUSTERING_LIBS)
#Tree_DBSCAN_BE_bin_LDADD    = 

lib_LTLIBRARIES = libfilterClustering.la
libfilterClustering_la_SOURCES  = \
  Clustering_Filter.cpp \
  Clustering_Filter.h \
  NoiseManager.cpp \
  NoiseManager.h \
  tags.h \
  utils.h

libfilterClustering_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @CGAL_CPPFLAGS@
libfilterClustering_la_LDFLAGS  = @CGAL_LDFLAGS@ $(CLUSTERING_LIBS) @CLUSTERING_RPATH@
#libfilterClustering_la_LIBADD   = @CGAL_LDFLAGS@ -static 
