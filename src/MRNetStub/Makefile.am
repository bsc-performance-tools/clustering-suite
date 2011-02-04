INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/../common-files \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libDistributedClustering \
	-I$(top_srcdir)/src/libSharedComponents

bin_PROGRAMS = \
	MRNetStub

MRNetStub_SOURCES = \
	MRNetStub.cpp

MRNetStub_LDADD = \
	$(top_srcdir)/src/BasicClasses/libBasicClasses.la \
	$(top_srcdir)/src/libDistributedClustering/libDistributedClustering.la 

# To compile this file CGAL must be active!
MRNetStub_CXXFLAGS = ${CXXFLAGS} @CGAL_CPPFLAGS@
MRNetStub_CFLAGS   = ${CFLAGS}
MRNetStub_LDFLAGS  = ${LDFLAGS} -static  @CGAL_LDFLAGS@
MRNetStub_LDADD   +=  @XML2_LIBS@ ${LIBS} @CGAL_LDFLAGS@

if HAVE_MUSTER

MRNetStub_CXXFLAGS += @MUSTER_CXXFLAGS@
MRNetStub_LDFLAGS += @MUSTER_LDFLAGS@
MRNetStub_LDADD   += @MUSTER_LIBS@

if HAVE_MPI
MRNetStub_CXXFLAGS += @MPI_CFLAGS@
MRNetStub_LDFLAGS += @MPI_LDFLAGS@
MRNetStub_LDADD   += @MPI_LIBS@
endif

endif

