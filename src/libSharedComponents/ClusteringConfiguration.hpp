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

#ifndef _CLUSTERINGCONFIGURATION_HPP_
#define _CLUSTERINGCONFIGURATION_HPP_

#include <Error.hpp>
using cepba_tools::Error;

#include "trace_clustering_types.h"

#include <vector>
using std::vector;
#include <map>
using std::map;

class ParameterContainer;
class PlotDefinition;

/**
 * Singleton container of all information present on the XML file needed to 
 * execute different aspects of clustering
 */
class ClusteringConfiguration: public Error
{
  private:
    bool            Initialized;

    bool            Distributed;     ///< Distributed version of the library
    INT32           MyRank;          ///< My Rank in MPI/distributed environment
    INT32           TotalRanks;      ///< Total number of workers in MPI/distributed environment

    duration_t      DurationFilter;  ///< Duration filter for short bursts

    percentage_t    FilterThreshold; ///< Threshold filter for small clusters
    bool            NormalizeData;   ///< Apply range data normalization

    sampling_type_t Sampling;           ///< Apply Sampling over the data
    percentage_t    SamplingPercentage; ///< Percentage of data points for sampling

    bool            AccumulateParameters; ///< Accumulates parameters values when reading a burst

    bool            AllPlots; ///< Print all plots combining clustering and extrapolation metrics

    /* Clustering Algorithm Information */
    string              ClusteringAlgorithmName;
    map<string, string> ClusteringAlgorithmParameters;
    bool                ClusteringAlgorithmError;
    string              ClusteringAlgorithmErrorMessage;

    /* Clustering Algorithm Dimensions */
    vector<string>             ClusteringParametersNames;
    vector<ParameterContainer> ClusteringParametersDefinitions;
    bool                       ClusteringParametersError;
    string                     ClusteringParametersErrorMessage;
    
    /* Extrapolation Dimensions */
    vector<string>             ExtrapolationParametersNames;
    vector<ParameterContainer> ExtrapolationParametersDefinitions;
    bool                       ExtrapolationParametersError;
    string                     ExtrapolationParametersErrorMessage;

    /* Plot scripts definition */
    vector<PlotDefinition*> PlotsDefinitions;
    bool                    PlotsDefinitionsError;
    string                  PlotsDefinitionsErrorMessage;

  private:
    static ClusteringConfiguration* _ClusteringConfiguration;

    ClusteringConfiguration(void);
    
  public:
    /* Instances getters */
    /* static ClusteringConfiguration* GetInstance(string XMLFileName,
                                                bool   ApplyCPIStack); */

    static ClusteringConfiguration* GetInstance(void);

    /* Initialization */
    bool   Initialize(string XMLFileName);

    bool   IsInitialized(void) const;
    
    /* Setters and Getters of general attributes */
    void       SetDistributed(bool _Distributed);
    bool       GetDistributed(void);
    void       SetMyRank(INT32 _MyRank);
    INT32      GetMyRank(void);
    void       SetTotalRanks(INT32 _TotalRanks);
    INT32      GetTotalRanks(void);
    void       SetDurationFilter(duration_t DurationFilter);
    duration_t GetDurationFilter(void);
    
    void         SetFilterThreshold(percentage_t FilterThreshold);
    percentage_t GetFilterThreshold(void);
    
    void SetNormalizeData(bool NormalizeData);
    bool GetNormalizeData(void);

    /*  Clustering algorithm getters and setters */
    void SetClusteringAlgorithm(string              ClusteringAlgorithmName,
                                map<string, string> ClusteringAlgorithmParameters);

    string                 GetClusteringAlgorithmName(void);
    map<string, string>&   GetClusteringAlgorithmParameters(void);

    void SetClusteringAlgorithmError(bool ClusteringAlgorithmError);
    bool GetClusteringAlgorithmError(void);

    void   SetClusteringAlgorithmErrorMessage(string ClusteringAlgorithmErrorMessage);
    string GetClusteringAlgorithmErrorMessage(void);

