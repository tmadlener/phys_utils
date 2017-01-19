#!/bin/bash

## script for generating correction maps from data histograms and B to J/Psi K input data

## TODO:
## + add data hist creation
## + add reference lambda json creation

for arg in "$@"; do
  case ${arg} in
  # this can probably be done better!
  all )
    CREATE_REF=1
    FIT_REF=1
    CREATE_CORR=1
    CREATE_DATA=1
    FIT_DATA=1
    ;;
  ref )
    CREATE_REF=1
    FIT_REF=1
    ;;
  data )
    CREATE_DATA=1
    FIT_DATA=1
    ;;
  createRef )
    CREATE_REF=1
    ;;
  fitRef )
    FIT_REF=1
    ;;
  createCorr|corr )
    CREATE_CORR=1
    ;;
  createData )
    CREATE_DATA=1
    ;;
  fitData )
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
# file where the raw B to J/Psi K data is stored (needed for reference lambdas)
rawDataInput=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins/tmpFiles/selEvents_data.root
# file where the background subtracted data (B to J/Psi K) cos th phi histograms are stored (needed for correction map)
dataHistBJpsiKFile=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins/data_costhphi_hists.root
# file where the background subtracted data costh phi histograms are stored (this is the "measurement" data)
dataHistFile=${dataHistBJpsiKFile}
# output directory, where intermediately created files and results will be stored
outputDir=$(pwd)/testdir

## options and other constants
# normHistsCorrMapCreation="--normalize" # empty/undefined for no normalization in correction map creation
# normHistsResults="--normalize" # empty/undefined for no normalization for division for results
# base names
refMapBase="cosThPhi_refMap" # the base name of the reference map histograms (NOTE: defined in runCreateRefMap.cc)
corrMapBase="correctionMap" # the base name of the correction map histograms
corrDataBase="costhphi" # base name of the data histograms used for correction map creation
dataOutBase="corr_costhphi" # the base name for the corrected data histograms


# check if the setup is done and do it if it's not yet done
if [ -z ${PHYS_UTILS_DIR+x} ]; then
  source ../setup.sh
fi

condExecute ${DO_BUILD} make -C ${PHYS_UTILS_DIR}/PolUtils -k all

## executables
refLambdasCalc=${PHYS_UTILS_DIR}/PolUtils/bin/runCalcRefLambdas
refMapCreator=${PHYS_UTILS_DIR}/python/PolUtils/createReferenceMaps.py
histDivider=${PHYS_UTILS_DIR}/python/PolUtils/divideHistsMaps.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py
histPlotter=${PHYS_UTILS_DIR}/python/PlotUtils/plotAllHists.py

## files created on the fly
refMapsFile=${outputDir}/reference_maps.root
corrMapsFile=${outputDir}/correction_maps.root
corrDataFile=${outputDir}/corr_data_costhphi_hists.root
refLambdasJson=${outputDir}/reference_lambdas.json
refLambdasRoot=${outputDir}/reference_lambdas.root

## setup the results directory
plotDir=${outputDir}/plots/
mkdir -p ${plotDir}


## create the reference maps from the json file (after calculating them from data)
condExecute ${CREATE_REF} ${refLambdasCalc} --input ${rawDataInput} --output ${refLambdasRoot} --jsonoutput ${refLambdasJson}
condExecute ${CREATE_REF} ${refMapCreator} --createmaps --fitmaps ${refLambdasJson} ${refMapsFile}
## to also have reference lambdas as TGraphAsymmErrors after fitting
condExecute ${FIT_REF} ${histFitter} --histrgx="^"${refMapBase} --graphbase="reference" ${refMapsFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --histrgx="^"${refMapBase} --output-path=${plotDir} ${refMapsFile}

## create correction maps
condExecute ${CREATE_CORR} ${histDivider} --numerator-base="^"${corrDataBase} --denominator-base="^"${refMapBase} --output-base=${corrMapBase} --create-covmap ${normHistsCorrMapCreation} ${dataHistBJpsiKFile} ${refMapsFile} ${corrMapsFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --output-path=${plotDir} ${corrMapsFile}

## apply to data (and fit results)
condExecute ${CREATE_DATA} ${histDivider} --numerator-base="^"${corrDataBase} --denominator-base="^"${corrMapBase} --output-base=${dataOutBase} --create-covmap ${normHistsResults} ${dataHistFile} ${corrMapsFile} ${corrDataFile}
condExecute ${FIT_DATA} ${histFitter} --histrgx="^"${dataOutBase} --graphbase="results" ${corrDataFile}
condExecute ${MAKE_PLOTS} ${histPlotter} --output-path=${plotDir} ${corrDataFile}
