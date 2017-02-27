#!/bin/bash

# load condExecute and parseArgs
source ${PHYS_UTILS_DIR}/PolUtils/sh/bash_helper.sh
parseArgs $@ # sets variables CREATE, FIT and PLOT depending on passed args

# topDir=${MY_DATA_DIR}/ChicPol/InclusiveJpsiResults # top directory of output
topDir=${MY_DATA_DIR}/ChicPol/TestMe # top directory of output

Binning=JpsiBinning
ptBinning="10 12 14 16 18 20 22 25 30 35 40 50 70"
rapBinning="0 1.2"

rawDataFile=${MY_DATA_DIR}/ChicPol/JpsiFromB/ReferenceMapCreation/${Binning}/FullSet/MassWindow_3sigma_1rapBins/tmpFiles/selEvents_data.root

outputDir=${topDir}/ReferenceMaps/${Binning}/FullSet
mkdir -p ${outputDir}

# executables
refLamCalculator=${PHYS_UTILS_DIR}/PolUtils/bin/runCalcRefLambdas
refMapCreator=${PHYS_UTILS_DIR}/python/PolUtils/createReferenceMaps.py
histPlotter=${PHYS_UTILS_DIR}/python/PlotUtils/plotAllHists.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py
refMapTestPlotter=${PHYS_UTILS_DIR}/python/PolUtils/plotReferenceMapsTest.py
jsonAdapter=${PHYS_UTILS_DIR}/python/PolUtils/alterGraphPlotJson.py
plotGraphs=${PHYS_UTILS_DIR}/python/PlotUtils/plotGraphs.py

# create the reference maps (in different binnings)
for ctBins in 32 16; do
  for pBins in 16 8; do
    binStr="ct_"${ctBins}"_p_"${pBins}
    refLambdaRoot=${outputDir}/"refLambdas_full_"${binStr}".root"
    refLambdasJson=${outputDir}/"refLambdas_full_"${binStr}".json"
    condExecute ${CREATE} ${refLamCalculator} --input ${rawDataFile} --output ${refLambdaRoot} --jsonoutput ${refLambdasJson} --ptBinning ${ptBinning} --rapBinning ${rapBinning}

    refMapsFile=${outputDir}/"refMaps_"${binStr}".root"
    condExecute ${CREATE} ${refMapCreator} --createmaps --fitmaps --nBinsPhi=${pBins} --nBinsCosTh=${ctBins} ${refLambdasJson} ${refMapsFile}

    # plot reference maps
    condExecute ${PLOT} mkdir -p ${outputDir}/${binStr}
    condExecute ${PLOT} ${histPlotter} --histrgx="^cosThPhi_refMap" --output=${outputDir}/${binStr}/ ${refMapsFile}
    # fit the reference maps, to get easily plottable TGraphAsymmErrors
    condExecute ${FIT} ${histFitter} --ptBinning ${ptBinning} --histrgx="^cosThPhi_refMap" --graphbase="reference" ${refMapsFile}
  done
done


# make the test plots
# construct the inputfiles argument from the reference map filenames
inputfiles=""
for f in ${outputDir}/refMaps*.root; do
  ctBins=$(basename ${f} | awk -F'[_.]' '{print $3}')
  pBins=$(basename ${f} | awk -F'[_.]' '{print $5}')
  inputfiles=${inputfiles}" --inputfile "${f}" "${ctBins}"/"${pBins}
done

mkdir -p ${outputDir}/testPlots
condExecute ${PLOT} ${refMapTestPlotter} ${inputfiles} --output ${outputDir}/testPlots/

# take one of the above created refLambdas as additional inputs files (all of them contain the same graphs anyways)
# NOTE: make sure that it is actually created or chose one that has been created
condExecute ${PLOT} ${jsonAdapter} ${inputfiles} --inputfile ${outputDir}/"refLambdas_full_ct_16_p_16.root" --outbase ${outputDir}/testPlots/fit_v_ref ${outputDir}/plotRefLambdas_lth.json referenceGraphs_lth_base.json
condExecute ${PLOT} ${plotGraphs} ${outputDir}/plotRefLambdas_lth.json

condExecute ${PLOT} ${jsonAdapter} ${inputfiles} --inputfile ${outputDir}/"refLambdas_full_ct_16_p_16.root" --outbase ${outputDir}/testPlots/fit_v_ref ${outputDir}/plotRefLambdas.json referenceGraphs_base.json
condExecute ${PLOT} ${plotGraphs} ${outputDir}/plotRefLambdas.json
