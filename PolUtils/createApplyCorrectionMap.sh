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
    SANITY_CHECK=1
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
  san|sanityCheck )
    SANITY_CHECK=1
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
## execute command only if first passed argument is 1 or 1+1
## command is all but the first argument (making it possible to pass in arguments)
function condExecute() {
  if [ ${1} = "1" ] || [ ${1} = "1+1" ]; then
    shift
    $@
  fi
}

sample_input=even
sample_data=odd

nBinsPhi=16
nBinsCosTh=32

## inputs:
# file where the raw B to J/Psi K data is stored (needed for reference lambdas)
rawDataInput=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/CorrectionMaps/mw_3_rap_1_seagulls_${sample_input}/selEvents_data.root
# base file name where the background subtracted B to J/Psi K data can be found
bkgSubtrDataBJpsiKBase=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins_${sample_input}/dataResults/data/results_Psi1S
# base file name where the background subtracted data costh phi values can be found
# bkgSubtrDataBase=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/AllJpsi/results/inclusive_jpsi_full_1rapBin/results_Fit_1_Psi1S
bkgSubtrDataBase=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/ReferenceMapCreation/Seagulls/MassWindow_3sigma_1rapBins_${sample_data}/dataResults/data/results_Psi1S

# output directory, where intermediately created files and results will be stored
outputDir=/afs/hephy.at/data/tmadlener01/ChicPol/JpsiFromB/CorrectionMaps/mw_3_rap_1_seagulls_${sample_input}_${nBinsCosTh}_${nBinsPhi}

basicPlotJson=${PHYS_UTILS_DIR}/PolUtils/crossCheckGraphsBasic.json

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
refMapTest=${PHYS_UTILS_DIR}/python/PolUtils/plotReferenceMapsTest.py
histDivider=${PHYS_UTILS_DIR}/python/PolUtils/divideHistsMaps.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py
histPlotter=${PHYS_UTILS_DIR}/python/PlotUtils/plotAllHists.py
graphPlotter=${PHYS_UTILS_DIR}/python/PlotUtils/plotGraphs.py
jsonAdapter=${PHYS_UTILS_DIR}/python/PolUtils/alterGraphPlotJson.py
cosThPhiHistCreator=${PHYS_UTILS_DIR}/PolUtils/bin/runCreateCosThPhiHists

## files created on the fly
refMapsFile=${outputDir}/reference_maps.root
corrMapsFile=${outputDir}/correction_maps.root
corrDataFile=${outputDir}/corrData_hists.root
crossCheckFile=${outputDir}/cross_check_sanity.root
refLambdasJson=${outputDir}/reference_lambdas.json
refLambdasRoot=${outputDir}/reference_lambdas.root
# file where the background subtracted data costh phi histograms are stored (this is the "measurement" data)
dataHistFile=${outputDir}/data_costhphi_hists.root
# file where the background subtracted data (B to J/Psi K) cos th phi histograms are stored (needed for correction map)
dataHistBJpsiKFile=${outputDir}/data_costhphi_hists_BJpsiK.root

## setup the results directory
plotDir=${outputDir}/plots/
mkdir -p ${plotDir}


## create the reference maps from the json file (after calculating them from data)
condExecute ${CREATE_REF} ${refLambdasCalc} --input ${rawDataInput} --output ${refLambdasRoot} --jsonoutput ${refLambdasJson}
condExecute ${CREATE_REF} ${refMapCreator} --createmaps --fitmaps ${refLambdasJson} ${refMapsFile} --nBinsPhi=${nBinsPhi} --nBinsCosTh=${nBinsCosTh}
## to also have reference lambdas as TGraphAsymmErrors after fitting
condExecute ${FIT_REF} ${histFitter} --histrgx="^"${refMapBase} --graphbase="reference" ${refMapsFile}
condExecute ${CREATE_REF}+${MAKE_PLOTS} ${histPlotter} --histrgx="^"${refMapBase} --output-path=${plotDir} ${refMapsFile}
condExecute ${CREATE_REF}+${MAKE_PLOTS} ${refMapTest} --output=${plotDir} ${refMapsFile}

## create correction maps
condExecute ${CREATE_CORR} ${cosThPhiHistCreator} --inputbase ${bkgSubtrDataBJpsiKBase} --outputfile ${dataHistBJpsiKFile} --ptMin 1 --ptMax 12 --rapMin 1 --rapMax 1 --nBinsPhi ${nBinsPhi} --nBinsCosTh ${nBinsCosTh}
condExecute ${CREATE_CORR} ${histDivider} --numerator-base="^"${corrDataBase} --denominator-base="^"${refMapBase} --output-base=${corrMapBase} --create-covmap ${normHistsCorrMapCreation} ${dataHistBJpsiKFile} ${refMapsFile} ${corrMapsFile}
condExecute ${CREATE_CORR}+${MAKE_PLOTS} ${histPlotter} --output-path=${plotDir} ${corrMapsFile}

## apply to data (and fit results)
condExecute ${CREATE_DATA} ${cosThPhiHistCreator} --inputbase ${bkgSubtrDataBase} --outputfile ${dataHistFile} --ptMin 1 --ptMax 12 --rapMin 1 --rapMax 1 --nBinsPhi ${nBinsPhi} --nBinsCosTh ${nBinsCosTh}
condExecute ${CREATE_DATA} ${histDivider} --numerator-base="^"${corrDataBase} --denominator-base="^"${corrMapBase} --output-base=${dataOutBase} --create-covmap ${normHistsResults} ${dataHistFile} ${corrMapsFile} ${corrDataFile}
condExecute ${FIT_DATA} ${histFitter} --histrgx="^"${dataOutBase} --graphbase="results" ${corrDataFile}
condExecute ${CREATE_DATA}+${MAKE_PLOTS} ${histPlotter} --output-path=${plotDir} ${corrDataFile}

## make a sanity check by applying to the data which has originally be used
condExecute ${SANITY_CHECK} ${histDivider} --numerator-base="^"${corrDataBase} --denominator-base="^"${corrMapBase} --output-base="cross_check" ${normHistResults} ${dataHistBJpsiKFile} ${corrMapsFile} ${crossCheckFile}
condExecute ${SANITY_CHECK} ${histFitter} --histrgx="^cross_check" --graphbase="cross_check" ${crossCheckFile}
condExecute ${SANITY_CHECK}+${MAKE_PLOTS} ${jsonAdapter} -i ${outputDir}/"reference_lambdas.root" "reference" -i ${crossCheckFile} "sanity check" -i ${corrDataFile} "results" --outbase ${plotDir}"san_check" ${outputDir}/crossCheckPlots.json ${basicPlotJson}
condExecute ${SANITY_CHECK}+${MAKE_PLOTS} ${graphPlotter} ${outputDir}/crossCheckPlots.json
