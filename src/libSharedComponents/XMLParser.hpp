/*****************************************************************************\
 *                        ANALYSIS PERFORMANCE TOOLS                         *
 *                             ClusteringSuite                               *
 *   Infrastructure and tools to apply clustering analysis to Paraver and    *
 *                              Dimemas traces                               *
 *                                                                           *
 *****************************************************************************
 *     ___     This library is free software; you can redistribute it and/or *
 *    /  __         modify it under the terms of the GNU LGPL as published   *
 *   /  /  _____    by the Free Software Foundation; either version 2.1      *
 *  /  /  /     \   of the License, or (at your option) any later version.   *
 * (  (  ( B S C )                                                           *
 *  \  \  \_____/   This library is distributed in hope that it will be      *
 *   \  \__         useful but WITHOUT ANY WARRANTY; without even the        *
 *    \___          implied warranty of MERCHANTABILITY or FITNESS FOR A     *
 *                  PARTICULAR PURPOSE. See the GNU LGPL for more details.   *
 *                                                                           *
 * You should have received a copy of the GNU Lesser General Public License  *
 * along with this library; if not, write to the Free Software Foundation,   *
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA          *
 * The GNU LEsser General Public License is contained in the file COPYING.   *
 *                                 ---------                                 *
 *   Barcelona Supercomputing Center - Centro Nacional de Supercomputacion   *
\*****************************************************************************/

/* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- *\

  $Id::                                           $:  Id
  $Rev::                                          $:  Revision of last commit
  $Author::                                       $:  Author of last commit
  $Date::                                         $:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#ifndef _XMLPARSER_H_
#define _XMLPARSER_H_

#include <types.h>
#include <Error.hpp>
using cepba_tools::Error;

#include <ClusteringConfiguration.hpp>

#include <UIParaverTraceConfig.h>

#include <map>
using std::map;
#include <string>
using std::string;

#include <libxml/parser.h>
#include <libxml/tree.h>

/* General strings */
#define STR_YES      "yes"
#define STR_NO       "no"

/* Duration related information */
#define ATTR_USEDURATION "use_duration"
#define ATTR_FILTER      "duration_filter"
#define ATTR_THRESHOLD   "threshold_filter"
#define ATTR_APPLYLOG    "apply_log"
#define ATTR_NORMALIZE   "normalize_data"
#define ATTR_ACC_PARAMS  "accumulate_parameters"

/* Node 'clustering_algorithm' */
#define NODE_CLUSTERING_ALGORITHM "clustering_algorithm"
// #define ATTR_EPS                  "epsilon"
// #define ATTR_MINPOINTS            "min_points"

/* Node types */
#define NODE_CL_PARAM    "clustering_parameters"
#define NODE_PR_PARAM    "extrapolation_parameters"

/* Attribute of 'extrapolation_parameters' */
#define ATTR_ALLCOUNTERS   "all_counters"

/* Node 'single_event' */
#define NODE_SINGLEEVT     "single_event"
#define NODE_EVENTTYPE     "event_type"
#define NODE_EVENTPOSITION "position"
#define NODE_FACTOR        "factor"
#define NODE_RANGE_MIN     "range_min"
#define NODE_RANGE_MAX     "range_max"

#define ATTR_NAME           "name"

/* Node 'mixed_events' */
#define NODE_MIXEDEVT        "mixed_events"
#define NODE_EVENTTYPE_A     "event_type_a"
#define NODE_EVENTPOSITION_A "position_a"
#define NODE_EVENTTYPE_B     "event_type_b"
#define NODE_EVENTPOSITION_B "position_b"
#define ATTR_OPERATION       "operation"

/* Node 'plot_options' */
#define NODE_PLOTS       "output_plots"
#define NODE_PLOT_DEF    "plot_definition"
#define ATTR_ALLPLOTS    "all_plots"
#define ATTR_RAWMETRICS  "raw_metrics"
#define ATTR_GNUPLOTVERS "gnuplot_vers"
#define NODE_XMETRIC     "x_metric"
#define NODE_YMETRIC     "y_metric"
#define NODE_ZMETRIC     "z_metric"
#define ATTR_TITLE       "title"
#define ATTR_METRIC_MIN  "min"
#define ATTR_METRIC_MAX  "max"

#define TEXT_DURATION    "duration"

class XMLParser: public Error
{
  private:

    bool                      PCFParserPresent;
    UIParaverTraceConfig      PCFParser;
    bool                      PCFParserError;
    string                    PCFParserErrorString;
    map<string, event_type_t> PCFEventsMap;
    map<event_type_t, string> PCFEventsReverseMap;

    bool   ClusteringAlgorithmError;
    string ClusteringAlgorithmErrorMessage;

    bool   ClusteringParametersError;
    string ClusteringParametersErrorMessage;

    bool   ExtrapolationParametersError;
    string ExtrapolationParametersErrorMessage;

    bool   PlotsDefinitionsError;
    string PlotsDefinitionsErrorMessage;

    bool  ReadingExtrapolationParameters;

    string             ClusteringAlgorithmName;
    map<string,string> ClusteringAlgorithmParameters;

    vector<string>             ClusteringParametersNames;
    vector<ParameterContainer> ClusteringParametersDefinitions;

    vector<string>             ExtrapolationParametersNames;
    vector<ParameterContainer> ExtrapolationParametersDefinitions;

    bool                       AllPlots;
    vector<PlotDefinition*>    PlotsDefinitions;

  public:
    XMLParser(void);

    bool ParseXML(string                   XMLFileName,
                  string                   PCFParserFileName,
                  ClusteringConfiguration* ExtractionManager);

  private:

    bool InitializePCFParser(string PCFFileName);

    bool ParseXMLNodes(xmlNodePtr CurrentNode);

    bool ParseXMLClusteringAlgorithm(xmlNodePtr CurrentClusteringAlgorithm);

    bool ParseXMLClusteringParameters(xmlNodePtr NodeClusteringParameters);
    bool ParseXMLExtrapolationParameters(xmlNodePtr NodeExtraParameters);

    bool ParseXMLSingleEvent(xmlNodePtr CurrentSingleEvent);
    bool ParseXMLMixedEvents(xmlNodePtr CurrentMixedEvent);

    bool ParseXMLOutputPlots(xmlNodePtr NodeOutputPlots);
    bool ParseXMLPlotDefinition(xmlNodePtr CurrentPlotDefinition);

    bool GenerateAllCountersExtrapolations(void);

    event_type_t FindEventTypeByName(string EventName);

    bool IsNumber(const string& String);

};

#endif // _DATAEXTRACTIONXMLPARSER_H_

