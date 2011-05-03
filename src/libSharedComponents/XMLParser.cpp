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

  $URL:: https://svn.bsc.#$:  File
  $Rev:: 20               $:  Revision of last commit
  $Author:: jgonzale      $:  Author of last commit
  $Date:: 2010-03-09 17:1#$:  Date of last commit

\* -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- */

#include "XMLParser.hpp"

#include <iostream>
using std::cout;
using std::endl;

#include <string.h>
#include <errno.h>
#include <math.h>

/*
class ParameterContainer
{
  public:
    INT32  ParameterType;
    INT32  EventType;
    INT32  EventType2;
    bool   ApplyLog;
    INT32  Operation;
    double Factor;
    double RangeMin;
    double RangeMax;
};
*/

/**
 * Default constructor
 */
XMLParser::XMLParser(void)
{
  ClusteringAlgorithmError        = false;
  ClusteringAlgorithmErrorMessage = "";
  
  ClusteringParametersError        = false;
  ClusteringParametersErrorMessage = "";
    
  ExtrapolationParametersError        = false;
  ExtrapolationParametersErrorMessage = "";
    
  PlotsDefinitionsError        = false;
  PlotsDefinitionsErrorMessage = "";
}

/**
 * Runs the XML parsing at top level node
 * \param XMLFileName String containing the name of the XML file
 * \param Configuration ClusteringConfiguration object where all information will be stored
 */
