#!/bin/bash

# topDir=${MY_DATA_DIR}/ChicPol/InclusiveJpsiResults # top directory of output
topDir=${MY_DATA_DIR}/ChicPol/TestMe

## prompt or NP input data
rawDataDir=${MY_DATA_DIR}/ChicPol/JpsiFromB/AllJpsi/results
# dataName=inclusive_NP_2012_1rapBin # JobID in runDataFits
dataName=inclusive_jpsi_full_1rapBin # JobID in runDataFits
rawDataBase=${rawDataDir}/${dataName}
# histBase="np_costhphi"
histBase="pr_costhphi"

# ## B to J/psi K
# DataSet=Seagulls
# rawDataDir=${MY_DATA_DIR}/ChicPol/JpsiFromB/ReferenceMapCreation/JpsiBinning/${DataSet}/MassWindow_3sigma_1rapBins/dataResults/data
# histBase="bjpsik_costhphi" # base name of the produced histograms
# dataName=BtoJpsiK_${DataSet} # slightly different treatment necessary for B to Jpsi K data
# rawDataBase=${rawDataDir}

Binning=JpsiBinning

outputDir=${topDir}/DataHistograms/${Binning}/${dataName}
mkdir -p ${outputDir}

dataHistCreator=~/phys_utils/PolUtils/bin/runCreateCosThPhiHists

for cosThBins in 16 32; do
  for phiBins in 16 8; do
    for i in {1..10}; do # currently have only 10 fits available
    # for i in 1; do # B to Jpsi K (no bkg -> no differences between fits)
      outFile=${outputDir}/"data_costhphi_hists_fit_"${i}"_ct_"${cosThBins}"_p_"${phiBins}.root
      inputBase=${rawDataBase}/"results_Fit_"${i}"_Psi1S"
      ${dataHistCreator} --inputbase ${inputBase} --outputfile ${outFile} --histbase ${histBase} --ptMin 1 --ptMax 12 --rapMin 1 --rapMax 1 --nBinsPhi ${phiBins} --nBinsCosTh ${cosThBins}
    done
  done
done
