## Process this file with automake to produce Makefile.in

bin_PROGRAMS =  \
	ClustersDiff.bin

ClustersDiff_bin_SOURCES =  \
	ClustersDiff.cpp \
	MirkinDistance.cpp \
	MirkinDistance.hpp

ClustersDiff_bin_CPPFLAGS =\
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents
 
ClustersDiff_bin_LDFLAGS  = @CLUSTERING_LDFLAGS@
ClustersDiff_bin_LDADD = \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	@CLUSTERING_LIBS@

EXTRA_DIST=ClustersDiff
bin_SCRIPTS=ClustersDiff
