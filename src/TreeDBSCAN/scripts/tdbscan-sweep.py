import os
import subprocess
import sys

Launcher="@sub_PREFIX@/bin/tdbscan-launcher"
ClustersDiff="@sub_PREFIX@/bin/ClustersDiff"
ClustersSequenceScore="@sub_PREFIX@/bin/ClustersSequenceScore"

OutputPrefix=""
OutputSuffix="_sweep_results.csv"

if ((len(sys.argv) < 6) or (len(sys.argv) > 7)):
  print "Usage: $0 <min-backends> <max-backends> <xml> <input-trace> <output-prefix> [reference-trace]"
  sys.exit(-1)

MinBackends   = int(sys.argv[1])
MaxBackends   = int(sys.argv[2])
ClusteringXML = sys.argv[3]
InputTrace    = sys.argv[4]
OutputPrefix  = sys.argv[5]
if (len(sys.argv) == 7):
  HaveReference = True
  ReferenceTrace = sys.argv[6]
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

    # Launch TDBSCAN
    cmd = Launcher + " " + TopologySpec + " " + ClusteringInput + " -o " + OutputPrefix + "_output_" + str(ExperimentNo) + ".prv"
    print "Running: " + cmd

    output = ""
    try:
      output = subprocess.check_output( cmd.split(' ') , stderr=subprocess.STDOUT)
      print output
    except subprocess.CalledProcessError as error:
      print "Error!"
      print error
      sys.exit(-1)

    performance_output = [line for line in output.split('\n') if "[CSV]" in line]
    performance_output = performance_output[0][6:]

    # Check Mirkin Distance and Sequence Score
    if (HaveReference):
      ReferenceCSVSorted = ReferenceCSV + ".sorted.csv"
      FinalCSV = OutputPrefix + "_output_" + str(ExperimentNo) + ".FINAL.DATA.csv"
      FinalCSVSorted = FinalCSV + ".sorted.csv"
     
      os.system( "sort -t, -h -k5 " + ReferenceCSV + " > " + ReferenceCSVSorted )
      os.system( "sort -t, -h -k5 " + FinalCSV + " > " + FinalCSVSorted )
 
      cmd = ClustersDiff + " -1 " + FinalCSVSorted + " -2 " + ReferenceCSVSorted
      print "Running: " + cmd
      output = subprocess.check_output(cmd, shell=True)
      MirkinDistance = float(output)
    else:
      MirkinDistance = 0

    cmd = ClustersSequenceScore + " output_" + str(ExperimentNo) + ".FINAL.DATA.csv"
    try:
      output = subprocess.check_output(cmd, shell=True)
      SequenceScore = float(output)
    except:
      SequenceScore = 0

    performance_output = str(ExperimentNo) + ", " + str(NumTasks) + ", " + str(FanIn) + ", " + performance_output + ", " + str(MirkinDistance) + ", " + str(SequenceScore)
    print performance_output
    fd.write( performance_output + "\n" )

    ExperimentNo=ExperimentNo + 1
    FanIn=FanIn/2

  NumBackends=NumBackends/2

fd.close()
