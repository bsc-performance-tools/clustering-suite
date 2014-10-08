
# Include this files/directories for the package generation
EXTRA_DIST = UIParaverTraceConfig.ipp ParaverTraceConfig.ipp ParaverTraceConfigGrammar.ipp


lib_LTLIBRARIES = libparavertraceconfig.la

include_HEADERS = UIParaverTraceConfig.h UIParaverTraceConfig.ipp

LIBPARAVERTRACECONFIG_COREFILES= \
	debug.h \
	ParaverTraceConfigGrammar.h \
	UIParaverTraceConfig.cpp UIParaverTraceConfig.h \
	ParaverTraceConfig.cpp ParaverTraceConfig.h

libparavertraceconfig_la_SOURCES = $(LIBPARAVERTRACECONFIG_COREFILES)

libparavertraceconfig_la_CPPFLAGS = $(BOOST_CPPFLAGS)
libparavertraceconfig_la_LDFLAGS  =
libparavertraceconfig_la_LIBADD  =



