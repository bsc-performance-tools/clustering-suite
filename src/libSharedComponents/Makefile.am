## common_filesdir = @common_filesdir@

SUBDIRS = kalign2

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(top_srcdir)/common_utilities \
	-I$(top_srcdir)/pcfparser_svn1816

noinst_LTLIBRARIES = libSharedComponents.la

libSharedComponents_la_LIBADD =  \
	$(top_srcdir)/pcfparser_svn1816/libparavertraceconfig.la \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la \
	./kalign2/libKalign2.la
##	$(top_srcdir)/common_utilities/libCommonUtilities.la \
##	./kalign2/libKalign2.la

libSharedComponents_la_CFLAGS   = @CFLAGS@ @XML2_CFLAGS@
libSharedComponents_la_CXXFLAGS = @CXXFLAGS@ @XML2_CFLAGS@
libSharedComponents_la_LDFLAGS  = \
  -L@XML2_SHAREDLIBSDIR@ -R@XML2_SHAREDLIBSDIR@ @XML2_LIBS@
## Flags to use the external LIBTOOLS. It will be used with the
## new 'common_files' package 
## libSharedComponents_la_LDFLAGS  = \
##  -L@XML2_SHAREDLIBSDIR@ -R@XML2_SHAREDLIBSDIR@ @XML2_LIBS@ \
##  -L@LIBTOOLS_LIBSDIR@ -R@LIBTOOLS_LIBSDIR@ @LIBTOOLS_LIBS@


if HAVE_MPI
libSharedComponents_la_CXXFLAGS += @MPI_CFLAGS@
libSharedComponents_la_LDFLAGS += @MPI_LIBSDIR@ -R @MPI_LIBS@
endif

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
libSharedComponents_la_CXXFLAGS += @SQLITE3_CFLAGS@
libSharedComponents_la_LDFLAGS += @SQLITE3_LDFLAGS@

libSharedComponents_la_SOURCES += \
	BurstsDB.hpp \
	BurstsDB.cpp

endif

