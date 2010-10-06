noinst_LTLIBRARIES = libDataExtraction.la

common_filesdir = @common_filesdir@
INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(common_filesdir) \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/SharedComponents \
	-I$(top_srcdir)/src/libParaverTraceParser \
	-I$(top_srcdir)/src/libTraceClustering \
	-I$(top_srcdir)/src/libClustering

#libDataExtraction_la_CFLAGS = @XML2_CFLAGS@

#libDataExtraction_la_CXXFLAGS = @XML2_CFLAGS@

libDataExtraction_la_SOURCES = \
	DataExtractor.cpp \
	DataExtractor.hpp \
	PRVDataExtractor.cpp \
	PRVDataExtractor.hpp \
	TRFDataExtractor.cpp \
	TRFDataExtractor.hpp \
	DataExtractorFactory.cpp \
	DataExtractorFactory.hpp \
	CPUBurst.cpp \
	CPUBurst.hpp \
	TraceData.cpp \
	TraceData.hpp

libDataExtraction_la_LIBADD = \
	$(top_srcdir)/src/libParaverTraceParser/libParaverTraceParser.la 
