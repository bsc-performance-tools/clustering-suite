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

if TREEDBSCAN_ENABLED
SUBDIRS += libDistributedClustering
SUBDIRS += TreeDBSCAN 
endif

