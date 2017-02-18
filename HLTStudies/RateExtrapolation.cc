#include "general/string_helper.h"
#include "general/root_utils.h"
#include "general/misc_utils.h"
#include "general/vector_helper.h"
#include "general/progress.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TFitResultPtr.h"
#include "TFitResult.h"
#include "TF1.h"

// stl
#include <map>
#include <utility>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>


using RunLumi = std::pair<int, int>;

/** store anything with (run, lumi) as key. */
template<class T> using RunLumiMap = std::map<RunLumi, T>;

template<typename T>
std::ostream& operator<<(std::ostream& os, const RunLumiMap<T>& map)
{
  for (const auto& el : map) {
    os << el.first.first << ", " << el.first.second << ": " << el.second << std::endl;
  }
  return os;
}


/**
 * Get the pile-up and the inst. lumi map from the puFile
 * puFile is a .csv file obtained from brilcalc with the --byls option
 */
std::pair<RunLumiMap<double>, RunLumiMap<double>>
getPuInstLumiMaps(const std::string& puFileName)
{
  RunLumiMap<double> puMap;
  RunLumiMap<double> lumiMap;

  std::ifstream puFile(puFileName.c_str());
  if(!puFile) std::cerr << "Can't open file " << puFileName << std::endl;
  std::string line;
  while(std::getline(puFile, line)) {
    if (!startsWith(line, "#")) { // assume lines starting with # are comments
      const auto fields = splitString(line, ',');
      const int run = std::stoi(splitString(fields[0], ':')[0]);
      const int lumi = std::stoi(splitString(fields[1], ':')[0]);
      const auto runLumi = std::make_pair(run, lumi);
      const double PU = std::stof(fields[7]);
      // length of lumi section is 23.31 secs, data is in ub^-1 -> convert to cm^-2 s^-1
      // using delievered lumi here. Use recorded instead?
      const double instLumi = std::stof(fields[5]) * 1e30 / 23.31;
      if (PU > 0) {
        auto insertIt = puMap.insert({runLumi, PU});
        if (!insertIt.second) {
          std::cerr << "(run, lumi) - (" << run << "," << lumi << ") already in map with PU "
                    << insertIt.first->second << std::endl;
        }
      }
      if (instLumi > 0) {
        auto insertIt = lumiMap.insert({runLumi, instLumi});
        if (!insertIt.second) {
          std::cerr << "(run, lumi) - (" << run << "," << lumi << ") already in map with inst. lumi. "
                    << insertIt.first->second << std::endl;
        }
      }
    }
  }

  // puFile.close();
  return {puMap, lumiMap};
}

/** read the HLTPS values form the corresponding .tsv file into a map. */
RunLumiMap<int> getHLTPSMap(const std::string& hltpsFileName, const std::string& option = "HLTPS")
{
  RunLumiMap<int> hltpsMap;

  /*const*/ std::map<std::string, int> optMap = {
    {"PScolumn", 4}, {"HLTPS", 3}
  };

  // std::cout << hltpsFileName << std::endl;

  std::ifstream hltpsFile(hltpsFileName.c_str());
  std::string line = "";
  if (!hltpsFile) std::cerr << "Can't open file " << hltpsFileName << std::endl;
  while (std::getline(hltpsFile, line)) {
    if (!startsWith(line, "#")) {
      const auto fields = splitString(line, '\t');
      const int run = std::stoi(fields[0]);
      const int lumiStart = std::stoi(fields[1]);
      const int lumiEnd = std::stoi(fields[2]);
      const int PS = std::stoi(fields[optMap[option]]); // splitString returns no empty tokens
      for (int i = lumiStart; i <= lumiEnd; ++i){
        hltpsMap.insert({{run, i}, PS});
      }
    }
  }

  hltpsFile.close();
  return hltpsMap;
}

/** helper struct for getting info from the HLTBitAnalyser file. */
struct TriggerInfo {
  void Init(TTree* t, const std::string& paths)
  {
    t->SetBranchAddress("LumiBlock", &lumi);
    t->SetBranchAddress("Run", &run);
    t->SetBranchAddress(paths.c_str(), &trig);
  }

  int trig;
  int run;
  int lumi;
};


/**
 * calc rate from data in file
 * TODO: at the moment this gives very small rates in the HLTPhysics dataset, have to look into it, to find out
 * what is happening.
 * TODO: make alessios macro work with my data
 */
