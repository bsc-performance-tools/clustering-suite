noinst_LTLIBRARIES=libParaverTraceParser.la

libParaverTraceParser_la_SOURCES=\
	ParaverApplicationDescription.cpp \
	ParaverApplicationDescription.hpp \
	ParaverCommunicator.cpp \
	ParaverCommunicator.hpp \
	ParaverHeader.cpp \
	ParaverHeader.hpp \
	ParaverRecord.cpp \
	ParaverRecord.hpp \
	ParaverTraceParser.cpp \
	ParaverTraceParser.hpp \
	ParaverMetadataManager.cpp \
	ParaverMetadataManager.hpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/SharedComponents \
	-I$(top_srcdir)/common_utilities

libParaverTraceParser_la_CPPFLAGS = @CLUSTERING_CPPFLAGS@
libParaverTraceParser_la_LDFLAGS  = @CLUSTERING_LDFLAGS@
libParaverTraceParser_la_LIBADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la \
	@CLUSTERING_LIBS@

