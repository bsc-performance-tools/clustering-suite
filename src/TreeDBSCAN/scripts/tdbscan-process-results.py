import os
import subprocess
import sys

ClustersDiff="@sub_PREFIX@/bin/ClustersDiff"
ClustersSequenceScore="@sub_PREFIX@/bin/ClustersSequenceScore"

LOCAL_HULLS         = 'Hulls'
GLOBAL_HULLS        = 'Sum hulls'
CLUSTERING_POINTS   = 'Clustering points'
NOISE_POINTS        = 'Noise'
EXTRACTION_TIME     = 'Extraction time'
CLUSTERING_TIME     = 'Clustering time'
MERGE_TIME          = 'Merge time'
CLASSIFICATION_TIME = 'Classification time'
RECONSTRUCT_TIME    = 'Reconstruct time'
TOTAL_TIME          = 'Total time'

def GetValue(AllStats, WhichStat):
  stats_array = AllStats.replace('\\n', ',').split(',')
  for stat in stats_array:
    if (stat[0:len(WhichStat)] == WhichStat):
      return float(stat.split('=')[1])
  return float(0)

AvgClusteringPoints = 0
AvgNoisePoints = 0
AvgExtractionTime = 0
AvgClusteringTime = 0
AvgMergeTime = 0
AvgClassificationTime = 0
AvgReconstructTime = 0
AvgTotalTime = 0
AvgLocalHulls = 0
MirkinDistance = 0
SequenceScore = 0

if ((len(sys.argv) < 7) or (len(sys.argv) > 8)):
  print "Usage: $0 <experiment-id> <input-trace> <fan> <num-backends> <total-resources> <output-prefix> [reference-trace]"
  sys.exit(-1)

ExperimentId  = int(sys.argv[1])
InputTrace    = sys.argv[2]
Fan           = int(sys.argv[3])
NumBackends   = int(sys.argv[4])
TotalResources = int(sys.argv[5])
OutputPrefix  = sys.argv[6]
OutputCSV     = "experiment_results.csv"
if (len(sys.argv) == 8):
  HaveReference = True
  ReferenceTrace = sys.argv[7]
  ReferenceCSV = ReferenceTrace[0:-4] + ".DATA.csv"
else:
  HaveReference = False

# Parse the number of tasks from the input trace
NumTasks = 0
with open(InputTrace, 'r') as f:
  first_line = f.readline()
  NumTasks = first_line.split(':')[3].split('(')[0]

# Check Mirkin Distance and Sequence Score
if (HaveReference):
  ReferenceCSVSorted = ReferenceCSV.split('/')[-1][0:-4] + ".sorted.csv"
  FinalCSV = OutputPrefix + ".FINAL.DATA.csv"
  FinalCSVSorted = FinalCSV[0:-4] + ".sorted.csv"

  os.system( "sort -t, -h -k5 " + ReferenceCSV + " > " + ReferenceCSVSorted )
  os.system( "sort -t, -h -k5 " + FinalCSV + " > " + FinalCSVSorted )
 
  cmd = ClustersDiff + " -1 " + FinalCSVSorted + " -2 " + ReferenceCSVSorted
  print "Running: " + cmd
  try:
    output = subprocess.check_output(cmd, shell=True)
    MirkinDistance = float(output)
  except:
    MirkinDistance = 0

  cmd = ClustersSequenceScore + " " + OutputPrefix + ".FINAL.DATA.csv"
  print "Running: " + cmd
  try:
    output = subprocess.check_output(cmd, shell=True)
    SequenceScore = float(output)
  except:
    SequenceScore = 0

# Read the statistics from the MRNETSTATS.data file
StatisticsDataFile = OutputPrefix + ".MRNETSTATS.data"
stats_fd = open(StatisticsDataFile, 'r')
for i in range(0, NumBackends):
  CurrentBackendStats = stats_fd.readline()
  AvgClusteringPoints += GetValue(CurrentBackendStats, CLUSTERING_POINTS)
  AvgNoisePoints += GetValue(CurrentBackendStats, NOISE_POINTS)
  AvgExtractionTime += GetValue(CurrentBackendStats, EXTRACTION_TIME)
  AvgClusteringTime += GetValue(CurrentBackendStats, CLUSTERING_TIME)
  AvgMergeTime += GetValue(CurrentBackendStats, MERGE_TIME)
  AvgClassificationTime += GetValue(CurrentBackendStats, CLASSIFICATION_TIME)
  AvgReconstructTime += GetValue(CurrentBackendStats, RECONSTRUCT_TIME)
  AvgTotalTime += GetValue(CurrentBackendStats, TOTAL_TIME)
  AvgLocalHulls += GetValue(CurrentBackendStats, LOCAL_HULLS)
stats_fd.close()

AvgClusteringPoints /= NumBackends
AvgClusteringPoints = int(AvgClusteringPoints)
AvgNoisePoints /= NumBackends
AvgNoisePoints = int(AvgNoisePoints)
AvgExtractionTime /= NumBackends
AvgClusteringTime /= NumBackends
AvgMergeTime /= NumBackends
AvgClassificationTime /= NumBackends
AvgReconstructTime /= NumBackends
AvgTotalTime /= NumBackends
AvgLocalHulls /= NumBackends
AvgLocalHulls = int(AvgLocalHulls)
 
# Read the number of global hulls from the final clusters_info.csv
ClustersInfoFile = OutputPrefix + ".FINAL.clusters_info.csv" 
clusters_info_fd = open(ClustersInfoFile, 'r')
clusters_names = clusters_info_fd.readline()
GlobalHulls = len(clusters_names.split(',')) - 2 
clusters_info_fd.close()

fd = open(OutputCSV, 'w');
fd.write("Experiment, NumTasks, FanIn, NumBackends, TotalResources, NumPoints, LocalHulls, GlobalHulls, ExtractionTime, LocalClusteringTime, MergeTime, ClassificationTime, ReconstructTime, TotalTime, MirkinDistance, SequenceScore\n")
fd.write(str(ExperimentId) + ", " +
         str(NumTasks) + ", " + 
         str(Fan) + ", " +
         str(NumBackends) + ", " + 
         str(TotalResources) + ", " + 
         str(AvgClusteringPoints) + ", " + 
         str(AvgLocalHulls) + ", " + 
         str(GlobalHulls) + ", " + 
         str(AvgExtractionTime) + ", " + 
         str(AvgClusteringTime) + ", " + 
         str(AvgMergeTime) + ", " + 
         str(AvgClassificationTime) + ", " + 
         str(AvgReconstructTime) + ", " + 
         str(AvgTotalTime) + ", " + 
         str(MirkinDistance) + ", " +
         str(SequenceScore) + "\n")
fd.close()
