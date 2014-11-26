#! /bin/bash

REAL_PATH=`readlink -f "${0}"`
CLUSTERING_HOME="$( cd "$( dirname "$REAL_PATH" )" && pwd )"
CLUSTERING_HOME=${CLUSTERING_HOME%/bin}

if ! [ -f "${CLUSTERING_HOME}/lib/libTraceClustering.so" -a \
 -f "${CLUSTERING_HOME}/lib/libBasicClasses.so" -a \
 -f "${CLUSTERING_HOME}/lib/libparavertraceconfig.so" -a \
 -f "${CLUSTERING_HOME}/lib/libClustering.so" ] ; then
     
  echo "Basic clustering libraries not found in \$CLUSTERING_HOME/lib, please check your installation"
  exit 1

elif ! [ -e "${CLUSTERING_HOME}/bin/ClustersSequenceScore.bin" ] ; then

  echo "ClustersSequenceScore binary not found in \$CLUSTERING_HOME/bin/"
  exit 1
  
fi

LD_LIBRARY_PATH="${CLUSTERING_HOME}/lib:$LD_LIBRARY_PATH" "${CLUSTERING_HOME}/bin/ClustersSequenceScore.bin" "$@"
