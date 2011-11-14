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

#include "trace_clustering_types.h"

#include <SystemMessages.hpp>
using cepba_tools::system_messages;

#include <ClusteringConfiguration.hpp>
#include <ParaverColors.h>

#include "PlottingManager.hpp"


#include <fstream>
using std::ofstream;

#include <fstream>
using std::ofstream;
using std::ios_base;

#include <sstream>
using std::ostringstream;

using std::cout;
using std::endl;

#include <algorithm>
using std::sort;

#include <cerrno>
#include <cstring>


/******************************************************************************
 * Singleton pointer
 *****************************************************************************/

/**
 * Singleton pointer
 */
PlottingManager* PlottingManager::_PlottingManager = NULL;

/******************************************************************************
 * GetInstance methods
 *****************************************************************************/

/**
 * Returns the single instance of the plotting manager and creates it if doesn't
 * exists
 * \param  Boolean indicating if data to plot is clustered or just extracted from a file
 * \return The plotting manager singleton instance
 */
PlottingManager*
PlottingManager::GetInstance(bool DataExtraction)
{
  /* DEBUG
  cout << "GETTING A PLOT INSTANCE WITH DataExtraction SET TO " << DataExtraction << endl; */
  if (PlottingManager::_PlottingManager == NULL)
  {
    PlottingManager::_PlottingManager = new PlottingManager(DataExtraction);
  }

  return PlottingManager::_PlottingManager;
}

/**
 * Returns the single instance of the plotting manager, *do not create it* if
 * it does not exists
 *
 * \return Plotting manager if it exists. NULL otherwise.
 *
 */
PlottingManager*
PlottingManager::GetInstance(void)
{
  return PlottingManager::_PlottingManager;
}

/******************************************************************************
 * State consults
 *****************************************************************************/

vector<bool>
PlottingManager::GetCorrectPlots(void)
{
  return this->CorrectPlots;
}

vector<string>
PlottingManager::GetPlotsWarning(void)
{
  return this->PlotsWarnings;
}

/******************************************************************************
 * Print Plots method, main method of the class
 *****************************************************************************/

/**
 * Print all plots correctly defined
 * 
 * \param PlotsDataFileName   Name of the file where data plots data will be located
 * \param PlotsFileNamePrefix Prefix to be used in the resulting .gnuplot files 
 * \param Title          Title to be used in the plots
 * \param DifferentIDs Set with the different IDs present in the data
 * 
 * \return True if all plots have been written correctly, false otherwise
 */
bool PlottingManager::PrintPlots(string             PlotsDataFileName,
                                 string             PlotsFileNamePrefix,
                                 string             Title,
                                 set<cluster_id_t>& DifferentIDs,
                                 bool               PrintingModels)
{

  // system_messages::show_progress("Writing plots to disc", 0, Plots.size());
  
  this->PrintingModels = PrintingModels;
  
  for (size_t i = 0; i < Plots.size(); i++)
  {
    ostringstream PlotMessage;
    PlotMessage << "Printing plot " << i+1 << ": ";
    system_messages::information(PlotMessage.str().c_str());
    
    if(CorrectPlots[i])
    {
      /* DEBUG: Temporary, raw data not available in model plots */
      if (PrintingModels && Plots[i]->RawMetrics)
      { 
        continue;
      }

      if (!PrintSinglePlot (PlotsFileNamePrefix,
                            PlotsDataFileName,
                            Plots[i],
                            Title,
                            DifferentIDs))
      {
        SetError(true);
        return false;
      }
    }
    else
    {
      ostringstream PlotWarning;
      PlotWarning << "wrong plot (" << PlotsWarnings[i] << ")" << endl;
      system_messages::information(PlotWarning.str().c_str());
    }
    // system_messages::show_progress("Writing plots to disc", i, Plots.size());
  }
  // system_messages::show_progress_end("Writing plots to disc", Plots.size());
  
  return true;
}

