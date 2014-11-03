SUBDIRS = scripts

AM_CPPFLAGS = \
    -I$(top_srcdir)/include \
    -I$(top_srcdir)/common_utilities \
    -I$(top_srcdir)/src/BasicClasses \
    -I$(top_srcdir)/src/libClustering \
    -I$(top_srcdir)/src/libDistributedClustering \
    -I$(top_srcdir)/src/libSharedComponents

#########################################################
#             Global Variables                          #
#########################################################
CLUSTERING_FULL_CPPFLAGS=\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libDistributedClustering\
 -I$(top_srcdir)/src/libSharedComponents\
 @CLUSTERING_CPPFLAGS@

CLUSTERING_FULL_LIBS=\
 $(top_srcdir)/src/BasicClasses/libBasicClasses.la\
 $(top_srcdir)/src/libClustering/libClustering.la\
 $(top_srcdir)/src/libDistributedClustering/libDistributedClustering.la\
 @CLUSTERING_LIBS@
  

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

#libfilterTDBSCAN_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @CGAL_CPPFLAGS@
#libfilterTDBSCAN_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @BOOST_CPPFLAGS@
#libfilterTDBSCAN_la_CXXFLAGS = @MRNETAPP_CP_CPPFLAGS@ @BOOST_CPPFLAGS@
libfilterTDBSCAN_la_CPPFLAGS = @MRNETAPP_CP_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#libfilterTDBSCAN_la_LDFLAGS  =  @CGAL_LDFLAGS@ @CLUSTERING_RPATH@
#libfilterTDBSCAN_la_LDFLAGS  =  -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@ @CLUSTERING_RPATH@
#libfilterTDBSCAN_la_LDFLAGS  =  -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
libfilterTDBSCAN_la_LDFLAGS  = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
libfilterTDBSCAN_la_LIBADD   = $(CLUSTERING_FULL_LIBS)

libfilterXchangeDimensions_la_SOURCES = \
  XchangeDimensionsFilter.cpp \
  TDBSCANTags.h \
  Utils.h
#libfilterXchangeDimensions_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @BOOST_CPPFLAGS@
#libfilterXchangeDimensions_la_CXXFLAGS = @MRNETAPP_CP_CPPFLAGS@ @BOOST_CPPFLAGS@
libfilterXchangeDimensions_la_CPPFLAGS = @MRNETAPP_CP_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)

libfilterSupport_la_SOURCES = \
  SupportFilter.cpp \
  Support.cpp \
  Support.h   
#  JGG 'RTree.h' not available. Ask German!

#libfilterSupport_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --cp-cflags` @BOOST_CPPFLAGS@
#libfilterSupport_la_CXXFLAGS = @MRNETAPP_CP_CPPFLAGS@ @BOOST_CPPFLAGS@
libfilterSupport_la_CPPFLAGS = @MRNETAPP_CP_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)

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
  TDBSCANTags.h \
  Utils.h
#  JGG 'RTree.h' not available. Ask German!

libTDBSCAN_fe_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANRoot.cpp \
  TDBSCANRoot.h
#libTDBSCAN_fe_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@ @BOOST_CPPFLAGS@
#libTDBSCAN_fe_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @BOOST_CPPFLAGS@
#libTDBSCAN_fe_la_CXXFLAGS = @MRNETAPP_FE_CPPFLAGS@ @BOOST_CPPFLAGS@
libTDBSCAN_fe_la_CPPFLAGS = @MRNETAPP_FE_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#libTDBSCAN_fe_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --fe-libs`
#libTDBSCAN_fe_la_LDFLAGS  = $(MRNETAPP_LDFLAGS)
#libTDBSCAN_fe_la_LDFLAGS  = -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#libTDBSCAN_fe_la_LDFLAGS  = @MRNETAPP_LIBTOOL_RPATH@
libTDBSCAN_fe_la_LDFLAGS  = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
libTDBSCAN_fe_la_LIBADD   = @MRNETAPP_FE_LIBS@ $(CLUSTERING_FULL_LIBS) 

libTDBSCAN_be_offline_la_SOURCES  = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANWorker.cpp \
  TDBSCANWorker.h \
  TDBSCANWorkerOffline.cpp \
  TDBSCANWorkerOffline.h
#libTDBSCAN_be_offline_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
#libTDBSCAN_be_offline_la_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @BOOST_CPPFLAGS@
#libTDBSCAN_be_offline_la_CXXFLAGS = @MRNETAPP_BE_CPPFLAGS@ @BOOST_CPPFLAGS@
libTDBSCAN_be_offline_la_CPPFLAGS = @MRNETAPP_BE_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#libTDBSCAN_be_offline_la_LDFLAGS  = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs` 
#libTDBSCAN_be_offline_la_LDFLAGS  = $(MRNETAPP_LDFLAGS)
#libTDBSCAN_be_offline_la_LDFLAGS  = @CLUSTERING_RPATH@  -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#libTDBSCAN_be_offline_la_LDFLAGS  = -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#libTDBSCAN_be_offline_la_LDFLAGS  = @MRNETAPP_LIBTOOL_RPATH@
libTDBSCAN_be_offline_la_LDFLAGS  = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
#libTDBSCAN_be_offline_la_LIBADD   = $(CLUSTERING_LIBS)
libTDBSCAN_be_offline_la_LIBADD   = @MRNETAPP_BE_LIBS@ $(CLUSTERING_FULL_LIBS)


