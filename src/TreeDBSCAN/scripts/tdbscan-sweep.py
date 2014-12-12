import os
import subprocess
import sys

Launcher="@sub_PREFIX@/bin/tdbscan-launcher"
ClustersDiff="@sub_PREFIX@/bin/ClustersDiff"
ClustersSequenceScore="@sub_PREFIX@/bin/ClustersSequenceScore"

ExperimentTag="_experiment_"
OutputPrefix=""
OutputSuffix="_sweep_results.csv"

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


if ((len(sys.argv) < 7) or (len(sys.argv) > 8)):
  print "Usage: $0 <iterations> <min-backends> <max-backends> <xml> <input-trace> <output-prefix> [reference-trace]"
  sys.exit(-1)

NumIters      = int(sys.argv[1])
MinBackends   = int(sys.argv[2])
MaxBackends   = int(sys.argv[3])
ClusteringXML = sys.argv[4]
InputTrace    = sys.argv[5]
OutputPrefix  = sys.argv[6]
if (len(sys.argv) == 8):
  HaveReference = True
  ReferenceTrace = sys.argv[7]
  ReferenceCSV = ReferenceTrace[0:-4] + ".DATA.csv"
else:
  HaveReference = False
ClusteringInput = "-d " + ClusteringXML + " -i " + InputTrace 

OutputCSV = OutputPrefix + OutputSuffix
fd = open(OutputCSV, 'w');
fd.write("Experiment, NumTasks, FanIn, NumBackends, NumPoints, LocalHulls, GlobalHulls, ExtractionTime, LocalClusteringTime, MergeTime, ClassificationTime, ReconstructTime, TotalTime, MirkinDistance, SequenceScore\n")

NumTasks = 0
with open(InputTrace, 'r') as f:
  first_line = f.readline()
  NumTasks = first_line.split(':')[3].split('(')[0]

ExperimentNo = 1
NumBackends=MaxBackends
while (NumBackends >= MinBackends):

  FanIn=NumBackends
  while (FanIn > 1):

    ProcsPerLvl=NumBackends
    while (ProcsPerLvl >= FanIn):
      ProcsPerLvl=ProcsPerLvl / FanIn

    if (ProcsPerLvl == 1):
      ProcsPerLvl=FanIn
    TopologySpec=""
    TopologySpec=str(ProcsPerLvl) + ":"
    while (ProcsPerLvl != NumBackends):
      TopologySpec=TopologySpec + str(ProcsPerLvl) + "x" + str(FanIn) + ":"
      ProcsPerLvl=ProcsPerLvl * FanIn

    TopologySpec = "g:" + TopologySpec[0:-1]
    print "NumBackends: ", NumBackends, "FanIn: ", FanIn, "TopologySpec: ", TopologySpec

    for i in range(1, NumIters+1):
      # Launch TDBSCAN
      cmd = Launcher + " " + TopologySpec + " " + ClusteringInput + " -o " + OutputPrefix + ExperimentTag + str(ExperimentNo) + ".prv"
      print "Running: " + cmd

      output = ""
      try:
        output = subprocess.check_output( cmd.split(' ') )
        print output
      except subprocess.CalledProcessError as error:
        print "Error!"
        print error
        sys.exit(-1)

      # Check Mirkin Distance and Sequence Score
      if (HaveReference):
        ReferenceCSVSorted = ReferenceCSV + ".sorted.csv"
        FinalCSV = OutputPrefix + ExperimentTag + str(ExperimentNo) + ".FINAL.DATA.csv"
        FinalCSVSorted = FinalCSV + ".sorted.csv"
     
        os.system( "sort -t, -h -k5 " + ReferenceCSV + " > " + ReferenceCSVSorted )
        os.system( "sort -t, -h -k5 " + FinalCSV + " > " + FinalCSVSorted )
 
        cmd = ClustersDiff + " -1 " + FinalCSVSorted + " -2 " + ReferenceCSVSorted
        print "Running: " + cmd
        output = subprocess.check_output(cmd, shell=True)
        MirkinDistance = float(output)
      else:
        MirkinDistance = 0

      cmd = ClustersSequenceScore + " " + OutputPrefix + ExperimentTag + str(ExperimentNo) + ".FINAL.DATA.csv"
      print "Running: " + cmd
      try:
        output = subprocess.check_output(cmd, shell=True)
        SequenceScore = float(output)
      except:
        SequenceScore = 0

      StatisticsDataFile = OutputPrefix + ExperimentTag + str(ExperimentNo) + ".MRNETSTATS.data"
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
 

      ClustersInfoFile = OutputPrefix + ExperimentTag + str(ExperimentNo) + ".FINAL.clusters_info.csv" 
      clusters_info_fd = open(ClustersInfoFile, 'r')
      clusters_names = clusters_info_fd.readline()
      GlobalHulls = len(clusters_names.split(',')) - 2 

      fd.write(str(ExperimentNo) + ", " +
               str(NumTasks) + ", " + 
               str(FanIn) + ", " +
               str(NumBackends) + ", " + 
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

      ExperimentNo=ExperimentNo + 1
    FanIn=FanIn/2
  NumBackends=NumBackends/2

fd.close()