/******************************************************************************
 * Class private constructor
 *****************************************************************************/

/**
 * Creates the single instance of the class using the clustering configuration
 */
PlottingManager::PlottingManager(bool DataExtraction)
{
  ClusteringConfiguration* Configuration;
  ParametersManager*       Parameters;

  this->DataExtraction = DataExtraction;

  Configuration = ClusteringConfiguration::GetInstance();
  if (!Configuration->IsInitialized())
  {
    SetError(true);
    SetErrorMessage("configuration not initialized");
    return;
  }

  Parameters = ParametersManager::GetInstance();

  if (Parameters == NULL)
  {
    SetError(true);
    SetErrorMessage("parameters not loaded correctly");
    return;
  }

  if (Configuration->GetAllPlots())
  {
    GenerateAllPlots (Parameters);
  }
  else
  {
    if (Configuration->GetPlotsDefinitionsError())
    {
      SetError(true);
      SetErrorMessage(Configuration->GetPlotsDefinitionsErrorMessage());
      return;
    }

    Plots = Configuration->GetPlotsDefinitions();

    for (INT32 i = 0; i < Plots.size(); i++)
    {
      LoadSinglePlot(Plots[i], Parameters);
    }
  }

  ClusterIdPosition = CSV_HEADING_FIELDS +
                      Parameters->GetClusteringParametersSize ()*2 +
                      Parameters->GetExtrapolationParametersSize () + 1;
  
  return;
}

/**
 * Generates all possible combinations of parameters
 *
 * \param Parameters Parameters manager to get parameters read from the XML
 */
void
PlottingManager::GenerateAllPlots(ParametersManager* Parameters)
{
  INT32 ClusteringParameters;
  INT32 ExtrapolationParameters;

  vector<string> ClusteringParametersNames;
  vector<string> ExtrapolationParametersNames;


  PlotDefinition* Definition;
  
  ClusteringParameters      = Parameters->GetClusteringParametersSize();
  ClusteringParametersNames = Parameters->GetClusteringParametersNames();
  
  ExtrapolationParameters      = Parameters->GetExtrapolationParametersSize();
  ExtrapolationParametersNames = Parameters->GetExtrapolationParametersNames();

  for (INT32 X = 0; X < ClusteringParameters-1; X++)
  {
    for (INT32 Y = X+1; Y < ClusteringParameters; Y++)
    { /* Combination of clustering parameters, raw and normalized */

      /* Raw combination */
      Definition = new PlotDefinition();

      Definition->RawMetrics      = true;
      Definition->ThreeDimensions = false;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X];
      Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y];
      Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");
      
      /* Normalized */
      Definition = new PlotDefinition();

      Definition->RawMetrics      = false;
      Definition->ThreeDimensions = false;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X]+" Normalized";
      Definition->XMetricPosition = CSV_HEADING_FIELDS + ClusteringParameters + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y]+" Normalized";
      Definition->YMetricPosition = CSV_HEADING_FIELDS + ClusteringParameters + Y + 1;

      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");


      for (INT32 Z = 0; Z < ExtrapolationParameters; Z++)
      { /* Combination of each pair of clustering metrics and one extrapolation metric */
        Definition = new PlotDefinition();

        Definition->RawMetrics      = true;
        Definition->ThreeDimensions = true;

        Definition->XMetric         = ClusteringParametersNames[X];
        Definition->XMetricTitle    = ClusteringParametersNames[X];
        Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

        Definition->YMetric         = ClusteringParametersNames[Y];
        Definition->YMetricTitle    = ClusteringParametersNames[Y];
        Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

        Definition->ZMetric         = ExtrapolationParametersNames[Z];
        Definition->ZMetricTitle    = ExtrapolationParametersNames[Z];
        Definition->ZMetricPosition = CSV_HEADING_FIELDS + 
                                      (2*ClusteringParameters) + Z + 1;
        
        Plots.push_back(Definition);
        CorrectPlots.push_back(true);
        PlotsWarnings.push_back("Correct Plot");
      }

      /* And 3D plot with duration */
      Definition = new PlotDefinition();
      
      Definition->RawMetrics      = true;
      Definition->ThreeDimensions = true;

      Definition->XMetric         = ClusteringParametersNames[X];
      Definition->XMetricTitle    = ClusteringParametersNames[X];
      Definition->XMetricPosition = CSV_HEADING_FIELDS + X + 1;

      Definition->YMetric         = ClusteringParametersNames[Y];
      Definition->YMetricTitle    = ClusteringParametersNames[Y];
      Definition->YMetricPosition = CSV_HEADING_FIELDS + Y + 1;

      Definition->ZMetric         = "Duration";
      Definition->ZMetricTitle    = "Duration";
      Definition->ZMetricPosition = CSV_DURATION_FIELD;
      
      Plots.push_back(Definition);
      CorrectPlots.push_back(true);
      PlotsWarnings.push_back("Correct Plot");
    }
  }
}

