INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libSharedComponents \
	-I$(top_srcdir)/src/libTraceClustering

bin_PROGRAMS = \
	BurstClustering

BurstClustering_SOURCES = \
	BurstClustering.cpp

BurstClustering_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

BurstClustering_CXXFLAGS = ${CXXFLAGS}
BurstClustering_CFLAGS   = ${CFLAGS}
BurstClustering_LDFLAGS  = ${LDFLAGS} -static
BurstClustering_LDADD   +=  @XML2_LDFLAGS@ @XML2_LIBS@ ${LIBS}

if HAVE_MUSTER

BurstClustering_CXXFLAGS += @MUSTER_CXXFLAGS@
BurstClustering_LDFLAGS += @MUSTER_LDFLAGS@
BurstClustering_LDADD   += @MUSTER_LIBS@

if HAVE_MPI
BurstClustering_CXXFLAGS += @MPI_CFLAGS@
BurstClustering_LDFLAGS += @MPI_LDFLAGS@
BurstClustering_LDADD   += @MPI_LIBS@
endif


endif
