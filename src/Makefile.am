include $(top_srcdir)/PATHS

common_filesdir = @common_filesdir@

SUBDIRS = \
	BasicClasses \
	libParaverTraceParser \
	libANN \
	libClustering \
	libSharedComponents \
	libTraceClustering \
	ClusteringDataExtractor \
	DBSCANParametersApproximation \
	BurstClustering 
	

if HAVE_MPI
if HAVE_MUSTER
SUBDIRS += MusterDistributedClustering
endif
endif

if HAVE_CGAL
SUBDIRS += \
	libDistributedClustering \
	MRNetStub
endif

