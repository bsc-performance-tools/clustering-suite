
SUBDIRS = scripts

INCLUDES = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/common_utilities \
    -I$(top_srcdir)/src/BasicClasses \
    -I$(top_srcdir)/src/libClustering \
    -I$(top_srcdir)/src/libDistributedClustering \
    -I$(top_srcdir)/src/libSharedComponents

CLUSTERING_LIBS = \
  $(top_srcdir)/src/BasicClasses/libBasicClasses.la \
  $(top_srcdir)/src/libDistributedClustering/libDistributedClustering.la

#########################################################
#             Clustering MRNetApp protocol              # 
#########################################################

lib_LTLIBRARIES = \
  libfilterTreeDBSCAN.la \
  libfilterXchangeDimensions.la \
  libfrontendTreeDBSCAN.la \
  libbackendTreeDBSCANOffline.la \
  libbackendTreeDBSCANOnline.la

libfilterTreeDBSCAN_la_SOURCES  = \
  ClusteringFilter.cpp \
  ClusteringFilter.h \
  NoiseManager.cpp \
  NoiseManager.h \
  HullManager.cpp \
  HullManager.h \
  Statistics.cpp \
  Statistics.h \
  ClusteringTags.h \
  Utils.h
libfilterTreeDBSCAN_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @CGAL_CPPFLAGS@
libfilterTreeDBSCAN_la_LDFLAGS  =  @CGAL_LDFLAGS@ @CLUSTERING_RPATH@
libfilterTreeDBSCAN_la_LIBADD   = $(CLUSTERING_LIBS) 

libfilterXchangeDimensions_la_SOURCES = \
  XchangeDimensionsFilter.cpp \
  ClusteringTags.h \
  Utils.h
libfilterXchangeDimensions_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags`

PROTOCOL_COMMON_SRCS = \
  ClusteringCore.cpp \
  ClusteringCore.h \
  NoiseManager.cpp \
  NoiseManager.h \
  HullManager.cpp \
  HullManager.h \
  Statistics.cpp \
  Statistics.h \
  ClusteringTags.h \
  Utils.h

libfrontendTreeDBSCAN_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  ClusteringFrontEnd.cpp \
  ClusteringFrontEnd.h
libfrontendTreeDBSCAN_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
libfrontendTreeDBSCAN_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --fe-libs` -static
libfrontendTreeDBSCAN_la_LIBADD   = $(CLUSTERING_LIBS) 

libbackendTreeDBSCANOffline_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  ClusteringBackEnd.cpp \
  ClusteringBackEnd.h \
  ClusteringBackEndOffline.cpp \
  ClusteringBackEndOffline.h
libbackendTreeDBSCANOffline_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
libbackendTreeDBSCANOffline_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs` -static
libbackendTreeDBSCANOffline_la_LIBADD   = $(CLUSTERING_LIBS)

libbackendTreeDBSCANOnline_la_SOURCES   = \
  $(PROTOCOL_COMMON_SRCS) \
  ClusteringBackEnd.cpp \
  ClusteringBackEnd.h \
  ClusteringBackEndOnline.cpp \
  ClusteringBackEndOnline.h
libbackendTreeDBSCANOnline_la_CXXFLAGS  = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
libbackendTreeDBSCANOnline_la_LDFLAGS   = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs` -static
libbackendTreeDBSCANOnline_la_LIBADD    = $(CLUSTERING_LIBS)

#########################################################
#                       Headers                         #
#########################################################

include_HEADERS = ClusteringCore.h ClusteringFrontEnd.h ClusteringBackEnd.h ClusteringBackEndOnline.h ClusteringBackEndOffline.h ClusteringTags.h

#########################################################
#         Interactive front-end and back-ends           #
#########################################################

bin_PROGRAMS = Tree_DBSCAN_FE.bin Tree_DBSCAN_BE.bin

Tree_DBSCAN_FE_bin_SOURCES  = \
  Tree_DBSCAN_FE.cpp \
  Tree_DBSCAN_FE.h
Tree_DBSCAN_FE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
Tree_DBSCAN_FE_bin_LDFLAGS  = -static 
Tree_DBSCAN_FE_bin_LDADD    = libfrontendTreeDBSCAN.la

Tree_DBSCAN_BE_bin_SOURCES  = \
  Tree_DBSCAN_BE.cpp
Tree_DBSCAN_BE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
Tree_DBSCAN_BE_bin_LDFLAGS  = -static 
Tree_DBSCAN_BE_bin_LDADD    = libbackendTreeDBSCANOffline.la

