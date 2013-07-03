#ifndef _UIPARAVER_TRACE_CONFIG_PYTHON_H
#define _UIPARAVER_TRACE_CONFIG_PYTHON_H

#include "UIParaverTraceConfig.h"
#include <boost/python.hpp>

// Overladed methods
bool (libparaver::UIParaverTraceConfig::*parse1)(const std::string &, bool) = &libparaver::UIParaverTraceConfig::parse;
int (libparaver::UIParaverTraceConfig::*getEventType_int)(const std::string) const = &libparaver::UIParaverTraceConfig::getEventType;
std::string (libparaver::UIParaverTraceConfig::*getEventType_str)(const int) const = &libparaver::UIParaverTraceConfig::getEventType;

BOOST_PYTHON_MODULE(pcfparser)
{
    using namespace boost::python;

    class_<libparaver::UIParaverTraceConfig>("UIParaverTraceConfig")
        .def("getDebug", &libparaver::UIParaverTraceConfig::getDebug)
        .def("setDebug", &libparaver::UIParaverTraceConfig::getDebug)
        .def("getEventType_int", getEventType_int)
        .def("getEventType_str", getEventType_str)
        .def("parse", parse1);
}

#endif
