
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
  libfilterTDBSCAN.la \
  libfilterXchangeDimensions.la \
  libfilterSupport.la \
  libTDBSCAN-fe.la \
  libTDBSCAN-be-offline.la \
  libTDBSCAN-be-online.la

libfilterTDBSCAN_la_SOURCES  = \
  TDBSCANFilter.cpp \
  TDBSCANFilter.h \
  NoiseManager.cpp \
  NoiseManager.h \
  HullManager.cpp \
  HullManager.h \
  Statistics.cpp \
  Statistics.h \
  ClustersInfo.cpp \
  ClustersInfo.h \
  TDBSCANTags.h \
  Utils.h
libfilterTDBSCAN_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @CGAL_CPPFLAGS@
libfilterTDBSCAN_la_LDFLAGS  =  @CGAL_LDFLAGS@ @CLUSTERING_RPATH@
libfilterTDBSCAN_la_LIBADD   = $(CLUSTERING_LIBS) 

libfilterXchangeDimensions_la_SOURCES = \
  XchangeDimensionsFilter.cpp \
  TDBSCANTags.h \
  Utils.h
libfilterXchangeDimensions_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags`

libfilterSupport_la_SOURCES = \
  SupportFilter.cpp \
  Support.cpp \
  Support.h   \
  RTree.h 
libfilterSupport_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags`

PROTOCOL_COMMON_SRCS = \
  TDBSCANCore.cpp \
  TDBSCANCore.h \
  NoiseManager.cpp \
  NoiseManager.h \
  HullManager.cpp \
  HullManager.h \
  Statistics.cpp \
  Statistics.h \
  ClustersInfo.cpp \
  ClustersInfo.h \
  Support.cpp \
  Support.h \
  RTree.h \
  TDBSCANTags.h \
  Utils.h

libTDBSCAN_fe_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANRoot.cpp \
  TDBSCANRoot.h
libTDBSCAN_fe_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
libTDBSCAN_fe_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --fe-libs` 
libTDBSCAN_fe_la_LIBADD   = $(CLUSTERING_LIBS) 

libTDBSCAN_be_offline_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANWorker.cpp \
  TDBSCANWorker.h \
  TDBSCANWorkerOffline.cpp \
  TDBSCANWorkerOffline.h
libTDBSCAN_be_offline_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
libTDBSCAN_be_offline_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs` 
libTDBSCAN_be_offline_la_LIBADD   = $(CLUSTERING_LIBS)

libTDBSCAN_be_online_la_SOURCES   = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANWorker.cpp \
  TDBSCANWorker.h \
  TDBSCANWorkerOnline.cpp \
  TDBSCANWorkerOnline.h
libTDBSCAN_be_online_la_CXXFLAGS  = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
libTDBSCAN_be_online_la_LDFLAGS   = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs` 
libTDBSCAN_be_online_la_LIBADD    = $(CLUSTERING_LIBS)

#########################################################
#                       Headers                         #
#########################################################

include_HEADERS = TDBSCANCore.h TDBSCANRoot.h TDBSCANWorker.h TDBSCANWorkerOnline.h TDBSCANWorkerOffline.h TDBSCANTags.h Statistics.h ClustersInfo.h Support.h

#########################################################
#         Interactive front-end and back-ends           #
#########################################################

bin_PROGRAMS = TDBSCAN_FE.bin TDBSCAN_BE.bin

TDBSCAN_FE_bin_SOURCES  = \
  TDBSCAN_FE.cpp \
  TDBSCAN_FE.h
TDBSCAN_FE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
TDBSCAN_FE_bin_LDFLAGS  = -static 
TDBSCAN_FE_bin_LDADD    = libTDBSCAN-fe.la

TDBSCAN_BE_bin_SOURCES  = \
  TDBSCAN_BE.cpp
TDBSCAN_BE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
TDBSCAN_BE_bin_LDFLAGS  = -static 
TDBSCAN_BE_bin_LDADD    = libTDBSCAN-be-offline.la

