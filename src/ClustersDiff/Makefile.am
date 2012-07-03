## Process this file with automake to produce Makefile.in

INCLUDES =  \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../common-files \
    -I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents


bin_PROGRAMS =  \
	ClustersDiff

ClustersDiff_SOURCES =  \
	ClustersDiff.cpp \
	MirkinDistance.cpp \
	MirkinDistance.hpp

ClustersDiff_LDADD = \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

ClustersDiff_CXXFLAGS = ${CXXFLAGS}
ClustersDiff_LDFLAGS  = ${LDFLAGS} -static
