common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/libANN \
	-I$(top_srcdir)/src/libClustering \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(common_filesdir)

noinst_LTLIBRARIES = libSharedComponents.la

libSharedComponents_la_LIBADD =  \
	$(top_srcdir)/src/libClustering/libClustering.la \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la

libSharedComponents_la_CFLAGS   = @XML2_CFLAGS@
libSharedComponents_la_CXXFLAGS = @XML2_CFLAGS@
libSharedComponents_la_LDFLAGS  = -static @XML2_LIBS@

if HAVE_MPI
libSharedComponents_la_CXXFLAGS += @MPI_CFLAGS@
libSharedComponents_la_LDFLAGS += @MPI_LDFLAGS@
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
	TRFDataExtractor.cpp \
	TRFDataExtractor.hpp \
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
	XMLParser.hpp 

if HAVE_SEQAN
libSharedComponents_la_CFLAGS   += @SEQAN_CPPFLAGS@
libSharedComponents_la_CXXFLAGS += @SEQAN_CPPFLAGS@

libSharedComponents_la_SOURCES += \
	ClusterInformation.hpp \
	ClusterInformation.cpp \
	ClusteringRefinementDivisive.hpp \
	ClusteringRefinementDivisive.cpp \
	ClusteringRefinementAggregative.hpp \
	ClusteringRefinementAggregative.cpp \
	SequenceScore.hpp \
	SequenceScore.cpp
endif
