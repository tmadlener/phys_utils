#include "general/ArgParser.h"
#include "general/progress.h"
#include "general/vector_helper.h"


#include "root_utils.h"
#include "misc_utils.h"
#include "referenceMapCreation.h"

#include "TFile.h"
#include "TTree.h"
#include "TLorentzVector.h"
#include "TGraphAsymmErrors.h"

#include <string>
#include <vector>
#include <array>
#include <cmath> // abs, sqrt, cos, sin
#include <sstream>

/** create an NxM array of (empty) vector<double> */
std::vector<std::vector<std::vector<double> > > createArray(const size_t N, const size_t M)
{
  using Vec = std::vector<double>;
  using VecVec = std::vector<Vec>;

  std::vector<VecVec> array;
  for (size_t i = 0; i < N; ++i) {
    VecVec innerArray;
    for (size_t j = 0; j < M; ++j) {
      innerArray.push_back(Vec{});
    }
    array.push_back(innerArray);
  }
  return array;
}

/**
 * The ArgParser::getOptionVal<std::vector<double>>() does not return the values in any
 * guaranteed order, so sert them ascending.
 */
std::vector<double> getBinning(const ArgParser& p, const std::string& k)
{
  auto binning = p.getOptionVal<std::vector<double> >(k);
  std::sort(binning.begin(), binning.end());

  return binning;
}

// TODO: split up this monolithic block into a collection and a calculation part
#ifndef __CINT__
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto infile = parser.getOptionVal<std::string>("--input");
  const auto outfile = parser.getOptionVal<std::string>("--output");
  const auto jsonfile = parser.getOptionVal<std::string>("--jsonoutput");
  const auto ptBinning =  getBinning(parser, "--ptBinning");
  const auto rapBinning = getBinning(parser, "--rapBinning");

  TFile* fin = checkOpenFile(infile);
  TTree* tin = checkGetFromFile<TTree>(fin, "selectedData"); // treename hardcoded at the moment
  TLorentzVector* jpsi = nullptr;
  TLorentzVector* B = nullptr;
  tin->SetBranchAddress("JpsiP", &jpsi);
  tin->SetBranchAddress("BplusP", &B);

  const auto nRapBins = rapBinning.size() - 1;
  const auto nPtBins = ptBinning.size() - 1;

  auto cosTh2 =  createArray(nRapBins, nPtBins) ; // cosTheta^2
  auto sinTh2Cos2Phi = createArray(nRapBins, nPtBins) ; // sinTheta^2 * cos(2 Phi)
  auto sin2ThCosPhi = createArray(nRapBins, nPtBins) ; // sin(2 Theta) * cosPhi
  auto pT = createArray(nRapBins, nPtBins) ;

  unsigned overflowEvts{}; // evts that did not fit into the binning
  const auto nEntries = tin->GetEntries();
  const auto startTime = ProgressClock::now();
  for (auto i = 0; i < nEntries; ++i) {
    checkGetEntry(tin, i);
    const int ptBin = getBin(jpsi->Pt(), ptBinning);
    const int rapBin = getBin(std::abs(jpsi->Rapidity()), rapBinning);

    if (ptBin >= 0 && rapBin >= 0) {
      const auto cosThetaPhi = calcCosThetaPhiInBFrame(B, jpsi);
      const double costh2 = cosThetaPhi.first * cosThetaPhi.first;
      cosTh2[rapBin][ptBin].push_back(costh2);

      const double sinth2 = 1 - costh2; // sin^2 + cos^2 = 1
      sinTh2Cos2Phi[rapBin][ptBin].push_back(sinth2 * std::cos(cosThetaPhi.second * 2));

      const double sin2th = 2 * std::sqrt(sinth2) * cosThetaPhi.first; // sin(2x) = 2*sin(x)*cos(x)
      sin2ThCosPhi[rapBin][ptBin].push_back(sin2th * std::cos(cosThetaPhi.second));

      pT[rapBin][ptBin].push_back(jpsi->Pt());
    } else {
      overflowEvts++;
    }

    printProgress(i, nEntries, startTime, 5);
  }
  fin->Close();

  std::cout << "overflowEvts = " << overflowEvts << std::endl;

  std::vector<Lambdas> lamVec;
  TFile* fout = new TFile(outfile.c_str(), "recreate");
  fout->cd();
  // calculate lambdas in each bin and store in TGraphAsymmErrors (one per rap bin and parameter)
  for (size_t iRap = 0; iRap < nRapBins; ++iRap) {
    auto* lth = new TGraphAsymmErrors(nPtBins);
    auto* lph = new TGraphAsymmErrors(nPtBins);
    auto* ltp = new TGraphAsymmErrors(nPtBins);

    for (size_t iPt = 0; iPt < nPtBins; ++iPt) {
      auto lambdas = calcLambdasFromData(cosTh2[iRap][iPt],
                                         sinTh2Cos2Phi[iRap][iPt],
                                         sin2ThCosPhi[iRap][iPt]);
      lambdas.iPt = iPt + 1; lambdas.iRap = iRap + 1;
      lambdas.meanPt = mean(pT[iRap][iPt]);
      lamVec.push_back(lambdas);

      const double ptLow = lambdas.meanPt - getBinMin(iPt + 1, ptBinning);
      const double ptHigh = getBinMax(iPt + 1, ptBinning) - lambdas.meanPt;
      setPoint(lth, iPt, lambdas.meanPt, lambdas.lth, ptLow, ptHigh, lambdas.lthErr, lambdas.lthErr);
      setPoint(lph, iPt, lambdas.meanPt, lambdas.lph, ptLow, ptHigh, lambdas.lphErr, lambdas.lphErr);
      setPoint(ltp, iPt, lambdas.meanPt, lambdas.ltp, ptLow, ptHigh, lambdas.ltpErr, lambdas.ltpErr);
    }

    fout->cd();
    std::stringstream rapStr;
    rapStr << "_rap" << iRap + 1;
    lth->SetName(("lth_obs_data" + rapStr.str()).c_str());
    lth->Write();

    lph->SetName(("lph_obs_data" + rapStr.str()).c_str());
    lph->Write();

    ltp->SetName(("ltp_obs_data" + rapStr.str()).c_str());
    ltp->Write();
  }

  // store lambdas in json file so that they can be run through the python script
  storeLambdasInJSON(lamVec, jsonfile);

  // store raw data to file as well.
  std::cout << "Storing raw data to file" << std::endl;
  TTree* t = new TTree("rawdata", "raw data for calculating the lambda values");
  for (size_t iRap = 0; iRap < cosTh2.size(); ++iRap) {
    for (size_t iPt = 0; iPt < cosTh2[iRap].size(); ++iPt) {
      const auto binStr = getBinString(iRap + 1, iPt + 1);
      t->Branch(("cosTh2" + binStr).c_str(), &cosTh2[iRap][iPt]);
      t->Branch(("sinTh2cos2Ph" + binStr).c_str(), &sinTh2Cos2Phi[iRap][iPt]);
      t->Branch(("sin2ThcosPh" + binStr).c_str(), &sin2ThCosPhi[iRap][iPt]);
    }
  }
  t->Fill();

  fout->Write();
  fout->Close();

  return 0;
}
#endif