bool
XMLParser::ParseXML(string                   XMLFileName,
                    ClusteringConfiguration* Configuration)
{
  xmlDoc  *XMLDocument = NULL;
  xmlNode *RootElement = NULL;

  char  *AuxCharStr;
  string UseDurationStr, ApplyLogStr, NormalizeDataStr;

  ParameterContainer NewParameter;
  
  bool         UseDuration;
  bool         ApplyLogToDuration;
  duration_t   DurationFilter;
  percentage_t FilterThreshold;
  bool         NormalizeData;

  AllPlots = false;
  
  FILE* XMLFile;

  if ( (XMLFile = fopen(XMLFileName.c_str(), "r")) == NULL)
  {
    SetErrorMessage("unable to open XML definition file",
                    strerror(errno));
    SetError(true);
    return false;
  }
  fclose(XMLFile);

  /*
   * this initialize the library and check potential ABI mismatches
   * between the version it was compiled for and the actual shared
   * library used.
   */
  LIBXML_TEST_VERSION

  /* Parse the file and get the DOM */
  XMLDocument = xmlParseFile(XMLFileName.c_str());
  if (XMLDocument == NULL)
  {
    SetErrorMessage("Error parsing clustering definition file");
    SetError(true);
    return false;
  }

  RootElement = xmlDocGetRootElement(XMLDocument);

  /* Check attributes of 'clustering_definition' element: */
  {
  /* 1. ATTR_USEDURATION -> UseDuration */
  AuxCharStr =
    (char*) xmlGetProp(RootElement, ((const xmlChar *) ATTR_USEDURATION));

  if (AuxCharStr == NULL)
    UseDuration = false;
  else
  {
    UseDurationStr = AuxCharStr;
    xmlFree(AuxCharStr);

    if (UseDurationStr.compare(STR_YES) == 0)
    {
      UseDuration = true;

      /* 2. ATTR_APPLYLOG    -> ApplyLogToDuration */
      AuxCharStr =
        (char*) xmlGetProp(RootElement, ((const xmlChar *) ATTR_APPLYLOG));

      if (AuxCharStr == NULL)
        ApplyLogToDuration = false;
      else
      {
        ApplyLogStr = AuxCharStr;
        xmlFree(AuxCharStr);

        if (ApplyLogStr.compare(STR_YES) == 0)
          ApplyLogToDuration = true;
        else
          ApplyLogToDuration = false;
      }
    }
    else
    {
      UseDuration        = false;
      ApplyLogToDuration = false;
    }
  }

  /* 3. ATTR_FILTER      -> DurationFilter */
  AuxCharStr =
    (char*) xmlGetProp(RootElement, (const xmlChar *) ATTR_FILTER);

  if (AuxCharStr == NULL)
    DurationFilter = DEFAULT_DURATION_FILTER;
  else
  {
    DurationFilter = strtoull(AuxCharStr, NULL, 10);
    DurationFilter = llround(DurationFilter*1e3);    /* Duration filter in nanosecs. */
    xmlFree(AuxCharStr);
  }

  /* 4. ATTR_NORMALIZE   -> NormalizeData */
  AuxCharStr =
    (char*) xmlGetProp(RootElement, (const xmlChar*) ATTR_NORMALIZE);

  if (AuxCharStr == NULL)
    NormalizeData = true;
  else
  {
    NormalizeDataStr = AuxCharStr;
    xmlFree(AuxCharStr);

    if (NormalizeDataStr.compare(STR_NO) == 0)
      NormalizeData = false;
    else
      NormalizeData = true;
  }
    
  /* 5. ATTR_THRESHOLD   -> Threshold Filter */
  AuxCharStr =
    (char*) xmlGetProp(RootElement, (const xmlChar *) ATTR_THRESHOLD);

  if (AuxCharStr == NULL)
    FilterThreshold = DEFAULT_FILTER_THRESHOLD;
  else
  {
    FilterThreshold = atof(AuxCharStr);

    if (FilterThreshold < 0 || FilterThreshold > 100.0)
    {
      cout << "Threshold Filter must be a value between 0 and 100%. ";
      cout << "Using default value (";
      cout << DEFAULT_FILTER_THRESHOLD << "%)" << endl;
      FilterThreshold = DEFAULT_FILTER_THRESHOLD;
    }

    xmlFree(AuxCharStr);
  }
  }

/*
  INT32  ParameterType;
  INT32  EventType;
  INT32  EventType2;
  bool   ApplyLog;
  INT32  Operation;
  double Factor;
  double RangeMin;
  double RangeMax;
*/
  
  if (UseDuration)
  {
    NewParameter.ParameterType = SingleEventParameter;
    NewParameter.EventType     = DURATION_EVENT_TYPE;
    NewParameter.ApplyLog      = ApplyLogToDuration;

    ClusteringParametersNames.push_back("Duration");
    ClusteringParametersDefinitions.push_back(NewParameter);
  }

  if (!ParseXMLNodes(RootElement->xmlChildrenNode))
  {
    return false;
  }
  else
  {
    Configuration->SetNormalizeData(NormalizeData);
    Configuration->SetDurationFilter(DurationFilter);

    /*
    if (ClusteringParametersDefinitions.size() == 0)
    {
      SetErrorMessage("at least one clustering parameter must be defined");
      SetError(true);
      return false;
    }
    */


    if (!ClusteringAlgorithmError)
    {
      Configuration->SetClusteringAlgorithmError(false);
      Configuration->SetClusteringAlgorithm(ClusteringAlgorithmName,
                                            ClusteringAlgorithmParameters);
    }
    else
    {
      Configuration->SetClusteringAlgorithmError(true);
      Configuration->SetClusteringAlgorithmErrorMessage(ClusteringAlgorithmErrorMessage);
    }
    
    if (!ClusteringParametersError)
    {
      Configuration->SetClusteringParametersError(false);
      Configuration->SetClusteringParameters(ClusteringParametersNames,
                                             ClusteringParametersDefinitions);
    }
    else
    {
      Configuration->SetClusteringParametersError(true);
      Configuration->SetClusteringParametersErrorMessage(ClusteringParametersErrorMessage);
    }

    if (!ExtrapolationParametersError)
    {
      Configuration->SetExtrapolationParametersError(false);
      Configuration->SetExtrapolationParameters(ExtrapolationParametersNames,
                                                ExtrapolationParametersDefinitions);
    }
    else
    {
      Configuration->SetExtrapolationParametersError(true);
      Configuration->SetExtrapolationParametersErrorMessage(ExtrapolationParametersErrorMessage);
    }

    if (!PlotsDefinitionsError)
    {
      Configuration->SetPlotsDefinitionsError(false);
      Configuration->SetAllPlots(AllPlots);
      if (!AllPlots)
      {
        Configuration->SetPlotsDefinitions(PlotsDefinitions);
      }
    }
    else
    {
      Configuration->SetPlotsDefinitionsError(true);
      Configuration->SetPlotsDefinitionsErrorMessage(PlotsDefinitionsErrorMessage);
    }
  }

  return true;
}

