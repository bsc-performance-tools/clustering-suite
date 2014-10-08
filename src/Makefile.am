include $(top_srcdir)/PATHS

# common_filesdir = @common_filesdir@

SUBDIRS = \
	BasicClasses \
	libParaverTraceParser \
	libANN \
	libClustering \
	libSharedComponents \
	libTraceClustering \
	ClusteringDataExtractor \
	DBSCANParametersApproximation \
	BurstClustering \
	ClustersDiff \
	ClustersSequenceScore


if HAVE_MPI
if HAVE_MUSTER
SUBDIRS += MusterDistributedClustering
endif
endif

if CGAL_INSTALLED
SUBDIRS += libDistributedClustering
if MRNETAPP_INSTALLED
SUBDIRS += TreeDBSCAN 
endif
endif

