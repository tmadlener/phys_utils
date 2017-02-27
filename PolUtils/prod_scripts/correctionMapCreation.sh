#!/bin/bash

# topDir=${MY_DATA_DIR}/ChicPol/InclusiveJpsiResults # top directory of output
topDir=${MY_DATA_DIR}/ChicPol/TestMe # top directory of output

Binning=JpsiBinning
dataHistBase=${topDir}/DataHistograms/${Binning}

# ## B to J/psi K corrections
# corrDataName=BtoJpsiK_Seagulls
# dataHistRgx="^bjpsik_costhphi" # regex for the data histograms
# corrMapBase="bjpsik_corrmap" # base name of the created correction maps

## NP corrections
corrDataName=inclusive_NP_2012_1rapBin
dataHistRgx="^np_costhphi" # regex for the data histograms
corrMapBase="np_corrmap" # base name of the created correction maps

refMapRgx="^cosThPhi_refMap" # NOTE: this is fixed in the reference map creation
refMapBase=${topDir}/ReferenceMaps/${Binning}/FullSet

outputDir=${topDir}/CorrectionMaps/${Binning}/${corrDataName}
mkdir -p ${outputDir}

histDivider=~/phys_utils/python/PolUtils/divideHistsMaps.py

for ctBins in 32 16; do
  for pBins in 16 8; do
    for fit in {1..10}; do
      binStr="ct_"${ctBins}"_p_"${pBins}

      # input files
      dataFile=${dataHistBase}/${corrDataName}/"data_costhphi_hists_fit_"${fit}"_"${binStr}".root"
      referenceMaps=${refMapBase}/"refMaps_"${binStr}".root"

      # output file containing the correction maps
      corrMaps=${outputDir}/"corrMaps_fit_"${fit}"_"${binStr}".root"

      ${histDivider} --numerator-base ${dataHistRgx} --denominator-base ${refMapRgx} --output-base ${corrMapBase} ${dataFile} ${referenceMaps} ${corrMaps}
    done
  done
done