/******************************************************************************
 * Plots definitions loader
 *****************************************************************************/

/**
 * Checks the definition of a single plot read from the XML, if some parameters are missing
 * it adds a warning message into the warnings vector, and sets the plot as incorrect plot
 * in the correct plots vector
 * \param CurrentPlot Configuration object describing the plot extracted from the XML
 * \param Parameters Parameters manager object to find the positions of the different parameters
 */
void
PlottingManager::LoadSinglePlot(PlotDefinition*    CurrentPlot,
                                ParametersManager *Parameters)
{
  INT32           XMetricPosition, YMetricPosition, ZMetricPosition;
  bool            Error = false;
  
  /* Check parameters positions on the XML file */

  /* X axis */
  CurrentPlot->XMetricPosition = CheckParameterPosition(CurrentPlot->XMetric,
                                                        CurrentPlot->RawMetrics,
                                                        Parameters);

  if (CurrentPlot->XMetricPosition == NOT_FOUND)
  {
    ostringstream Message;
    Message << "X metric not found on plot defined in line " << CurrentPlot->Line;

    PlotsWarnings.push_back(Message.str());
    CorrectPlots.push_back(false);
    Error = true;
    return;
  }

  /* Y axis */
  CurrentPlot->YMetricPosition = CheckParameterPosition(CurrentPlot->YMetric,
                                                        CurrentPlot->RawMetrics,
                                                        Parameters);

  if (CurrentPlot->YMetricPosition == NOT_FOUND)
  {
    ostringstream Message;
    Message << "Y metric not found on plot defined in line " << CurrentPlot->Line;

    PlotsWarnings.push_back(Message.str());
    CorrectPlots.push_back(false);
    Error = true;
    return;
  }

  
  if (CurrentPlot->ThreeDimensions)
  { /* Z axis */
    CurrentPlot->ZMetricPosition = CheckParameterPosition(CurrentPlot->ZMetric,
                                                          CurrentPlot->RawMetrics,
                                                          Parameters);

    if (CurrentPlot->ZMetricPosition == NOT_FOUND)
    {
      ostringstream Message;
      Message << "Z metric not found on plot defined in line " << CurrentPlot->Line;

      PlotsWarnings.push_back(Message.str());
      CorrectPlots.push_back(false);
      Error = true;
    }
  }

  if (!Error)
  {
    PlotsWarnings.push_back("Correct Plot");
    CorrectPlots.push_back(true);
  }

  return;
}

/**
 * Returns for the position of a given paramater into the CSV file
 * \param ParameterName Name of the parameter we want the position
 * \param RawMetric Boolean indicating if the parameter values should be the raw or normalized
 * \param Parameters Parameters manager to obtain the positions
 * \return The parameter position if found, Constants::NOT_FOUND otherwise
 */
