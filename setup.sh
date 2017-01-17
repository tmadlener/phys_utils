#!/bin/bash

# setup script

## get the absolute directory of this script, regardless of where it's called from
export PHYS_UTILS_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd -P)"

## add the python directory to the python search path
PYTHONPATH=$PYTHONPATH:${PHYS_UTILS_DIR}/python
