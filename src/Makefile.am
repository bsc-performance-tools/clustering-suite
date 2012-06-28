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
	BurstClustering \
	ClustersDiff \
	ClustersSequenceScore
	

if HAVE_MPI
if HAVE_MUSTER
SUBDIRS += MusterDistributedClustering
endif
endif

if HAVE_CGAL

SUBDIRS += \
	libDistributedClustering

if HAVE_MRNETAPP
SUBDIRS += \
  TreeDBSCAN 
endif

endif