bool
XMLParser::ParseXMLNodes(xmlNodePtr CurrentNode)
{
  INT32 ClusteringParametersCount;

  bool ClusteringAlgorithmRead = false;
  xmlNodePtr CurrentChild;
  string     CurrentNodeName;
  int i = 0;

  CurrentChild = CurrentNode;
  while(CurrentChild != NULL)
  {
    i++;
    CurrentNodeName = (const char*) CurrentChild->name;

    if (CurrentNodeName.compare(NODE_CLUSTERING_ALGORITHM) == 0)
    {
      if (!ParseXMLClusteringAlgorithm(CurrentChild))
      {
        ClusteringAlgorithmError        = true;
        ClusteringAlgorithmErrorMessage = GetLastError();
      }
    }
    else if (CurrentNodeName.compare(NODE_CL_PARAM) == 0)
    {
      if (!ParseXMLClusteringParameters(CurrentChild))
      {
        ClusteringParametersError = true;
        ClusteringParametersErrorMessage = GetLastError();
      }
    }
    else if (CurrentNodeName.compare(NODE_PR_PARAM) == 0)
    {
      if (!ParseXMLExtrapolationParameters(CurrentChild))
      {
        ExtrapolationParametersError        = true;
        ExtrapolationParametersErrorMessage = GetLastError();
      }
    }
    else if (CurrentNodeName.compare(NODE_PLOTS) == 0)
    {
      if(!ParseXMLOutputPlots(CurrentChild))
      {
        PlotsDefinitionsError        = true;
        PlotsDefinitionsErrorMessage = GetLastError();
      }
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "Unknown node \"" + CurrentNodeName + "\" on line ";
      sprintf(line, "%d", CurrentChild->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
    
    CurrentChild = CurrentChild->next;
  }

  return true;
}

bool
XMLParser::ParseXMLClusteringAlgorithm(xmlNodePtr CurrentClusteringAlgorithm)
{
  xmlNodePtr CurrentAlgorithmParameter;
  string     CurrentAlgorithmParameterName;
  string     CurrentAlgorithmParameterValue;
  
  char*  AuxCharStr;
  string AuxStr;

  /* Read Name */
  AuxCharStr =
    (char*) xmlGetProp(CurrentClusteringAlgorithm, (const xmlChar*) ATTR_NAME);

  if (AuxCharStr != NULL)
  {
    ClusteringAlgorithmName = AuxCharStr;
    xmlFree(AuxCharStr);
  }
  else
  {
    char line[15];
    string ErrorMessage;
    ErrorMessage += "clustering algorithm name not defined on line";
    sprintf(line, "%d", CurrentClusteringAlgorithm->line);
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  for (CurrentAlgorithmParameter  = CurrentClusteringAlgorithm->xmlChildrenNode;
       CurrentAlgorithmParameter != NULL;
       CurrentAlgorithmParameter  = CurrentAlgorithmParameter->next)
  {
    CurrentAlgorithmParameterName = (const char*) CurrentAlgorithmParameter->name;

    if (CurrentAlgorithmParameterName.compare("text") != 0 && 
        CurrentAlgorithmParameterName.compare("comment") != 0)
    {
    
      AuxCharStr = (char*) xmlNodeGetContent(CurrentAlgorithmParameter);
    
      if (AuxCharStr != NULL)
      {
        CurrentAlgorithmParameterValue = (const char*) AuxCharStr;
        xmlFree(AuxCharStr);
      }
      else
      {
        char line[15];
        string ErrorMessage;
        ErrorMessage += "empty clustering parameter \"" + CurrentAlgorithmParameterName;
        ErrorMessage += "\" on line ";
        sprintf(line, "%d", CurrentAlgorithmParameter->line);
        ErrorMessage += line;

        SetErrorMessage(ErrorMessage);
        SetError(true);
        return false;
      }

      ClusteringAlgorithmParameters.insert(make_pair(CurrentAlgorithmParameterName,
                                                     CurrentAlgorithmParameterValue));
    }
  }
  
  return true;
}

bool
XMLParser::ParseXMLClusteringParameters(
  xmlNodePtr NodeClusteringParameters)
{
  xmlNodePtr CurrentNode;
  string     CurrentNodeName;

  ReadingExtrapolationParameters = false;

  for (CurrentNode  = NodeClusteringParameters->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_SINGLEEVT) == 0)
    {
      if (!ParseXMLSingleEvent(CurrentNode))
        return false;
    }
    else if (CurrentNodeName.compare(NODE_MIXEDEVT) == 0)
    {
      if (!ParseXMLMixedEvents(CurrentNode))
        return false;
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "Unknown or unexpected node \"" + CurrentNodeName;
      ErrorMessage += "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  return true;
}

bool XMLParser::ParseXMLExtrapolationParameters(xmlNodePtr NodeExtraParameters)
{
  xmlNodePtr CurrentNode;
  string     CurrentNodeName;
  int i = 0;

  ReadingExtrapolationParameters = true;
  
  for (CurrentNode  = NodeExtraParameters->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_SINGLEEVT) == 0)
    {
      if (!ParseXMLSingleEvent(CurrentNode))
        return false;
    }
    else if (CurrentNodeName.compare(NODE_MIXEDEVT) == 0)
    {
      if (!ParseXMLMixedEvents(CurrentNode))
        return false;
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "Unknown or unexpected node \"" + CurrentNodeName;
      ErrorMessage += "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }
  
  return true;
}

bool
XMLParser::ParseXMLSingleEvent(xmlNodePtr CurrentSingleEvent)
{
  
  ParameterContainer NewParameter;
  
  xmlNodePtr   CurrentNode;
  string       CurrentNodeName;

  char*        AuxCharStr;
  string       AuxStr;

  string       ParameterName;
  bool         ApplyLog = false;
  event_type_t EventType;
  double       RangeMin = -1.0, RangeMax = -1.0;
  bool         EventTypeRead = false;
  double       Factor = 1.0;

  NewParameter.ParameterType = SingleEventParameter;
  
  AuxCharStr =
    (char*) xmlGetProp(CurrentSingleEvent, (const xmlChar*) ATTR_NAME);

  if (AuxCharStr == NULL)
  {
    char line[15];
    string ErrorMessage;

    sprintf (line, "%d", CurrentSingleEvent->line);

    ErrorMessage += "Missing attribute \"";
    ErrorMessage += ATTR_NAME;
    ErrorMessage += "\" from \"";
    ErrorMessage += NODE_SINGLEEVT;
    ErrorMessage += "\" node on line ";
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  ParameterName = AuxCharStr;
  xmlFree(AuxCharStr);

  AuxCharStr =
    (char*) xmlGetProp(CurrentSingleEvent, (const xmlChar*) ATTR_APPLYLOG);

  if (AuxCharStr != NULL)
  {
    AuxStr = AuxCharStr;
    xmlFree(AuxCharStr);
    if (AuxStr.compare(STR_YES) == 0)
      ApplyLog = true;
  }

  for (CurrentNode  = CurrentSingleEvent->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_EVENTTYPE) == 0)
    {
      string EventTypeStr = (const char*) xmlNodeGetContent(CurrentNode);
      
      if (EventTypeStr.compare(TEXT_DURATION) == 0)
      {
        EventType = DURATION_EVENT_TYPE;
      }
      else
      {
        EventType  = (event_type_t) atoll((char*) xmlNodeGetContent(CurrentNode));
      }
      EventTypeRead = true;
    }
    else if (CurrentNodeName.compare(NODE_FACTOR) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        Factor = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare(NODE_RANGE_MIN) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        RangeMin = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare(NODE_RANGE_MAX) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        RangeMax = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "Unknown node \"" + CurrentNodeName + "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  if (!EventTypeRead)
  {
    string ErrorMessage;

    ErrorMessage += "Mandatory element \"";
    ErrorMessage += NODE_EVENTTYPE;
    ErrorMessage += "\" not found on \"";
    ErrorMessage += NODE_SINGLEEVT;
    ErrorMessage += "\" node";

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  NewParameter.EventType = EventType;
  NewParameter.Factor    = Factor;
  NewParameter.ApplyLog  = ApplyLog;
  NewParameter.RangeMin  = RangeMin;
  NewParameter.RangeMax  = RangeMax;
  NewParameter.Line      = CurrentSingleEvent->line;
  
  if (ReadingExtrapolationParameters)
  {
    ExtrapolationParametersNames.push_back(ParameterName);
    ExtrapolationParametersDefinitions.push_back(NewParameter);
  }
  else
  {
    ClusteringParametersNames.push_back(ParameterName);
    ClusteringParametersDefinitions.push_back(NewParameter);
  }

  return true;
}


bool
XMLParser::ParseXMLMixedEvents(xmlNodePtr CurrentMixedEvent)
{
  ParameterContainer NewParameter;
  xmlNodePtr         CurrentNode;
  string             CurrentNodeName;

  char*        AuxCharStr;
  string       AuxStr;

  string       ParameterName;
  bool         ApplyLog = false;
  char         Operation;
  event_type_t EventTypeA, EventTypeB;
  bool         EventTypeARead = false, EventTypeBRead = false;
  double       Factor = 1.0, RangeMin = -1.0, RangeMax = -1.0;

  AuxCharStr =
    (char*) xmlGetProp(CurrentMixedEvent, (const xmlChar*) ATTR_NAME);

  NewParameter.ParameterType = DerivedEventParameter;
  
  if (AuxCharStr == NULL)
  {
    char line[15];
    string ErrorMessage;

    sprintf (line, "%d", CurrentMixedEvent->line);

    ErrorMessage += "Missing attribute \"";
    ErrorMessage += ATTR_NAME;
    ErrorMessage += "\" from \"";
    ErrorMessage += NODE_MIXEDEVT;
    ErrorMessage += "\" node on line ";
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  ParameterName = AuxCharStr;
  xmlFree(AuxCharStr);

  AuxCharStr =
    (char*) xmlGetProp(CurrentMixedEvent, (const xmlChar*) ATTR_OPERATION);

  if (AuxCharStr == NULL)
  {
    char line[15];
    string ErrorMessage;

    sprintf (line, "%d", CurrentMixedEvent->line);

    ErrorMessage += "Missing attribute \"";
    ErrorMessage += ATTR_OPERATION;
    ErrorMessage += "\" from \"";
    ErrorMessage += NODE_MIXEDEVT;
    ErrorMessage += "\" node on line ";
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  Operation = AuxCharStr[0];
  xmlFree(AuxCharStr);

  AuxCharStr =
    (char*) xmlGetProp(CurrentMixedEvent, (const xmlChar*) ATTR_APPLYLOG);

  if (AuxCharStr != NULL)
  {
    AuxStr = AuxCharStr;
    xmlFree(AuxCharStr);
    if (AuxStr.compare(STR_YES) == 0)
      ApplyLog = true;
  }

  for (CurrentNode  = CurrentMixedEvent->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_EVENTTYPE_A) == 0)
    {
      string EventTypeStr = (const char*) xmlNodeGetContent(CurrentNode);
      if (EventTypeStr.compare(TEXT_DURATION) == 0)
      {
        EventTypeA = DURATION_EVENT_TYPE;
      }
      else
      {
        EventTypeA     = (event_type_t) atoll((char*) xmlNodeGetContent(CurrentNode));
      }
      EventTypeARead = true;
    }
    else if (CurrentNodeName.compare(NODE_EVENTTYPE_B) == 0)
    {
      string EventTypeStr = (const char*) xmlNodeGetContent(CurrentNode);
      if (EventTypeStr.compare(TEXT_DURATION) == 0)
      {
        EventTypeB = DURATION_EVENT_TYPE;
      }
      else
      {
        EventTypeB     = (event_type_t) atoll((char*) xmlNodeGetContent(CurrentNode));
      }
      EventTypeBRead = true;
    }
    else if (CurrentNodeName.compare(NODE_FACTOR) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        Factor = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare(NODE_RANGE_MIN) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        RangeMin = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare(NODE_RANGE_MAX) == 0)
    {
      if (xmlNodeGetContent(CurrentNode) != NULL)
      {
        RangeMax = atof((char*) xmlNodeGetContent(CurrentNode));
      }
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "Unknown node \"" + CurrentNodeName + "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  if (!EventTypeARead)
  {
    char line[15];
    string ErrorMessage;

    sprintf(line, "%d", CurrentMixedEvent->line);

    ErrorMessage += "Mandatory element \"";
    ErrorMessage += NODE_EVENTTYPE_A;
    ErrorMessage += "\" not found on \"";
    ErrorMessage += NODE_MIXEDEVT;
    ErrorMessage += "\" node on line ";
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }

  if (!EventTypeBRead)
  {
    char line[15];
    string ErrorMessage;

    sprintf(line, "%d", CurrentMixedEvent->line);
    
    ErrorMessage = "Mandatory element \"";
    ErrorMessage += NODE_EVENTTYPE_B;
    ErrorMessage += "\" not found on \"";
    ErrorMessage += NODE_MIXEDEVT;
    ErrorMessage += "\" node on line ";
    ErrorMessage += line;

    SetErrorMessage(ErrorMessage);
    SetError(true);
    return false;
  }


  NewParameter.EventType  = EventTypeA;
  NewParameter.EventType2 = EventTypeB;
  NewParameter.Factor     = Factor;
  NewParameter.ApplyLog   = ApplyLog;
  NewParameter.Operation  = Operation;
  NewParameter.RangeMin   = RangeMin;
  NewParameter.RangeMax   = RangeMax;
  NewParameter.Line       = CurrentMixedEvent->line;
  
  /* Set range if available */
  if (ReadingExtrapolationParameters)
  {
    ExtrapolationParametersNames.push_back(ParameterName);
    ExtrapolationParametersDefinitions.push_back(NewParameter);
  }
  else
  {
    ClusteringParametersNames.push_back(ParameterName);
    ClusteringParametersDefinitions.push_back(NewParameter);
  }

  return true;
}

bool
XMLParser::ParseXMLOutputPlots(xmlNodePtr NodeOutputPlots)
{
  xmlNodePtr CurrentNode;
  string     CurrentNodeName;

  char*      AuxCharStr;
  string     AuxStr;


  AuxCharStr =
    (char*) xmlGetProp(NodeOutputPlots, (const xmlChar*) ATTR_ALLPLOTS);
  
  if (AuxCharStr != NULL)
  {
    AuxStr = AuxCharStr;
    xmlFree(AuxCharStr);
    
    if (AuxStr.compare(STR_YES) == 0)
    {
      AllPlots = true;
      return true;
    }
  }
  
  for (CurrentNode  = NodeOutputPlots->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_PLOT_DEF) == 0)
    {
      if (!ParseXMLPlotDefinition(CurrentNode))
        return false;
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "unknown or unexpected node \"" + CurrentNodeName;
      ErrorMessage += "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetErrorMessage(ErrorMessage);
      SetError(true);
      return false;
    }
  }

  return true;
}

bool
XMLParser::ParseXMLPlotDefinition(xmlNodePtr CurrentPlotDefinition)
{
  xmlNodePtr      CurrentNode;
  string          CurrentNodeName;

  PlotDefinition* Definition = new PlotDefinition();
  
  bool       XMetricPresent, YMetricPresent, ZMetricPresent, RawMetrics;
  char*      AuxCharStr;
  string     AuxStr, AllPlotsStr;

  XMetricPresent = YMetricPresent = ZMetricPresent = false;
  RawMetrics     = true;
  
  /* Print Raw dimensions */
  AuxCharStr =
    (char*) xmlGetProp(CurrentPlotDefinition, (const xmlChar*) ATTR_RAWMETRICS);

  Definition->Line = CurrentPlotDefinition->line;
  
  if (AuxCharStr != NULL)
  {
    AllPlotsStr = AuxCharStr;
    xmlFree(AuxCharStr);

    if (AllPlotsStr.compare(STR_NO) == 0)
    {
      Definition->RawMetrics = false;
    }
  }
  
  for (CurrentNode  = CurrentPlotDefinition->xmlChildrenNode;
       CurrentNode != NULL;
       CurrentNode  = CurrentNode->next)
  {
    CurrentNodeName = (const char*) CurrentNode->name;

    if (CurrentNodeName.compare(NODE_XMETRIC) == 0)
    { /* X Axis Metric */
      if (XMetricPresent)
      {
        char   line[15];
        string ErrorMessage;
        
        ErrorMessage += "double definition of plot X axis on line ";
        sprintf(line, "%d", CurrentNode->line);
        ErrorMessage += line;

        SetError(true);
        SetErrorMessage(ErrorMessage);
        return false;
      }

      AuxCharStr = (char*) xmlNodeGetContent(CurrentNode);
      Definition->XMetric    = AuxCharStr;
      xmlFree(AuxCharStr);

      XMetricPresent = true;
      
      /* Get parameter "title" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_TITLE);

      if (AuxCharStr != NULL)
      {
        Definition->XMetricTitle = AuxCharStr;
        xmlFree(AuxCharStr);
      }
      else
      {
        Definition->XMetricTitle = Definition->XMetric;
      }

      /* Get parameter "min" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MIN);

      if (AuxCharStr != NULL)
      {
        Definition->XMin = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }

      /* Get parameter "max" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MAX);

      if (AuxCharStr != NULL)
      {
        Definition->XMax = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }
    }
    else if (CurrentNodeName.compare(NODE_YMETRIC) == 0)
    { /* Y Axis Metric */
      if (YMetricPresent)
      {
        char   line[15];
        string ErrorMessage;
        
        ErrorMessage += "double definition of plot Y axis on line ";
        sprintf(line, "%d", CurrentNode->line);
        ErrorMessage += line;

        SetError(true);
        SetErrorMessage(ErrorMessage);
        return false;
      }

      AuxCharStr = (char*) xmlNodeGetContent(CurrentNode);
      Definition->YMetric = AuxCharStr;
      xmlFree(AuxCharStr);

      YMetricPresent = true;

      /* Get parameter "title" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_TITLE);

      if (AuxCharStr != NULL)
      {
        Definition->YMetricTitle = AuxCharStr;
        xmlFree(AuxCharStr);
      }
      else
      {
        Definition->YMetricTitle = Definition->YMetric;
      }

      /* Get parameter "min" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MIN);

      if (AuxCharStr != NULL)
      {
        Definition->YMin = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }

      /* Get parameter "max" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MAX);

      if (AuxCharStr != NULL)
      {
        Definition->YMax = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }
    }
    else if (CurrentNodeName.compare(NODE_ZMETRIC) == 0)
    { /* Z Axis Metric */
      if (ZMetricPresent)
      {
        char   line[15];
        string ErrorMessage;
        
        ErrorMessage += "double definition of plot Z axis on line ";
        sprintf(line, "%d", CurrentNode->line);
        ErrorMessage += line;

        SetError(true);
        SetErrorMessage(ErrorMessage);
        return false;
      }

      AuxCharStr = (char*) xmlNodeGetContent(CurrentNode);
      Definition->ZMetric = AuxCharStr;
      xmlFree(AuxCharStr);

      ZMetricPresent = true;

      /* Get parameter "title" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_TITLE);

      if (AuxCharStr != NULL)
      {
        Definition->ZMetricTitle = AuxCharStr;
        xmlFree(AuxCharStr);
      }
      else
      {
        Definition->ZMetricTitle = Definition->ZMetric;
      }

      /* Get parameter "min" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MIN);

      if (AuxCharStr != NULL)
      {
        Definition->ZMin = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }

      /* Get parameter "max" */
      AuxCharStr =
        (char*) xmlGetProp(CurrentNode, (const xmlChar*) ATTR_METRIC_MAX);

      if (AuxCharStr != NULL)
      {
        Definition->ZMax = atof(AuxCharStr);
        xmlFree(AuxCharStr);
      }
      
      Definition->ThreeDimensions = true;
    }
    else if (CurrentNodeName.compare("text") != 0 &&
             CurrentNodeName.compare("comment") != 0)
    {
      char line[15];
      string ErrorMessage;
      ErrorMessage += "unknown node \"" + CurrentNodeName + "\" on line ";
      sprintf(line, "%d", CurrentNode->line);
      ErrorMessage += line;

      SetError(true);
      SetErrorMessage(ErrorMessage);
      return false;
    }
  }

  if (!XMetricPresent || !YMetricPresent)
  {
    char line[15];
    string ErrorMessage;

    ErrorMessage += "X or Y dimension missing in the plot defined on line ";
    sprintf(line, "%d", CurrentPlotDefinition->line);
    ErrorMessage += line;
    SetError(true);
    SetErrorMessage(ErrorMessage);
    return false;
  }
  
  PlotsDefinitions.push_back(Definition);

  return true;
}
