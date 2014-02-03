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

MusterDistributedClustering_LDADD = \
	$(top_srcdir)/src/libTraceClustering/libTraceClustering.la \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

MusterDistributedClustering_CXXFLAGS = ${CXXFLAGS} @MUSTER_CXXFLAGS@ @MPI_CFLAGS@
MusterDistributedClustering_CFLAGS   = ${CFLAGS} @MUSTER_CXXFLAGS@ @MPI_CFLAGS@
MusterDistributedClustering_LDFLAGS  = ${LDFLAGS} @XML2_LDFLAGS@ -static @MUSTER_LDFLAGS@ @MPI_LDFLAGS@
MusterDistributedClustering_LDADD   +=  @XML2_LIBS@ ${LIBS} @MUSTER_LIBS@ @MPI_LIBS@

