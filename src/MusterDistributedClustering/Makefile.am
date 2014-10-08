AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/common_utilities\
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libTraceClustering

bin_PROGRAMS = \
	MusterDistributedClustering

MusterDistributedClustering_SOURCES = \
	MusterDistributedClustering.cpp	

MusterDistributedClustering_CPPFLAGS = $(top_srcdir)/include @CLUSTERING_CPPFLAGS@
MusterDistributedClustering_LDFLAGS  = @CLUSTERING_LDFLAGS@
MusterDistributedClustering_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	@CLUSTERING_LIBS@

