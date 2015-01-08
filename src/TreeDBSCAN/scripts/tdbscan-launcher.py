import os
import sys
import subprocess
import threading

# MRNEt topgen binary
MRNET_HOME      = subprocess.check_output("@sub_MRNETAPP_HOME@/bin/mrnapp-config --mrnet", shell=True).rstrip("\n")
mrnet_topgen    = MRNET_HOME + "/bin/mrnet_topgen"
TDBSCAN_HOME    = "@sub_PREFIX@"
TDBSCAN_FE      = TDBSCAN_HOME + "/bin/TDBSCAN_FE"
TDBSCAN_FE_NOBE = TDBSCAN_HOME + "/bin/TDBSCAN_FE_nobe"
TDBSCAN_BE_NOBE = TDBSCAN_HOME + "/bin/TDBSCAN_BE_nobe"

# Global variables
FrontEndHost      = "localhost"
TopologyFile      = ".tdbscan-topology.txt"
HostsFile         = ".tdbscan-resources.txt"
HostsAppl         = ".tdbscan-resources-appl.txt"
HostsTree         = ".tdbscan-resources-tree.txt"
ConnectionsFile   = ".tdbscan-connections.txt"
BackEndAttachMode = False

# Parses the resources assigned to the execution
def GetResources():
  global FrontEndHost
  global HostsFile
  global HostsAppl
  global HostsTree
  global BackEndAttachMode
  AllHosts = ()

  # Support for MareNostrum
  if (os.environ.has_key("LSB_DJOB_HOSTFILE")):
    LSB_HostFile = os.getenv("LSB_DJOB_HOSTFILE");
    print "LSF scheduler detected. Parsing available resources from LSB_DJOB_HOSTFILE file '" + LSB_HostFile + "'...";
    AllHosts = [Host.strip() for Host in open(LSB_HostFile)]
    for i in range(0, len(AllHosts)):
      AllHosts[i] = AllHosts[i]

  # Support for BlueWaters
  elif (os.environ.has_key("PBS_NODEFILE")):
    PBS_HostFile = os.getenv("PBS_NODEFILE");
    print "PBS scheduler detected. Parsing available resources from PBS_NODEFILE file '" + PBS_HostFile + "'...";
    AllHosts = [Host.strip() for Host in open(PBS_HostFile)]
    for i in range(0, len(AllHosts)):
      Host = AllHosts[i]
      AllHosts[i] = "nid%05u" % Host

  # Support for interactive runs
  elif (os.environ.has_key("TDBSCAN_HOSTS")):
    User_HostFile = os.getenv("TDBSCAN_HOSTS")
    print "User-defined host list detected. Parsing available hosts from TDBSCAN_HOSTS file '" + User_HostFile + "'..."
    AllHosts = [Host.strip() for Host in open(User_HostFile)]

  else:
    sys.exit("Any known job scheduler detected. Please specify the resources by setting the environment TDBSCAN_HOSTS pointing to the list of available hosts.");

  #print AllHosts
  fd = open(HostsFile, 'w')
  for Host in AllHosts:
    print >> fd, Host
  fd.close()

  # Front-end is the first host available
  FrontEndHost = AllHosts[0]+"-ib0"

  if (BackEndAttachMode):

    fd = open(HostsTree, 'w')
    for Host in AllHosts[0:len(AllHosts)-NumBackEnds] :
      print >> fd, Host+"-ib0"
    fd.close()

    fd = open(HostsAppl, 'w')
    for Host in AllHosts[len(AllHosts)-NumBackEnds:]:
      print >> fd, Host
    fd.close()

  else:

    fd = open(HostsTree, 'w')
    for Host in AllHosts:
      print >> fd, Host+"-ib0"
    fd.close() 

    fd = open(HostsAppl, 'w')
    fd.close() 


class RunProcess(threading.Thread):
     def __init__(self, cmd):
         super(RunProcess, self).__init__()
         self.cmd = cmd

     def run(self):
         p = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
 
         while True:
           out = p.stdout.read(1)
           if out == '' and p.poll() != None:
              break
           if out != '':
              sys.stdout.write(out)
              sys.stdout.flush()
  

############
### MAIN ###
############

run_topgen = True

# Check arguments
if len(sys.argv) < 4:
    sys.exit('Usage: %s <instantiation-mode> <topology-spec> <clustering-args>' % sys.argv[0])

InstantiationMode = int(sys.argv[1])
TopologySpec      = sys.argv[2]
ClusteringArgs    = sys.argv[3:]

if (InstantiationMode == 0):
  BackEndAttachMode = False
elif (InstantiationMode == 1):
  BackEndAttachMode = True
else:
  print "Unknown instantiation method"
  sys.exit(-1)

# Parse the topology specification to see the number of back-ends
LastLevel = TopologySpec.split(':')[-1].split('x')
if (len(LastLevel) == 1):
  NumBackEnds = int(LastLevel[0])
elif (len(LastLevel) == 2):
  NumBackEnds = int(LastLevel[0]) * int(LastLevel[1])
else: 
  print "Wrong topology specification '" + TopologySpec + "'"
  sys.exit(-1)

if (BackEndAttachMode):
  # To generate the topology in the back-end attach mode, we don't have to specify the last level
  TopologySpec = ':'.join(TopologySpec.split(':')[0:-1])
  if (TopologySpec == "g"):
    run_topgen = False
 
GetResources()

# Generate the topology 
if (run_topgen):
  cmd = mrnet_topgen + " --fehost=" + FrontEndHost + " --hosts=" + HostsTree + " --topology=" + TopologySpec + " -o " + TopologyFile

  print "Running the topology generator..."
  print cmd
  rc = os.system( cmd )
  if (rc != 0):
    sys.exit('Could not generate the topology file. See errors above...')

else:
  # Root-only topology in the back-end attach case
  fd = open(TopologyFile, 'w')
  print >> fd, FrontEndHost+":0;"
  fd.close()

print "Topology written to '" + TopologyFile + ":"
os.system( 'cat ' + TopologyFile )

# Run the TDBSCAN front-end
print "Setting MRNAPP_TOPOLOGY to '" + TopologyFile + "':"
print "Setting MRNET_STARTUP_TIMEOUT to '300'"
print "Setting MRNAPP_NUM_BE to " + str(NumBackEnds)
print "Setting MRNAPP_BE_CONNECTIONS to '" + ConnectionsFile + "'"
os.environ["MRNAPP_TOPOLOGY"] = TopologyFile
os.environ['MRNET_STARTUP_TIMEOUT'] = '300'
os.environ['MRNAPP_NUM_BE'] = str(NumBackEnds)
os.environ['MRNAPP_BE_CONNECTIONS'] = ConnectionsFile

if (BackEndAttachMode):
  cmd = TDBSCAN_FE_NOBE + " " + " ".join(ClusteringArgs)
else:
  cmd = TDBSCAN_FE + " " + " ".join(ClusteringArgs)
print "Running the front-end:"
print cmd
FE_thread = RunProcess(cmd)
FE_thread.start()

if (BackEndAttachMode):
  os.system("sleep 10")
  cmd = "mpirun -np " + str(NumBackEnds) + " -machinefile " + HostsAppl + " " + TDBSCAN_BE_NOBE 
  print "Running the back-ends:"
  print cmd
  BE_thread = RunProcess(cmd)
  BE_thread.start()

FE_thread.join()
if (BackEndAttachMode):
  BE_thread.join()

 
  
