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

#include <CPUBurst.hpp>

UINT64 CPUBurst::InstanceNumber = 0;

CPUBurst::CPUBurst(task_id_t           TaskId,
                   thread_id_t         ThreadId,
                   line_t              Line,
                   timestamp_t         BeginTime,
                   timestamp_t         EndTime,
                   duration_t          Duration,
                   vector<double>&     ClusteringRawData,
                   vector<double>&     ClusteringProcessedData,
                   map<INT32, double>& ExtrapolationData,
                   burst_type_t        BurstType,
                   bool                ToClassify)
:Point (ClusteringProcessedData)
{
  this->Instance                = (CPUBurst::InstanceNumber++);
  this->TaskId                  = TaskId;
  this->ThreadId                = ThreadId;
  this->Line                    = Line;
  this->BeginTime               = BeginTime;
  this->EndTime                 = EndTime;
  this->Duration                = Duration;
  this->RawDimensions           = ClusteringRawData;
  this->ExtrapolationDimensions = ExtrapolationData;
  this->BurstType               = BurstType;
  this->ToClassify              = ToClassify;
}

CPUBurst::~CPUBurst(void)
{
  /*
  RawDimensions.clear();
  Dimensions.clear();
  ExtrapolationDimensions.clear();
  */
}

double CPUBurst::GetRawDimension(size_t Index)
{
  if (Index >= RawDimensions.size() || Index < 0)
    return 0.0;
  else
    return RawDimensions[Index];
}

size_t CPUBurst::GetExtrapolationDimensionsCount(void) const
{
  return ExtrapolationDimensions.size();
};

burst_type_t CPUBurst::GetBurstType(void) const
{
  return BurstType;
}

bool CPUBurst::Scale(vector<double>& Mean, vector<double>& RMS)
{
  if ( Mean.size() != Dimensions.size() ||
       RMS.size()  != Dimensions.size())
    return false;

  
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    Dimensions[i] =
      (Dimensions[i]-Mean[i])/RMS[i];
  }
  
  return true;
}

bool
CPUBurst::MeanAdjust(vector<double>& DimensionsAverage)
{
  if (DimensionsAverage.size() != RawDimensions.size())
    return false;
  
  for (INT32 i = 0; i < DimensionsAverage.size(); i++)
  {
    Dimensions[i] = RawDimensions[i] - DimensionsAverage[i];
  }
  
  return true;
}

bool
CPUBurst::BaseChange(vector< vector<double> >& BaseChangeMatrix)
{
  vector<double> BaseChangedDimensions (BaseChangeMatrix.size());

  /* DEBUG
  cout << "Original RawDimensions = {";
  for (INT32 i = 0; i < RawDimensions.size(); i++)
  {
    cout << RawDimensions[i] << " ";
  }
  cout << "}" << endl;
  
  cout << "Original NormalizedDimensions = {";
  for (INT32 i = 0; i < NormalizedDimensions.size(); i++)
  {
    cout << NormalizedDimensions[i] << " ";
  }
  cout << "}" << endl;
  */

      
  for (INT32 i = 0; i < BaseChangeMatrix.size(); i++)
  {
    if (BaseChangeMatrix[i].size() != Dimensions.size())
    {
      /* DEBUG */
      cout << "ERROR!: BaseChangeMatrix.size = " << BaseChangeMatrix[i].size();
      cout << " NormalizedDimensions.size = " << Dimensions.size() << endl;
      return false;
    }
    
    BaseChangedDimensions[i] = 0.0;
    
    for (INT32 j = 0; j < BaseChangeMatrix[i].size(); j++)
    {
      BaseChangedDimensions[i] += 
        (Dimensions[j]*BaseChangeMatrix[i][j]);
    }
  }
  
  Dimensions = BaseChangedDimensions;
  
  /* DEBUG 
  cout << "Base changed NormalizedDimensions = {";
  for (INT32 i = 0; i < NormalizedDimensions.size(); i++)
  {
    cout << NormalizedDimensions[i] << " ";
  }
  cout << "}" << endl;
  */
  
  return true;
}

/**
 * Print all information contained in the point
 * \param str Output stream where information will be flushed
 * \param ExtrapolationDimensionsTotalSize Number of extrapolation dimensions to correctly flush this partial data
 * \resutl True if point has been written correctly, false otherwise
 */
bool
CPUBurst::Print(ostream&       str,
                 vector<bool>& ClusteringParametersPrecision,
                 vector<bool>& ExtrapolationParametersPrecision,
                 cluster_id_t   ClusterId)
{
  map<INT32, double>::iterator ExtrapolationData;

  size_t TotalExtrapolationDimensions = ExtrapolationParametersPrecision.size();
  
  /* Common data */
  str << Instance  << ", " << TaskId  << ", " << ThreadId << ", ";
  str << BeginTime << ", " << EndTime << ", " << Duration << ", " << Line;

  str.setf(ios::fixed,ios::floatfield);
  
  /* Clustering Dimensions Raw */
  for (INT32 i = 0; i < RawDimensions.size(); i++)
  {
    if (ClusteringParametersPrecision[i])
    { /* High precision */
      str.precision(6);
    }
    else
    {
      str.precision(0);
    }
    
    str << ", " << RawDimensions[i];
  }

  /* Clustering Dimensions Normalized */
  str.precision(6);
  for (size_t i = 0; i < Dimensions.size(); i++)
  {
    str << ", " << Dimensions[i];
  }

  /* Extrapolation Dimensions */
  for (size_t i = 0; i < TotalExtrapolationDimensions; i++)
  {
    str << ", ";

    ExtrapolationData = ExtrapolationDimensions.find(i);
    if (ExtrapolationData != ExtrapolationDimensions.end())
    {

      if (ExtrapolationParametersPrecision[i])
      { /* High precision */
        str.precision(6);
      }
      else
      {
        str.precision(0);
      }
      
      str << ExtrapolationData->second;
    }
    else
    {
      str << "nan";
    }
  }

  /* ClusterID */
  str << ", " << ClusterId << endl;
  
  return true;
}

/* DEBUG */
void
CPUBurst::PrintBurst(void)
{
  cout << "Instance: " << Instance;

  PrintPoint ();
}
