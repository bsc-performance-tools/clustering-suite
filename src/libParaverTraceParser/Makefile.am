noinst_LTLIBRARIES=libParaverTraceParser.la

common_filesdir = @common_filesdir@

INCLUDES = \
	-I$(top_srcdir)/include \
	-I$(common_filesdir) \
	-I$(top_srcdir)/src/BasicClasses \
	-I$(top_srcdir)/src/SharedComponents/ 

#libParaverTraceParser_la_LIBADD = \
#	$(top_srcdir)/src/BasicClasses/libBasicClasses.la

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

nodist_libParaverTraceParser_la_SOURCES=\
	$(common_filesdir)/Dimemas_Generation.c \
    $(common_filesdir)/EventEncoding.c \
    $(common_filesdir)/Dimemas2Prv.c
