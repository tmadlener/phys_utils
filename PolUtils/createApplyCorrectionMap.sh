#!/bin/bash

## script for generating correction maps from data histograms and B to J/Psi K input data

## TODO:
## + add data hist creation
## + add reference lambda json creation

for arg in "$@"; do
  case ${arg} in
  createRef|ref|all )
    CREATE_REF=1
    ;&
  fitRef|ref|all )
    FIT_REF=1
    ;&
  createCorr|corr|all )
    CREATE_CORR=1
    ;&
  createData|data|all )
    CREATE_DATA=1
    ;&
  fitData|data|all )
    FIT_DATA=1
    ;;
  plot )
    MAKE_PLOTS=1
    ;;
  build )
    DO_BUILD=1
    ;;
  esac
done

## small helper function for slightly less typing
## execute command only if first passed argument is 1
## command is all but the first argument (making it possible to pass in arguments)
function condExecute() {
  if [ ${1} = "1" ]; then
    shift
    $@
  fi
}

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

condExecute ${DO_BUILD} make -C ${PHYS_UTILS_DIR}/PolUtils -k all

## executables
refMapCreator=${PHYS_UTILS_DIR}/python/PolUtils/createReferenceMaps.py
histDivider=${PHYS_UTILS_DIR}/python/PolUtils/divideHistsMaps.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py
histPlotter=${PHYS_UTILS_DIR}/python/PlotUtils/plotAllHists.py

## files created on the fly
refMapsFile=${outputDir}/reference_maps.root
corrMapsFile=${outputDir}/correction_maps.root
corrDataFile=${outputDir}/corr_data_costhphi_hists.root

## setup the results directory
mkdir -p ${outputDir}/plots


## create the reference maps from the json file
condExecute ${CREATE_REF} ${refMapCreator} --createmaps --fitmaps ${refLambdasJson} ${refMapsFile}
## to also have reference lambdas as TGraphAsymmErrors after fitting
condExecute ${FIT_REF} ${histFitter} --histrgx="^cosThPhi_refMap" --graphbase="reference" ${refMapsFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --histrgx="^cosThPhi_refMap" --output-path=${outputDir}/plots ${refMapsFile}

## create correction maps
condExecute ${CREATE_CORR} ${histDivider} --numerator-base="^costhphi_" --denominator-base="^cosThPhi_refMap" --output-base=correctionMap --create-covmap ${dataHistBJpsiKFile} ${refMapsFile} ${corrMapsFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --output-path=${outputDir}/plots ${corrMapsFile}

## apply to data (and fit results)
condExecute ${CREATE_DATA} ${histDivider} --numerator-base="^costhphi" --denominator-base="^correctionMap" --output-base=corr_costhphi --create-covmap ${dataHistFile} ${corrMapsFile} ${corrDataFile}
condExecute ${FIT_DATA} ${histFitter} --histrgx=corr_costhphi --graphbase="" ${corrDataFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --output-path=${outputDir}/plots ${corrDataFile}
