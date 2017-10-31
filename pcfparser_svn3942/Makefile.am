
# Include this files/directories for the package generation
EXTRA_DIST = UIParaverTraceConfig.ipp ParaverTraceConfig.ipp ParaverTraceConfigGrammar.ipp


lib_LTLIBRARIES = libparavertraceconfig.la

include_HEADERS = UIParaverTraceConfig.h UIParaverTraceConfig.ipp

if MAKE_OLD_PCFPARSER
  LIBPARAVERTRACECONFIG_COREFILES = \
    old/ParaverEventType.cpp old/ParaverEventType.h \
    old/ParaverEventValue.cpp old/ParaverEventValue.h \
    old/ParaverGradientColor.cpp old/ParaverGradientColor.h \
    old/ParaverGradientNames.cpp old/ParaverGradientNames.h \
    old/ParaverState.cpp old/ParaverState.h \
    old/ParaverStatesColor.cpp old/ParaverStatesColor.h \
    old/ParaverTraceConfig.cpp old/ParaverTraceConfig.h \
    old/ParaverTraceConfigGrammar.cpp old/ParaverTraceConfigGrammar.h
else
  LIBPARAVERTRACECONFIG_COREFILES= \
	debug.h \
	ParaverTraceConfigGrammar.h \
	UIParaverTraceConfig.cpp UIParaverTraceConfig.h \
	ParaverTraceConfig.cpp ParaverTraceConfig.h
endif

libparavertraceconfig_la_SOURCES  = $(LIBPARAVERTRACECONFIG_COREFILES)

libparavertraceconfig_la_CPPFLAGS = $(BOOST_CPPFLAGS)
libparavertraceconfig_la_LDFLAGS  =
libparavertraceconfig_la_LIBADD   =