INT32
PlottingManager::CheckParameterPosition(string             ParameterName,
                                        bool               RawMetric,
                                        ParametersManager *Parameters)
{
  INT32 Position;

  if (ParameterName.compare("Duration") == 0 || ParameterName.compare("duration") == 0)
  {
    return CSV_DURATION_FIELD;
  }
  
  if (!RawMetric)
  { /* Normalized metric, just look into clustering parameters */
    Position = Parameters->GetClusteringParameterPosition(ParameterName);

    if (Position != NOT_FOUND)
    { /* Adjust to real position in the CSV (add number of heading fields + number of raw metrics) */
      Position += CSV_HEADING_FIELDS;
      Position += Parameters->GetClusteringParametersSize();
    }
    else
    {
      return NOT_FOUND;
    }
  }
  else
  { /* Raw metric, could be a clustering parameter or an extrapolation parameter */

    Position = Parameters->GetClusteringParameterPosition(ParameterName);

    if (Position != NOT_FOUND)
    {
      Position += CSV_HEADING_FIELDS;
    }
    else
    { /* Look into extrapolation parameters */
      Position = Parameters->GetExtrapolationParameterPosition(ParameterName);

      if (Position != NOT_FOUND)
      { /* Heading fields + Raw clustering metrics + Normalized clustering metrics */
        Position += CSV_HEADING_FIELDS;
        Position += (Parameters->GetClusteringParametersSize()*2);
      }
      else
      {
        return NOT_FOUND;
      }
    }
  }

  return Position+1;
}


/**
 * Writes to disc a single plot of the plots vector
 *
 * \param FileNamePrefix Prefix of the output script
 * \param DataFileName   Name of the file that contains the data of the current plot
 * \param Definition     Container of the plot definition
 * \param Title          Title to be used in the plots
 * \param DifferentIDs   Set with the different IDs present in the data
 * 
 * \return True if the plot has been written correctly, false otherwise
 */
