INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../common-files \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libTraceClustering

bin_PROGRAMS = \
	BurstClustering

BurstClustering_SOURCES = \
	BurstClustering.cpp

BurstClustering_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
    $(top_srcdir)/src/BasicClasses/libBasicClasses.la

BurstClustering_LDFLAGS = \
	-static \
    @XML2_LIBS@
