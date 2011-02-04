#include $(top_srcdir)/PATHS

lib_LTLIBRARIES = libTraceClustering.la

common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(common_filesdir)

libTraceClustering_la_LIBADD =  \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/libSharedComponents/libSharedComponents.la \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la


libTraceClustering_la_CXXFLAGS=@XML2_CXXFLAGS@
libTraceClustering_la_LDFLAGS=@LDFLAGS@ -static @XML2_LIBS@ 
libTraceClustering_la_LIBS=

if HAVE_MUSTER
libTraceClustering_la_CXXFLAGS += @MUSTER_CXXFLAGS@
libTraceClustering_la_LDFLAGS += @MUSTER_LDFLAGS@
libTraceClustering_la_LIBS += @MUSTER_LIBS@

if HAVE_MPI

libTraceClustering_la_CXXFLAGS += @MPI_CFLAGS@
libTraceClustering_la_LDFLAGS += @MPI_LDFLAGS@

endif
endif

if HAVE_CGAL
libTraceClustering_la_CXXFLAGS +=  @CGAL_CPPFLAGS@
libTraceClustering_la_LDFLAGS += @CGAL_LDFLAGS@ 
endif

instdir = $(includedir)

inst_HEADERS = \
	libTraceClustering.hpp

libTraceClustering_la_SOURCES = \
	libTraceClustering.cpp \
	libTraceClustering.hpp \
	libTraceClusteringImplementation.cpp \
	libTraceClusteringImplementation.hpp 