    /* Clustering Parameters management */
    void SetClusteringParameters(vector<string>&             ParametersNames,
                                 vector<ParameterContainer>& ParamatersDefinition);

    vector<string>&             GetClusteringParametersNames(void);
    vector<ParameterContainer>& GetClusteringParametersDefinitions(void);

    void SetClusteringParametersError(bool ClusteringParametersError);
    bool GetClusteringParametersError(void);

    void   SetClusteringParametersErrorMessage(string ClusteringParametersErrorMessage);
    string GetClusteringParametersErrorMessage(void);

    /* Extrapolation Parameters getters and setters */
    void SetExtrapolationParameters(vector<string>&            ParameterNames,
                                   vector<ParameterContainer>& ParametersDefinition);

    vector<string>&             GetExtrapolationParametersNames(void);
    vector<ParameterContainer>& GetExtrapolationParametersDefinitions(void);

    void SetExtrapolationParametersError(bool ExtrapolationParametersError);
    bool GetExtrapolationParametersError(void);

    void   SetExtrapolationParametersErrorMessage(string ExtrapolationParametersErrorMessage);
    string GetExtrapolationParametersErrorMessage(void);
    
    /* Output Plots definitions getters and setters */
    void                 SetAllPlots(bool AllPlots);
    bool                 GetAllPlots(void);
    
    void                     SetPlotsDefinitions(vector<PlotDefinition*>& PlotsDefinitions);
    vector<PlotDefinition*>& GetPlotsDefinitions(void);

    void SetPlotsDefinitionsError(bool PlotsDefinitionsError);
    bool GetPlotsDefinitionsError(void);

    void   SetPlotsDefinitionsErrorMessage(string PlotsDefinitionsErrorMessage);
    string GetPlotsDefinitionsErrorMessage(void);
    
    /* Checkers */
    bool CheckParameters(void);
    bool CheckClusteringAlgorithm(void);
    bool CheckGNUPlot(void);
};

class ParameterContainer
{
  public:
    parameter_type_t ParameterType;
    event_type_t     EventType;
    event_type_t     EventType2;
    bool             ApplyLog;
    char             Operation;
    double           Factor;
    double           RangeMin;
    double           RangeMax;
    INT32            Line;

    ParameterContainer(void)
    {
      ParameterType = UndefinedParameter;
      EventType     = 0;
      EventType2    = 0;
      ApplyLog      = false;
      Operation     = 'E';
      Factor        = 1.0;
      RangeMin      = -1.0;
      RangeMax      = -1.0;
    }
};

class PlotDefinition
{
  public:
    bool ThreeDimensions;

    string PlotTitle;

    bool   RawMetrics;

    string XMetric;
    string XMetricTitle;
    INT32  XMetricPosition;
    double XMin;
    double XMax;

    string YMetric;
    string YMetricTitle;
    INT32  YMetricPosition;
    double YMin;
    double YMax;

    string ZMetric;
    string ZMetricTitle;
    INT32  ZMetricPosition;
    double ZMin;
    double ZMax;

    line_t Line;
    
    PlotDefinition(void)
    {
      ThreeDimensions = false;
      RawMetrics      = true;

      PlotTitle       = UNDEFINED;

      XMetric         = UNDEFINED;
      XMetricTitle    = UNDEFINED;
      XMetricPosition = NOT_FOUND;
      XMin            = MIN_DOUBLE;
      XMax            = MAX_DOUBLE;

      YMetric         = UNDEFINED;
      YMetricTitle    = UNDEFINED;
      YMetricPosition = NOT_FOUND;
      YMin            = MIN_DOUBLE;
      YMax            = MAX_DOUBLE;

      ZMetric         = UNDEFINED;
      ZMetricTitle    = UNDEFINED;
      ZMetricPosition = NOT_FOUND;
      ZMin            = MIN_DOUBLE;
      ZMax            = MAX_DOUBLE;

      Line            = 0;
    }
};

#endif
