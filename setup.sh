#!/bin/bash

# setup script

## NOTE: need c++11 support and a working setup for using ROOT in python
## this is easiest to get by setting up a cmsenv (tested with CMSSW_8_0_X)
## this has to be done before calling this script!

## get the absolute directory of this script, regardless of where it's called from
export PHYS_UTILS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

## add the python directory to the python search path
PYTHONPATH=$PYTHONPATH:${PHYS_UTILS_DIR}/python

## build all the utilities in the PolUtils folder
make -C $PHYS_UTILS_DIR/PolUtils -k -j4 all
