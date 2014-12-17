
import os
import sys

NumBackends = int(sys.argv[1])
CSVPrefix = sys.argv[2]
OutputFile = sys.argv[3]

for Rank in range(0, NumBackends):

  if (Rank == 0):
    cmd = "head -n 1 " + CSVPrefix + str(Rank) + ".csv > " + OutputFile
    print cmd 
    os.system(cmd)

  cmd = "tail -qn +2 " + CSVPrefix + str(Rank) + ".csv >> " + OutputFile 
  print cmd 
  os.system(cmd)
