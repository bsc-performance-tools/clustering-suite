import os
import subprocess
import sys


if (len(sys.argv) != 6):
  print "Usage: $0 <template> <iterations> <min-backends> <max-backends> <output-prefix>"
  sys.exit(-1)

Template      = sys.argv[1]
NumIters      = int(sys.argv[2])
MinBackends   = int(sys.argv[3])
MaxBackends   = int(sys.argv[4])
OutputPrefix  = sys.argv[5]
ExperimentTag = "_experiment_"

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

    TotalResources = 1
    while (ProcsPerLvl != NumBackends):
      TotalResources += ProcsPerLvl 
      TopologySpec=TopologySpec + str(ProcsPerLvl) + "x" + str(FanIn) + ":"
      ProcsPerLvl=ProcsPerLvl * FanIn
    TotalResources += NumBackends

    Queue = "bsc_cs"
    if (TotalResources >= 2048):
      Queue = "xlarge"
    
    TopologySpec = "g:" + TopologySpec[0:-1]
    print "NumBackends: ", NumBackends, "FanIn: ", str(FanIn), "TopologySpec: ", TopologySpec, "TotalResources: ", TotalResources

    for i in range(1, NumIters+1):
      ExperimentDir = "experiment_" + str(ExperimentNo)
      cmd = "mkdir " + ExperimentDir 
      os.system(cmd)
      cmd = "cp " + Template + " " + ExperimentDir
      os.system(cmd)

      cmd = "sed -i 's/@SWEEP_RESOURCES@/" + str(TotalResources) + "/g' " + ExperimentDir+ "/" + Template 
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_QUEUE@/" + Queue + "/g' " + ExperimentDir+ "/" + Template 
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_TOPOLOGY@/" + TopologySpec + "/g' " + ExperimentDir+ "/" + Template 
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_OUTPUT@/" + OutputPrefix + ExperimentTag + str(ExperimentNo) + "/g' " + ExperimentDir+ "/" + Template 
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_EXPERIMENT@/" + str(ExperimentNo) + "/g' " + ExperimentDir+ "/" + Template
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_FAN@/" + str(FanIn) + "/g' " + ExperimentDir+ "/" + Template
      os.system(cmd)
      cmd = "sed -i 's/@SWEEP_NUM_BACKENDS@/" + str(NumBackends) + "/g' " + ExperimentDir+ "/" + Template
      os.system(cmd)

      cmd = "bsub < " + ExperimentDir+ "/" + Template
      print cmd
      os.system(cmd)

      ExperimentNo=ExperimentNo + 1
    FanIn=FanIn/2
  NumBackends=NumBackends/2

