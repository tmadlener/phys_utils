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

// stl
#include <map>
#include <utility>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>

/** store anything with (run, lumi) as key. */
template<class T> using RunLumiMap = std::map<std::pair<int, int>, T>;

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
RunLumiMap<int> getHLTPSMap(const std::string& hltpsFileName)
{
  RunLumiMap<int> hltpsMap;

  // std::cout << hltpsFileName << std::endl;

  std::ifstream hltpsFile(hltpsFileName.c_str());
  std::string line = "";
  while (std::getline(hltpsFile, line)) {
    if (!startsWith(line, "#")) {
      const auto fields = splitString(line, '\t');
      const int run = std::stoi(fields[0]);
      const int lumiStart = std::stoi(fields[1]);
      const int lumiEnd = std::stoi(fields[2]);
      const int HLTPS = std::stoi(fields[3]); // splitString returns no empty tokens
      for (int i = lumiStart; i <= lumiEnd; ++i){
        hltpsMap.insert({{run, i}, HLTPS});
      }
    }
  }

  hltpsFile.close();
  return hltpsMap;
}

/** helper struct */
struct Rate {
  double val;
  double err;
};

std::ostream& operator<<(std::ostream& os, const Rate& r)
{
  os << r.val << " +/- " << r.err;
  return os;
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

template<typename T>
std::map<int, T> getCounterMap(const size_t N)
{
  std::map<int, T> map;
  map.insert(std::make_pair(-1, T{})); // "overflow bin" for events that cannot be classified into the binning
  for (size_t i = 0; i < N; ++i) { // "normal bins"
    map.insert(std::make_pair(i, T{}));
  }

  return map;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::map<int, T>& m)
{
  for (const auto & e : m) {
    std::cout << e.first << ": " << e.second << std::endl;
  }
  return os;
}


/**
 * calc rate from data in file
 * TODO: at the moment this gives very small rates in the HLTPhysics dataset, have to look into it, to find out
 * what is happening.
 * TODO: make alessios macro work with my data
 */
template<typename T>
std::vector<Rate> calcRateFromFile(TFile* f, const std::string& path,
                                   const std::vector<T>& binning, const RunLumiMap<T>& binMap,
                                   const RunLumiMap<int>& hltpsMap/*, const RunLumiMap<T>& psIdxMap*/)
{
  TTree* t = getFromFile<TTree>(f, "HltTree");
  TriggerInfo trigInfo;
  trigInfo.Init(t, path);

  auto psCounts = getCounterMap<std::vector<int>>(binning.size()); // if trigger path passes, record its prescale (per each bin)
  auto lumiSecs = getCounterMap<unsigned>(binning.size()); // count the number of lumi sections in each bin

  for (const auto& rl : binMap) { // all lumi sections appear only once in this map!
    const int binIdx = getBin(rl.second, binning);
    lumiSecs[binIdx]++;
  }

  // TH1D* lumiSecs = new TH1D("lumiSecs", "lumi Sections", binning.size() - 1, binning.data());
  // TH1D* counts = new TH1D("counts", "trigger counts", binning.size() - 1, binning.data());

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
      const int binIdx = getBin(binIt->second, binning);
      // // lumiSecs[binIdx]++;
      // lumiSecs[binIdx] = 1; // do not double count lumi sections

      if (trigInfo.trig) {
        psCounts[binIdx].push_back(hltpsIt->second);
      }
    } else {
      std::cerr << "Couldn't find the Info needed for binning in the binMap for "
                << runLumi.first << ", " << runLumi.second << " (run, lumi)" << std::endl;
    }
    // printProgress(i, nEntries, startTime, 4);
  }

  std::cout << lumiSecs << std::endl;
  std::cout << psCounts << std::endl;

  // calculate the rate in each bin
  std::vector<Rate> rates;
  for (int i = -1; i < (int)binning.size() - 1; ++i) { // in order to have the results correpsond to the passed binning
    const unsigned nLumis = lumiSecs[i];
    const auto& psVec = psCounts[i]; // vector of prescales
    const size_t nCounts = psVec.size();
    const int psSum = std::accumulate(psVec.cbegin(), psVec.cend(), 0); // sum up all prescale values
    const double rate = nLumis == 0 ? 0 : psSum / (nLumis * 23.31); // avoid division by zero
    const double rateErr = nCounts == 0 ? 0 : rate / std::sqrt(nCounts);

    // std::cout << "i = " << i << ", nCounts = " << nCounts << ", psSum = " << psSum << ", nLumis = " << nLumis << std::endl;
    // std::cout << rate << ", " << rateErr << std::endl;
    rates.push_back({rate, rateErr});
  }

  return rates;
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

  std::cout << "reading HLTPS map" << std::endl;
  auto hltpsMap = getHLTPSMap("HLTPS.tsv");
  // std::cout << hltpsMap << std::endl;

  std::cout << "reading PU and inst lumi map" << std::endl;
  auto puLumiMaps = getPuInstLumiMaps("PUTests.csv");
  // std::cout << puLumiMaps.first << std::endl;
  // std::cout << puLumiMaps.second << std::endl;

  const std::string file = argv[1];
  TFile* f = TFile::Open(file.c_str());

  const std::string path = argv[2];
  auto puRates = calcRateFromFile(f, path, puBinning, puLumiMaps.first, hltpsMap);
  std::cout << "puRates = " << puRates << std::endl;

  auto luRates = calcRateFromFile(f, path, lumiBinning, puLumiMaps.second, hltpsMap);
  std::cout << "luRates = " << luRates << std::endl;

  return 0;
}
#endif
