#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libDistributedClustering.la

## common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libTraceClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(top_srcdir)/common_utilities

libDistributedClustering_la_LIBADD= \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	$(top_srcdir)/src/libSharedComponents/libSharedComponents.la 


libDistributedClustering_la_CXXFLAGS = @XML2_CXXFLAGS@ @CGAL_CPPFLAGS@ ${BOOST_CPPFLAGS}
libDistributedClustering_la_LDFLAGS  = @CGAL_LDFLAGS@ ${BOOST_THREAD_LDFLAGS} @XML2_LDFLAGS@ 
libDistributedClustering_la_LIBADD  += @XML2_LIBS@ ${BOOST_THREAD_LIBS}

if HAVE_MUSTER

libDistributedClustering_la_CXXFLAGS += @MUSTER_CXXFLAGS@
libDistributedClustering_la_LDFLAGS  += @MUSTER_LDFLAGS@
libDistributedClustering_la_LIBADD   += @MUSTER_LIBS@

if HAVE_MPI
libDistributedClustering_la_CXXFLAGS += @MPI_CFLAGS@
libDistributedClustering_la_LDFLAGS  += @MPI_LDFLAGS@
libDistributedClustering_la_LIBADD   += @MPI_LIBS@

endif
endif

if HAVE_SQLITE3
libDistributedClustering_la_CXXFLAGS += @SQLITE3_CFLAGS@
libDistributedClustering_la_LDFLAGS  += @SQLITE3_LDFLAGS@
endif


instdir = $(includedir)

inst_HEADERS = \
	libDistributedClustering.hpp \
	HullModel.hpp

libDistributedClustering_la_SOURCES = \
	libDistributedClustering.hpp \
	libDistributedClustering.cpp \
	libDistributedClusteringImplementation.hpp \
	libDistributedClusteringImplementation.cpp \
	HullModel.hpp \
	HullModel.cpp 


