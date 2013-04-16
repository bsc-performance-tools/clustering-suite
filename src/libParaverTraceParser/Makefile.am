noinst_LTLIBRARIES=libParaverTraceParser.la

## common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/SharedComponents \
	-I$(top_srcdir)/common_utilities

libParaverTraceParser_la_CXXFLAGS=@CXXFLAGS@

libParaverTraceParser_la_LIBADD = \
	$(top_srcdir)/common_utilities/libCommonUtilities.la

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
	ParaverTraceParser.hpp 

#nodist_libParaverTraceParser_la_SOURCES=\
#	$(common_filesdir)/Dimemas_Generation.c \
#    $(common_filesdir)/EventEncoding.c \
#    $(common_filesdir)/Dimemas2Prv.c
