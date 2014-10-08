bin_PROGRAMS = \
	BurstClustering

BurstClustering_SOURCES = \
	BurstClustering.cpp

BurstClustering_CPPFLAGS = \
 @CLUSTERING_CPPFLAGS@\
 -I$(top_srcdir)/src/libClustering\
 -I$(top_srcdir)/src/libTraceClustering\
 -I$(top_srcdir)/src/libSharedComponents

BurstClustering_LDFLAGS  = @CLUSTERING_LDFLAGS@
BurstClustering_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	@CLUSTERING_LIBS@


