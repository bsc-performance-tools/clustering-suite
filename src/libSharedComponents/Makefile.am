## common_filesdir = @common_filesdir@

SUBDIRS = kalign2

noinst_LTLIBRARIES = libSharedComponents.la

instdir = $(includedir)

inst_HEADERS = \
	trace_clustering_types.h

libSharedComponents_la_SOURCES = \
	trace_clustering_types.h \
	ClusteredTraceGenerator.cpp \
	ClusteredTraceGenerator.hpp \
	ClusteredStatesPRVGenerator.cpp \
	ClusteredStatesPRVGenerator.hpp \
	ClusteredEventsPRVGenerator.cpp \
	ClusteredEventsPRVGenerator.hpp \
	ClusteredTRFGenerator.cpp \
	ClusteredTRFGenerator.hpp \
	DataExtractor.cpp \
	DataExtractorFactory.cpp \
	DataExtractorFactory.hpp \
	DataExtractor.hpp \
	PRVStatesDataExtractor.cpp \
	PRVStatesDataExtractor.hpp \
	PRVEventsDataExtractor.cpp \
	PRVEventsDataExtractor.hpp \
	PRVSemanticGuidedDataExtractor.cpp \
	PRVSemanticGuidedDataExtractor.hpp \
	SemanticGuidedPRVGenerator.cpp \
	SemanticGuidedPRVGenerator.hpp \
	TRFDataExtractor.cpp \
	TRFDataExtractor.hpp \
	CSVDataExtractor.cpp \
	CSVDataExtractor.hpp \
	ClusteringConfiguration.cpp \
	ClusteringConfiguration.hpp \
	ClusteringParameter.cpp \
	ClusteringParameter.hpp \
	CPIStackModel.hpp \
	CPUBurst.cpp \
	CPUBurst.hpp \
	ParametersManager.cpp \
	ParametersManager.hpp \
	PlottingManager.cpp \
	PlottingManager.hpp \
	State.cpp \
	State.hpp \
	TraceData.cpp \
	TraceData.hpp \
	ClusteringStatistics.cpp \
	ClusteringStatistics.hpp \
	XMLParser.cpp \
	XMLParser.hpp \
	ClusterInformation.hpp \
	ClusterInformation.cpp \
	ClusteringRefinementDivisive.hpp \
	ClusteringRefinementDivisive.cpp \
	ClusteringRefinementAggregative.hpp \
	ClusteringRefinementAggregative.cpp \
	SequenceScore.hpp \
	SequenceScore.cpp \
	asa136.hpp \
	asa136.cpp

if HAVE_SQLITE3

libSharedComponents_la_SOURCES += \
	BurstsDB.hpp \
	BurstsDB.cpp

endif

libSharedComponents_la_CPPFLAGS = @CLUSTERING_CPPFLAGS@\
  -I${top_srcdir}/src/libClustering\
  -I${top_srcdir}/src/libParaverTraceParser\
  -I${top_srcdir}/src/libANN\
  -I${top_srcdir}/pcfparser_svn1816
  

libSharedComponents_la_LDFLAGS  = @CLUSTERING_LDFLAGS@ @CLUSTERING_CPPFLAGS@

libSharedComponents_la_LIBADD =  \
	$(top_srcdir)/pcfparser_svn1816/libparavertraceconfig.la \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la \
	./kalign2/libKalign2.la \
	@CLUSTERING_LIBS@




