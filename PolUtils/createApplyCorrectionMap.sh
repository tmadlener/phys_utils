#!/bin/bash

## script for generating correction maps from data histograms and B to J/Psi K input data

## TODO:
## + flags and some more automatization
## + add data hist creation
## + add reference lambda json creation

## inputs:
# file where the reference lambdas are stored (used to create the reference maps)
refLambdasJson=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins/data_lambdas_reference.json
# file where the background subtracted data (B to J/Psi K) cos th phi histograms are stored (needed for correction map)
dataHistBJpsiKFile=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins/data_costhphi_hists.root
# file where the background subtracted data costh phi histograms are stored (this is the "measurement" data)
dataHistFile=${dataHistBJpsiKFile}
# output directory, where intermediately created files and results will be stored
outputDir=$(pwd)/testdir

# check if the setup is done and do it if it's not yet done
if [ -z ${PHYS_UTILS_DIR+x} ]; then
  source ../setup.sh
fi

# remake all (TODO, make toggleable via flag, check make return value)
cd ${PHYS_UTILS_DIR}/PolUtils
make -k all
cd ${PHYS_UTILS_DIR}

## executables
refMapCreator=${PHYS_UTILS_DIR}/python/PolUtils/createReferenceMaps.py
histDivider=${PHYS_UTILS_DIR}/python/PolUtils/divideHistsMaps.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py


## files created on the fly
refMapsFile=${outputDir}/reference_maps.root
corrMapsFile=${outputDir}/correction_maps.root
corrDataFile=${outputDir}/corr_data_costhphi_hists.root

## setup the results directory
mkdir -p ${outputDir}

## create the reference maps from the json file
${refMapCreator} --createmaps --fitmaps ${refLambdasJson} ${refMapsFile}
## to also have reference lambdas as TGraphAsymmErrors after fitting
${histFitter} --histrgx="^cosThPhi_refMap" --graphbase="reference" ${refMapsFile}

## create control maps
${histDivider} --numerator-base="^costhphi_" --denominator-base="^cosThPhi_refMap" --output-base=correctionMap --create-covmap ${dataHistBJpsiKFile} ${refMapsFile} ${corrMapsFile}

## apply to data (and fit results)
${histDivider} --numerator-base="^costhphi" --denominator-base="^correctionMap" --output-base=corr_costhphi --create-covmap ${dataHistFile} ${corrMapsFile} ${corrDataFile}
${histFitter} --histrgx=corr_costhphi --graphbase="" ${corrDataFile}