bool PlottingManager::PrintSinglePlot(string             FileNamePrefix,
                                      string             DataFileName,
                                      PlotDefinition    *Definition,
                                      string             Title,
                                      set<cluster_id_t>& DifferentIDs)
{
  ofstream                    OutputStream;
  string                      OutputFileName;
  ostringstream               ScreenPlotName;
  set<cluster_id_t>::iterator IDsIterator;
  vector<cluster_id_t>        IDs;
  
  /* Transform the set of different IDs into a vector and sort it */
  if (DifferentIDs.size() == 0)
  { /* Data not clustered */
    IDs.push_back(UNCLASSIFIED);
    IDs.push_back(MISSING_DATA_CLUSTERID);
    IDs.push_back(DURATION_FILTERED_CLUSTERID);
    IDs.push_back(RANGE_FILTERED_CLUSTERID);
  }
  else
  {
    for (IDsIterator  = DifferentIDs.begin();
         IDsIterator != DifferentIDs.end();
         ++IDsIterator)
    {
      IDs.push_back((*IDsIterator));
    }
    sort(IDs.begin(), IDs.end());
  }

  if (!Definition->ThreeDimensions)
  {
    OutputFileName  = FileNamePrefix;
    OutputFileName += "."+Definition->XMetric;
    OutputFileName += "."+Definition->YMetric;

    if (!Definition->RawMetrics)
    {
     OutputFileName += ".Normalized";
    }

    OutputFileName += ".gnuplot";

    ScreenPlotName << Definition->XMetricTitle << " vs. ";
    ScreenPlotName << Definition->YMetricTitle << endl;
  }
  else
  {
    OutputFileName  = FileNamePrefix;
    OutputFileName += ".3D";
    OutputFileName += "."+Definition->XMetric;
    OutputFileName += "."+Definition->YMetric;
    OutputFileName += "."+Definition->ZMetric;
    OutputFileName += ".gnuplot";

    ScreenPlotName << Definition->XMetricTitle << " vs. ";
    ScreenPlotName << Definition->YMetricTitle << " vs. ";
    ScreenPlotName << Definition->ZMetricTitle << endl;
  }
  
  OutputStream.open(OutputFileName.c_str(), ios_base::trunc);

  if (!OutputStream)
  {
    SetError(true);
    SetErrorMessage("unable to open script file", strerror(errno));
    return false;
  }

  /* Write the plot title and common commands */

  OutputStream << "set datafile separator \",\"" << endl;
  OutputStream << endl;
  OutputStream << "set title ";

  if (DataExtraction)
  {
    OutputStream << "\"Data from file " << DataFileName << "\"" << endl;
  }
  else
  { /* CHECK THIS!! */
    OutputStream << "\"" << Title << "\"" << endl;
  }

  OutputStream << "set xlabel \"" << Definition->XMetricTitle << "\"" << endl;
  OutputStream << "set ylabel \"" << Definition->YMetricTitle << "\"" << endl;
  
  /*
  UNCLASSIFIED;
  MISSING_DATA_CLUSTERID;
  DURATION_FILTERED_CLUSTERID;
  RANGE_FILTERED_CLUSTERID;
  THRESHOLD_FILTERED_CLUSTERID;
  */ 
  
  /* Print concrete commands */
  if (Definition->ThreeDimensions)
  {
    INT32 X, Y, Z;

    X = Definition->XMetricPosition;
    Y = Definition->YMetricPosition;
    Z = Definition->ZMetricPosition;

    /* Z label */
    OutputStream << "set zlabel \"" << Definition->ZMetricTitle << "\"" << endl;
    OutputStream << endl;

    /* X-Range */
    OutputStream << "set xrange [";
    if (Definition->XMin != MIN_DOUBLE)
    {
      OutputStream << Definition->XMin;
    }
    OutputStream << ":";
    if (Definition->XMax != MAX_DOUBLE)
    {
      OutputStream << Definition->XMax;
    }
    OutputStream << "]" << endl;

    /* Y-Range */
    OutputStream << "set yrange [";
    if (Definition->YMin != MIN_DOUBLE)
    {
      OutputStream << Definition->YMin;
    }
    OutputStream << ":";
    if (Definition->YMax != MAX_DOUBLE)
    {
      OutputStream << Definition->YMax;
    }
    OutputStream << "]" << endl;
    
    /* Z-Range */
    OutputStream << "set zrange [";
    if (Definition->ZMin != MIN_DOUBLE)
    {
      OutputStream << Definition->ZMin;
    }
    OutputStream << ":";
    if (Definition->ZMax != MAX_DOUBLE)
    {
      OutputStream << Definition->ZMax;
    }
    OutputStream << "]" << endl;

    OutputStream << endl;
    OutputStream << "set key right outside";
    OutputStream << endl;
    
    /* Actual plot */
    OutputStream << "splot ";
    for (size_t i = 0; i < IDs.size(); i++)
    {
      Write3D_Definition (OutputStream, X, Y, Z,
                          IDs[i]+PARAVER_OFFSET,
                          GetClusterName(IDs[i]),
                          DataFileName);
                          
      if (i != IDs.size()-1)
        OutputStream << ",\\" << endl;
    }
    
    /*
    if (NumberOfClusters == 0)
    {
      Write3D_Definition (OutputStream, X, Y, Z, UNCLASSIFIED+PARAVER_OFFSET, "Unclassified", DataFileName);
      OutputStream << ",\\" << '\n';
      Write3D_Definition (OutputStream, X, Y, Z, DURATION_FILTERED_CLUSTERID, "Duration Filtered", DataFileName);
      OutputStream << ",\\" << '\n';
      Write3D_Definition (OutputStream, X, Y, Z, RANGE_FILTERED_CLUSTERID, "Range Filtered", DataFileName);
      OutputStream << ",\\" << '\n';
      Write3D_Definition (OutputStream, X, Y, Z, THRESHOLD_FILTERED_CLUSTERID, "Threshold Filtered", DataFileName);
      // OutputStream << ",\\" << '\n';
    }
    else
    {
      if (PrintNoiseCluster)
      {
        Write3D_Definition (OutputStream, X, Y, Z, NOISE_CLUSTERID+PARAVER_OFFSET, "Noise", DataFileName);
        OutputStream << ",\\" << '\n';
      }
    
      for (cluster_id_t Cluster = 1; Cluster <= NumberOfClusters; Cluster++)
      {
        ostringstream ClusterName;

        ClusterName << "Cluster " << Cluster;

        Write3D_Definition (OutputStream, X, Y, Z, Cluster+PARAVER_OFFSET, ClusterName.str(), DataFileName);

        if (Cluster != NumberOfClusters )
          OutputStream << ",\\" << endl;
      }
    }
    */
  }
  else
  {
    INT32 X, Y;

    X = Definition->XMetricPosition;
    Y = Definition->YMetricPosition;

    /* X-Range */
    OutputStream << "set xrange [";
    if (Definition->XMin != MIN_DOUBLE)
    {
      OutputStream << Definition->XMin;
    }
    OutputStream << ":";
    if (Definition->XMax != MAX_DOUBLE)
    {
      OutputStream << Definition->XMax;
    }
    OutputStream << "]" << endl;

    /* Y-Range */
    OutputStream << "set yrange [";
    if (Definition->YMin != MIN_DOUBLE)
    {
      OutputStream << Definition->YMin;
    }
    OutputStream << ":";
    if (Definition->YMax != MAX_DOUBLE)
    {
      OutputStream << Definition->YMax;
    }
    OutputStream << "]" << endl;

    OutputStream << endl;
    OutputStream << "set key right outside";
    OutputStream << endl;

    /* Actual plot */
    OutputStream << "plot ";
    for (size_t i = 0; i < IDs.size(); i++)
    {
      Write2D_Definition(OutputStream, X, Y,
                         IDs[i]+PARAVER_OFFSET,
                         GetClusterName(IDs[i]),
                         DataFileName);
                          
      if (i != IDs.size()-1)
        OutputStream << ",\\" << endl;
    }
    /*
    if (NumberOfClusters == 0)
    {
      Write2D_Definition (OutputStream, X, Y, UNCLASSIFIED+PARAVER_OFFSET, "Unclassified", DataFileName);
      OutputStream << ",\\" << '\n';
      Write2D_Definition (OutputStream, X, Y, DURATION_FILTERED_CLUSTERID, "Duration Filtered", DataFileName);
      OutputStream << ",\\" << '\n';
      Write2D_Definition (OutputStream, X, Y, RANGE_FILTERED_CLUSTERID, "Range Filtered", DataFileName);
      OutputStream << ",\\" << '\n';
      Write2D_Definition (OutputStream, X, Y, THRESHOLD_FILTERED_CLUSTERID, "Threshold Filtered", DataFileName);
      // OutputStream << ",\\" << '\n';
    }
    else
    {
      if (PrintNoiseCluster)
      {
        Write2D_Definition (OutputStream, X, Y, NOISE_CLUSTERID+PARAVER_OFFSET, "Noise", DataFileName);
        OutputStream << ",\\" << '\n';
      }
      
      for (size_t Cluster = 1; Cluster <= NumberOfClusters; Cluster++)
      {
        ostringstream ClusterName;
        ClusterName << "Cluster " << Cluster;

        Write2D_Definition (OutputStream, X, Y, Cluster+PARAVER_OFFSET, ClusterName.str(), DataFileName);

        if (Cluster != NumberOfClusters )
          OutputStream << ",\\" << endl;

      }
    }
    */
  }

  /* Print clusters */
  OutputStream << endl;
  
  /* Print pause command */
  OutputStream << "pause -1 \"Press return to continue...\"" << endl;
  
  /* Check if there is any error in the output */
  if (OutputStream.fail())
  {
    SetError(true);
    SetErrorMessage("error while writing a script file", strerror(errno));
    return false;
  }

  system_messages::information(ScreenPlotName.str().c_str());

  return true;
}