libTDBSCAN_be_online_la_SOURCES   = \
  $(PROTOCOL_COMMON_SRCS) \
  TDBSCANWorker.cpp \
  TDBSCANWorker.h \
  TDBSCANWorkerOnline.cpp \
  TDBSCANWorkerOnline.h
#libTDBSCAN_be_online_la_CXXFLAGS  = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
#libTDBSCAN_be_online_la_CXXFLAGS  = `@MRNETAPP_CONFIG@ --be-cflags` @BOOST_CPPFLAGS@
#libTDBSCAN_be_online_la_CXXFLAGS  = @MRNETAPP_BE_CPPFLAGS@ @BOOST_CPPFLAGS@
libTDBSCAN_be_online_la_CPPFLAGS  = @MRNETAPP_BE_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#libTDBSCAN_be_online_la_LDFLAGS   = @CGAL_LDFLAGS@ @CLUSTERING_RPATH@ `@MRNETAPP_CONFIG@ --be-libs`
#libTDBSCAN_be_online_la_LDFLAGS   = $(MRNETAPP_LDFLAGS)
#libTDBSCAN_be_online_la_LDFLAGS   = @CLUSTERING_RPATH@  -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#libTDBSCAN_be_online_la_LDFLAGS   = -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#libTDBSCAN_be_online_la_LDFLAGS   = @MRNETAPP_LIBTOOL_RPATH@
libTDBSCAN_be_online_la_LDFLAGS   = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
#libTDBSCAN_be_online_la_LIBADD    = $(CLUSTERING_LIBS)
libTDBSCAN_be_online_la_LIBADD    = @MRNETAPP_BE_LIBS@ $(CLUSTERING_FULL_LIBS)


#########################################################
#                       Headers                         #
#########################################################

include_HEADERS = \
  TDBSCANCore.h \
  TDBSCANRoot.h \
  TDBSCANWorker.h \
  TDBSCANWorkerOnline.h \
  TDBSCANWorkerOffline.h \
  TDBSCANTags.h \
  Statistics.h \
  ClustersInfo.h \
  Support.h

#########################################################
#         Interactive front-end and back-ends           #
#########################################################

bin_PROGRAMS = TDBSCAN_FE.bin TDBSCAN_BE.bin

TDBSCAN_FE_bin_SOURCES  = \
  TDBSCAN_FE.cpp \
  TDBSCAN_FE.h
#TDBSCAN_FE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @CGAL_CPPFLAGS@
#TDBSCAN_FE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --fe-cflags` @BOOST_CPPFLAGS@
#TDBSCAN_FE_bin_CXXFLAGS = @MRNETAPP_FE_CPPFLAGS@ @BOOST_CPPFLAGS@
TDBSCAN_FE_bin_CPPFLAGS = @MRNETAPP_FE_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#TDBSCAN_FE_bin_LDFLAGS  = -static 
#TDBSCAN_FE_bin_LDFLAGS = -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
# The Boost libraries shouldn't be added here
#TDBSCAN_FE_bin_LDFLAGS = @MRNETAPP_LIBTOOL_RPATH@ @BOOST_THREAD_LDFLAGS@
TDBSCAN_FE_bin_LDFLAGS = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
#TDBSCAN_FE_bin_LDADD    = libTDBSCAN-fe.la @MRNETAPP_FE_LIBS@ @BOOST_THREAD_LIBS@
TDBSCAN_FE_bin_LDADD    = libTDBSCAN-fe.la @MRNETAPP_FE_LIBS@ @CLUSTERING_LIBS@

TDBSCAN_BE_bin_SOURCES  = \
  TDBSCAN_BE.cpp
#TDBSCAN_BE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @CGAL_CPPFLAGS@
#TDBSCAN_BE_bin_CXXFLAGS = `@MRNETAPP_CONFIG@ --be-cflags` @BOOST_CPPFLAGS@
#TDBSCAN_BE_bin_CXXFLAGS = @MRNETAPP_BE_CPPFLAGS@ @BOOST_CPPFLAGS@
TDBSCAN_BE_bin_CPPFLAGS = @MRNETAPP_BE_CPPFLAGS@ $(CLUSTERING_FULL_CPPFLAGS)
#TDBSCAN_BE_bin_LDFLAGS  = -static 
#TDBSCAN_BE_bin_LDFLAGS = -L@MRNETAPP_LIBDIR@ -R@MRNETAPP_LIBDIR@
#TDBSCAN_BE_bin_LDFLAGS = @MRNETAPP_LIBTOOL_RPATH@ @BOOST_THREAD_LDFLAGS@
TDBSCAN_BE_bin_LDFLAGS = @MRNETAPP_LIBTOOL_RPATH@ @CLUSTERING_LDFLAGS@
#TDBSCAN_BE_bin_LDADD    = libTDBSCAN-be-offline.la @MRNETAPP_BE_LIBS@ @BOOST_THREAD_LIBS@
TDBSCAN_BE_bin_LDADD    = libTDBSCAN-be-offline.la @MRNETAPP_BE_LIBS@ @CLUSTERING_LIBS@

