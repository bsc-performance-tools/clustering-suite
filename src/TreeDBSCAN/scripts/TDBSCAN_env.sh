#!/bin/bash

source @sub_SYNAPSE_HOME@/etc/sourceme.sh

export TDBSCAN_HOME=@sub_PREFIX@
export SYNAPSE_FILTER_PATH=$TDBSCAN_HOME/lib:$SYNAPSE_FILTER_PATH
export LD_LIBRARY_PATH=@sub_SYNAPSE_LIBSDIR@:@sub_CLUSTERING_LD_LIBRARY_PATH@:$TDBSCAN_HOME/lib:$LD_LIBRARY_PATH
export PATH=$TDBSCAN_HOME/bin:$PATH