/**
 * Write the GNUPlot commands to print a 2 dimensional plot
 *
 * \param str Stream to write the script
 */
void
PlottingManager::Write2D_Definition(ostream& str,
                                    INT32    X,
                                    INT32    Y,
                                    INT32    ClusterId,
                                    string   ClusterName,
                                    string   DataFileName)
{
  if (!PrintingModels)
  {
    str << "\'" << DataFileName << "\' using ";
    str << X << ":($" << ClusterIdPosition << " == " << ClusterId << " ? $" << Y << " : 1/0) ";
    str << "w points ps 1.5 lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";
    str << "title \"" << ClusterName << "\"";
  }
  else
  { /* Temporary just use two dimensions */
    str << "\'" << DataFileName << "\' using ";
    str <<  "2:($3 ==  " << ClusterId << " ? $1 : 1/0) ";
    str << "w filledcurve lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";
    // str << "w points ps 1.5 lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";
    str << "title \"" << ClusterName << "\"";
  }

}

/**
 * Write the GNUPlot commands to print a 3 dimensional plot
 *
 * \param str Stream to write the script
 */
void
PlottingManager::Write3D_Definition(ostream& str,
                                    INT32    X,
                                    INT32    Y,
                                    INT32    Z,
                                    INT32    ClusterId,
                                    string   ClusterName,
                                    string   DataFileName)
{
  if (!PrintingModels)
  {
    str << "\'" << DataFileName << "\' using ";
    str << X << ":" << Y << ":($" << ClusterIdPosition << " == " << ClusterId << " ? $" << Z << " : 1/0) ";
    str << "w points ps 1.5 lt rgbcolor \"" << RGBStateColor(ClusterId) << "\" ";
    str << "title \"" << ClusterName << "\"";
  }
}

