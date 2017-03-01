#!/bin/bash

topDir=${MY_DATA_DIR}/ChicPol/InclusiveJpsiResults # top directory of output

Binning=JpsiBinning
ptBinning="10 12 14 16 18 20 22 25 30 35 40 50 70"

# ## B to J/psi K corrections
# correctionData=BtoJpsiK_Seagulls
# corrMapRgx="^bjpsik_corrmap"

## NP corrections
correctionData=inclusive_NP_2012_1rapBin
corrMapRgx="^np_corrmap"

inputData=inclusive_jpsi_full_1rapBin
dataHistRgx="^pr_costhphi"

# for relErrCut in 0.15 0.25 0.3 0.5 1.0; do
# resultBase=${topDir}/DataResults/${Binning}/corr_${correctionData}_data_${inputData}/relErr_${relErrCut}

relErrCut=0.2

resultBase=${topDir}/DataResults/${Binning}/corr_${correctionData}_data_${inputData}
mkdir -p ${resultBase}/indResults
inputDataBase=${topDir}/DataHistograms/${Binning}/${inputData}
corrMapBase=${topDir}/CorrectionMaps/${Binning}/${correctionData}

histDivider=${PHYS_UTILS_DIR}/python/PolUtils/divideHistsMaps.py
histFitter=${PHYS_UTILS_DIR}/python/PolUtils/fitHistsMaps.py
averager=${PHYS_UTILS_DIR}/python/PolUtils/averageResults.py

for ctBins in 32 16; do
  for pBins in 16 8; do
    binStr="ct_"${ctBins}"_p_"${pBins}
    for fit in {1..10}; do
    # for fit in 1; do # B to J/psi K
      corrMapFile=${corrMapBase}/"corrMaps_fit_"${fit}"_"${binStr}".root"

      dataFile=${inputDataBase}/"data_costhphi_hists_fit_"${fit}"_"${binStr}".root"
      outputFile=${resultBase}/indResults/"corr_costhphi_hists_fit_"${fit}"_"${binStr}".root"

      ${histDivider} --relerr-cut ${relErrCut} --numerator-base ${dataHistRgx} --denominator-base ${corrMapRgx} --output-base "corr_costhphi" ${dataFile} ${corrMapFile} ${outputFile}

      # fit histograms
      ${histFitter} --histrgx "^corr_costhphi" --ptBinning ${ptBinning} --graphbase "result" ${outputFile}
    done
    # calculate average of the individual fit results
    resultFile=${resultBase}/"result_graphs_"${binStr}".root"
    ${averager} --outputfile ${resultFile} ${resultBase}/indResults/*${binStr}.root
  done
done
# done # relErrCut loop