template<typename T>
TH1D calcRateFromFile(TFile* f, const std::string& path,
                      const std::vector<T>& binning, const RunLumiMap<T>& binMap,
                      const RunLumiMap<int>& hltpsMap/*, const RunLumiMap<T>& psIdxMap*/,
                      const std::string& binningName = "")
{
  TTree* t = getFromFile<TTree>(f, "HltTree");
  TriggerInfo trigInfo;
  trigInfo.Init(t, path);

  TH1D lumiSecs(("lumiSecs" + binningName).c_str(), "lumi sections", binning.size() - 1, binning.data());
  for (const auto& rl: binMap) {
    lumiSecs.Fill(rl.second);
  }

  TH1D counts(("counts" + binningName).c_str(), "counts", binning.size() - 1, binning.data());
  TH1D prescale(("PS" + binningName).c_str(), "pre scales", binning.size() - 1, binning.data());

  const int nEntries = t->GetEntries();
  // const auto startTime = ProgressClock::now();
  for (int i = 0; i < nEntries; ++i) {
    t->GetEntry(i);

    // check if we have the information needed for binning in the map and if we actually want the event (i.e. is it in
    // the runs and lumi sections we want to use?)
    const auto runLumi = std::make_pair(trigInfo.run, trigInfo.lumi);
    const auto binIt = binMap.find(runLumi);
    const auto hltpsIt = hltpsMap.find(runLumi);
    if (binIt != binMap.cend() && hltpsIt != hltpsMap.cend()) {
      if (trigInfo.trig) {
        counts.Fill(binIt->second);
        prescale.Fill(binIt->second, hltpsIt->second);
      }
    } else {
      std::cerr << "Couldn't find the Info needed for binning in the binMap for "
                << runLumi.first << ", " << runLumi.second << " (run, lumi)" << std::endl;
    }
    // printProgress(i, nEntries, startTime, 4);
  }

  // calculate the rate in each bin
  TH1D rates(("rates" + binningName).c_str(), "rates", binning.size() - 1, binning.data());
  for (int i = 0; i <= rates.GetNbinsX() + 1; ++i) { // include under- and overflow bin
    const unsigned nLumis = lumiSecs[i];
    const double nCounts = counts.GetBinContent(i);
    const double psSum = prescale.GetBinContent(i);
    const double rate = nLumis == 0 ? 0 : psSum / (nLumis * 23.31); // avoid division by zero
    const double rateErr = nCounts == 0 ? 0 : rate / std::sqrt(nCounts);

    rates.SetBinContent(i, rate);
    rates.SetBinError(i, rateErr);
  }

  lumiSecs.Write();
  counts.Write();
  prescale.Write();

  return rates;
}

TFitResultPtr fitRates(TH1D& rates, const double min, const double max)
{
  TF1* f = new TF1("fParab", "pol1", min, max);
  return rates.Fit(f, "SMR");
}

const std::vector<double> puBinning = {0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};
const std::vector<double> lumiBinning = {0, 0.2e34, 0.4e34, 0.6e34, 0.8e34, 1e34, 1.2e34, 1.4e34, 1.6e34, 1.8e34, 2e34};

#ifndef __CINT__
int main(int argc, char* argv[])
{
  if (argc < 3) {
    std::cerr << "Need root file and path" << std::endl;
    return 1;
  }

  const std::string preScaleFile = "/afs/hephy.at/user/t/tmadlener/phys_utils/HLTStudies/HLTPS.tsv";
  const std::string puFile = "/afs/hephy.at/user/t/tmadlener/phys_utils/HLTStudies/PUTests.csv";

  std::cout << "reading HLTPS map" << std::endl;
  auto hltpsMap = getHLTPSMap(preScaleFile, "HLTPS");
  // std::cout << hltpsMap << std::endl;

  auto psIdxMap = getHLTPSMap(preScaleFile, "PScolumn");

  std::cout << "reading PU and inst lumi map" << std::endl;
  auto puLumiMaps = getPuInstLumiMaps(puFile);
  // std::cout << puLumiMaps.first << std::endl;
  // std::cout << puLumiMaps.second << std::endl;

  const std::string file = argv[1];
  TFile* f = TFile::Open(file.c_str());

  const std::string path = argv[2];
  TFile* fout = new TFile((path + "_rates.root").c_str(), "recreate"); // open output file here to be able to write histos to it
  std::cout << "pileup rates" << std::endl;
  auto puRates = calcRateFromFile(f, path, puBinning, puLumiMaps.first, hltpsMap, "_PU");
  puRates.Write();


  std::cout << "inst lumi rates" << std::endl;
  auto luRates = calcRateFromFile(f, path, lumiBinning, puLumiMaps.second, hltpsMap, "_instLumi");
  luRates.Write();

  auto puFitRlt = fitRates(puRates, 10, 60);
  puFitRlt->SetName("pu_fit_results");
  puFitRlt->Write();

  auto luFitRlt = fitRates(luRates, 0.4e34, 1.4e34);
  luFitRlt->SetName("instlumi_fit_results");
  luFitRlt->Write();

  fout->cd();

  fout->Close();
  return 0;
}
#endif
