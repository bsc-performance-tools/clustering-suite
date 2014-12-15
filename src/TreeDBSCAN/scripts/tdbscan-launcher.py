import os
import sys
import subprocess

# MRNEt topgen binary
MRNET_HOME   = subprocess.check_output("@sub_MRNETAPP_HOME@/bin/mrnapp-config --mrnet", shell=True).rstrip("\n")
mrnet_topgen = MRNET_HOME + "/bin/mrnet_topgen"
TDBSCAN_HOME = "@sub_PREFIX@"
TDBSCAN_FE   = TDBSCAN_HOME + "/bin/TDBSCAN_FE"

# Global variables
TopologyFile  = ".tdbscan-topology.txt"
FrontEndHost  = "localhost"
InternalHosts = ".tdbscan-resources.txt"

# Parses the resources assigned to the execution
def GetResources():
  global FrontEndHost
  global InternalHosts
  AllHosts = ()

  if (os.environ.has_key("LSB_DJOB_HOSTFILE")):
    LSB_HostFile = os.getenv("LSB_DJOB_HOSTFILE");
    print "LSF scheduler detected. Parsing available resources from LSB_DJOB_HOSTFILE file '" + LSB_HostFile + "'...";
    AllHosts = [Host.strip() for Host in open(LSB_HostFile)]

  # Support for BlueWaters
  elif (os.environ.has_key("PBS_NODEFILE")):
    PBS_HostFile = os.getenv("PBS_NODEFILE");
    print "PBS scheduler detected. Parsing available resources from PBS_NODEFILE file '" + PBS_HostFile + "'...";
    AllHosts = [Host.strip() for Host in open(PBS_HostFile)]
    for i in range(0, len(AllHosts)):
      Host = AllHosts[i]
      AllHosts[i] = "nid" + str(100000 + int(Host))[1:]

  elif (os.environ.has_key("TDBSCAN_HOSTS")):
    User_HostFile = os.getenv("TDBSCAN_HOSTS")
    print "User-defined host list detected. Parsing available hosts from TDBSCAN_HOSTS file '" + User_HostFile + "'..."
    AllHosts = [Host.strip() for Host in open(User_HostFile)]

  else:
    sys.exit("Any known job scheduler detected. Please specify the resources by setting the environment TDBSCAN_HOSTS pointing to the list of available hosts.");

  #print AllHosts

  # Front-end is the first host available
  FrontEndHost = AllHosts[0]

  # All the rest are for the CP's and BE's, write the hosts in a file
  fd = open(InternalHosts, 'w')
  for Host in AllHosts:
    print >> fd, Host
  

############
### MAIN ###
############

# Check arguments
if len(sys.argv) < 3:
    sys.exit('Usage: %s <topology-spec> <clustering-args>' % sys.argv[0])

TopologySpec   = sys.argv[1]
ClusteringArgs = sys.argv[2:]

GetResources()

#cmd = mrnet_topgen + " --fehost=" + FrontEndHost + " --hosts=" + InternalHosts + " --topology=" + TopologySpec + " -o " + TopologyFile
cmd = mrnet_topgen + " --hosts=" + InternalHosts + " --topology=" + TopologySpec + " -o " + TopologyFile

print "Running the topology generator..."
print cmd
rc = os.system( cmd )
if (rc != 0):
  sys.exit('Could not generate the topology file. See errors above...')
else:
  print "Topology written to '" + TopologyFile + ":"
  os.system( 'cat ' + TopologyFile )

# Run the TDBSCAN front-end
print "Setting MRNAPP_TOPOLOGY to '" + TopologyFile + ":"
os.environ["MRNAPP_TOPOLOGY"] = TopologyFile

cmd = TDBSCAN_FE + " " + " ".join(ClusteringArgs)
print "Running the front-end:"
print cmd

rc = os.system( cmd )