/**
 * Returns the standard Paraver color state color for a given state number
 *
 * \param ClusterId/State color to find the RGB codification
 *
 * \return A string containing the RGB codification of the given state number
 */
string PlottingManager::RGBStateColor(INT32 StateValue)
{
  char RGBColor[8];

  if (StateValue < DEF_NB_COLOR_STATE)
  {

    if ((ParaverDefaultPalette[StateValue].RGB[0] == 0xFF) &&
        (ParaverDefaultPalette[StateValue].RGB[1] == 0xFF) &&
        (ParaverDefaultPalette[StateValue].RGB[2] == 0xFF))
    {
      sprintf(RGBColor, "#000000");
    }
    else
    {
      sprintf(RGBColor, "#%02x%02x%02x",
        ParaverDefaultPalette[StateValue].RGB[0],
        ParaverDefaultPalette[StateValue].RGB[1],
        ParaverDefaultPalette[StateValue].RGB[2]);
    }
  }
  else
  {
    sprintf(RGBColor, "#%02x%02x%02x",
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[0],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[1],
      ParaverDefaultPalette[DEF_NB_COLOR_STATE-1].RGB[2]);
  }

  return RGBColor;
}

/**
 * Returns the cluster name using the ID
 * 
 * \param ID ID value to generate the cluster name
 * 
 * \return The name of the cluster, taking into account the special cluster ids
 */
string PlottingManager::GetClusterName(cluster_id_t ID)
{
  ostringstream ClusterName;
  
  switch (ID)
  {
    case UNCLASSIFIED:
      ClusterName << "Unclassified";
      break;
    case DURATION_FILTERED_CLUSTERID:
      ClusterName << "Duration Filtered";
      break;
    case RANGE_FILTERED_CLUSTERID:
      ClusterName << "Range Filtered";
      break;
    case THRESHOLD_FILTERED_CLUSTERID:
      ClusterName << "Threshold Filtered";
      break;
    case NOISE_CLUSTERID:
      ClusterName << "Noise";
      break;
    default:
      ClusterName << "Cluster " << ID;
      break;
  }

  return ClusterName.str();
}

