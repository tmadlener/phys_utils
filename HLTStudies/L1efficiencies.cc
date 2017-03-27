#include <vector>
#include <iostream>
#include <cmath>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <array>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"
// #include "TGraphErrors.h"
#include "TROOT.h"


#include "ntupleTree.h"
#include "general/ArgParser.h"
#include "HLTStudies/L1Seeds.h"

// COMPILE
// g++ $(root-config --libs --cflags) -Wall -Wextra -O2 efficiencies.cc -o eff -I$HOME/phys_utils/

/** match muon and l1 muon via deltaR (and also pT if necessary).
 * Return the index of the muon in the l1Muons vector (or -1).
 */
int l1MuonMatch(const double muEta, const double muPhi, const double muPt,
                const std::vector<L1MuonCand>& l1Muons,
                const double dRcut, const double dPtCut)
{
  int i = 0;
  for (const auto& l1Muon : l1Muons) {
    // const double dEta = muEta - l1Muon.etaAtVtx;
    // const double dPhi = deltaPhi(muPhi, l1Muon.phiAtVtx);
    // const double deltaR = std::sqrt(dEta*dEta + dPhi*dPhi);
    const double dR = deltaR(muEta, l1Muon.etaAtVtx, muPhi, l1Muon.phiAtVtx);

    const double dPt = std::abs(muPt - l1Muon.pt) / l1Muon.pt;

    if (dR < dRcut /*&& dPt < dPtCut*/) return i;
    i++;
  }

  return -1;
}

/**
 * check if the muons of the dimuon can be matched to L1 muons and return their indices (or -1)
 */
std::pair<int, int> l1Match(const MuMuCand& dimu, const std::vector<L1MuonCand>& l1Muons,
                            const double dR, const double dPt)
{
  return {l1MuonMatch(dimu.Mu1Eta, dimu.Mu1Phi, dimu.Mu1Pt, l1Muons, dR, dPt),
      l1MuonMatch(dimu.Mu2Eta, dimu.Mu2Phi, dimu.Mu2Pt, l1Muons, dR, dPt)};
}

/** fiducial cuts. */
bool singleMuonCut(const double pt, const double absEta)
{
  // TODO: 3.5 and 3.0 for eta < 1.4 / > 1.4
  // return true;
  return (pt > 3.5 && absEta < 1.4) || (pt > 3.0 && absEta > 1.4);

  // if (pt > 4.5 && absEta < 1.2) return true;
  // if (pt > 3.5 && inRange(absEta, 1.2, 1.4)) return true;
  // if (pt > 3.0 && inRange(absEta, 1.4, 1.6)) return true;

  // return false;
}

/** preselection for one dimuon candidate. */
bool preselection(const MuMuCand& dimu)
{
  const double massMin = 2.9;
  const double massMax = 3.3;
  const double pTMin = 10;
  const double rapMax = 1.2;
  const double vtxProb = 0.01;

  if (singleMuonCut(dimu.Mu1Pt, dimu.Mu1Eta) && singleMuonCut(dimu.Mu2Pt, dimu.Mu2Eta) &&
      inRange(dimu.MuMuMass, massMin, massMax) && dimu.JpsiPt > pTMin &&
      std::abs(dimu.JpsiRap) <= rapMax && dimu.MuMuCL > vtxProb) return true;

  return false;
}

/** check if any of the dimuon candidates in the event satisfy the preselection and return its index (or -1) */
std::vector<int> offlineCuts(const std::vector<MuMuCand>& dimuons)
{
  std::vector<int> idcs;
  int i = 0;
  for (const auto& dimu : dimuons) {
    if (preselection(dimu)) idcs.push_back(i);
    i++;
  }

  return idcs;
}

////////////////////////////////////////////////////////////////////////////////

// // HISTOGRAMS FOR CALCULATING L1 EFFICIENCIES
// using HistMap = std::unordered_map<std::string, std::pair<TH1D*, TH1D*>>;
// HistMap
// createHistMap(const L1SeedMap& l1Seeds, const int nBins, const double min, const double max,
//               const std::string& suffix)
// {
//   HistMap hists;
//   for (const auto& seed : l1Seeds) {
//     hists.insert({seed.first,
//           {   new TH1D((seed.first + suffix + "_num").c_str(), seed.first.c_str(), nBins, min, max),
//               new TH1D((seed.first + suffix + "_denom").c_str(), seed.first.c_str(), nBins, min, max)}
//       });
//   }

//   return hists;
// }

using VariableMap = std::unordered_map<std::string, std::function<double(const MuMuCand&)>>;
VariableMap createVariableMap()
{
  VariableMap varMap;
  varMap["JpsiPt"] = [](const MuMuCand& m) { return m.JpsiPt; };
  varMap["Mu1Pt"] = [](const MuMuCand& m) { return m.Mu1Pt; };
  varMap["Mu2Pt"] = [](const MuMuCand& m) { return m.Mu2Pt; };
  // varMap["JpsiRap"] = [](const MuMuCand& m) { return m.JpsiRap; };
  // varMap["JpsiEta"] = [](const MuMuCand& m) { return m.JpsiEta; };
  // varMap["JpsiPhi"] = [](const MuMuCand& m) { return m.JpsiPhi; };
  // varMap["Mu1Phi"] = [](const MuMuCand& m) { return m.Mu1Phi; };
  // varMap["Mu2Phi"] = [](const MuMuCand& m) { return m.Mu2Phi; };
  // varMap["Mu1Eta"] = [](const MuMuCand& m) { return m.Mu1Eta; };
  // varMap["Mu2Eta"] = [](const MuMuCand& m) { return m.Mu2Eta; };
  // varMap["cosThHX"] = [](const MuMuCand& m) { return m.cosThHX; };
  // varMap["phiHX"] = [](const MuMuCand& m) { return m.phiHX; };

  return varMap;
}

using ValueMap = std::map<std::pair<std::string, std::string>,
                          std::pair<std::vector<double>, std::vector<double>>>;

/** create a map to store the numerator and denominator values. */
ValueMap createValueMap(const L1SeedMap& l1Seeds, const VariableMap& vars)
{
  ValueMap valMap;
  for (const auto& seed : l1Seeds) {
    for (const auto& var : vars) {
      valMap.insert({{seed.first, var.first}, {std::vector<double>(), std::vector<double>()}});
    }
  }
  return valMap;
}

/** fill the values into the value map. */
void fillValues(ValueMap& vals, const VariableMap& vars, const MuMuCand& dimuon,
                const std::unordered_map<std::string, bool>& l1ResultsMap)
{
  for (const auto& var : vars) {
    const double varVal = var.second(dimuon);
    for (const auto& seedResult : l1ResultsMap) {
      vals[{seedResult.first, var.first}].first.push_back(varVal);
      if (seedResult.second) {
        vals[{seedResult.first, var.first}].second.push_back(varVal);
      }
    }
  }
}

/** write the results to the root file (raw). */
void writeRawToFile(TFile* f, const std::string& treename, /*const*/ ValueMap& valMap)
{
  f->cd();
  TTree* t = new TTree(treename.c_str(), "raw values");

  for (auto& val : valMap) {
    const std::string branchName = val.first.first + "_" + val.first.second;
    t->Branch((branchName + "_denom").c_str(), &val.second.first);
    t->Branch((branchName + "_num").c_str(), &val.second.second);
  }

  t->Fill();
  t->Write();
}

/** calculate  the error of the division (of n by d). */
double calcErrDivision(const double n, const double d)
{
  const double nr = std::sqrt(n)/n;
  const double dr = std::sqrt(d)/d;
  const double r = n/d;

  return r * std::sqrt(nr*nr + dr*dr);
}

void printIntegratedEffs(const ValueMap& valMap)
{
  const std::string varName = "JpsiPt";
  for (const auto& val : valMap) {
    const double num = val.second.second.size();
    const double denom = val.second.first.size();

    const double ratio = num / denom;
    const double err = calcErrDivision(num, denom);

    std::cout << val.first.first << ": " << ratio << " +/- " << err << '\n';
  }
}

struct Binning {
  int bins;
  double min;
  double max;
};

using BinningMap = std::unordered_map<std::string, Binning>;

BinningMap createBinningMap()
{
  BinningMap binMap;
  binMap["JpsiPt"] = Binning{25, 20, 50};
  binMap["Mu1Pt"] = Binning{25, 0, 30};
  binMap["Mu2Pt"] = Binning{25, 0, 30};
  // binMap["JpsiRap"] = Binning{25, -1.205, 1.205};
  // binMap["JpsiEta"] = Binning{25, -2.4, 2.4};
  // binMap["JpsiPhi"] = Binning{25, -3.15, 3.15};
  // binMap["Mu1Phi"] = Binning{25, -3.15, 3.15};
  // binMap["Mu2Phi"] = Binning{25, -3.15, 3.15};
  // binMap["Mu1Eta"] = Binning{25, -2.4, 2.4};
  // binMap["Mu2Eta"] = Binning{25, -2.4, 2.4};
  // binMap["cosThHX"] = Binning{25, -1.0, 1.0};
  // binMap["phiHX"] = Binning{25, -180, 180};

  return binMap;
}


TH1D* createHist(const std::vector<double>& vals,
                 const Binning& binning, const std::string& name)
{
  auto* h = new TH1D(name.c_str(), "", binning.bins, binning.min, binning.max);
  h->Sumw2(true);
  for (const auto v : vals) h->Fill(v);
  return h;
}

TH1D* calcEff(const std::vector<double>& numVals, const std::vector<double>& denomVals,
              const Binning& binning, const std::string& name)
{
  auto* numHist = createHist(numVals, binning, name + "_num");
  auto* denomHist = createHist(denomVals, binning, name + "_denom");
  numHist->Write();
  denomHist->Write();

  auto* ratio = static_cast<TH1D*>(numHist->Clone(name.c_str()));

  ratio->Divide(denomHist);
  ratio->Write();
  return ratio;
}

void plotEff(const ValueMap::value_type& variable, const BinningMap& binningMap)
{
  const std::string varName = variable.first.second;
  const std::string plotName = variable.first.first + "_" + varName;

  auto it = binningMap.find(varName);
  auto* eff = calcEff(variable.second.second, variable.second.first, it->second, plotName);

  auto* can = new TCanvas("c", "c", 1000, 1000);
  can->cd();
  eff->Draw("PLE");

  const std::string filename = "plots/" + plotName + ".pdf";
  can->SaveAs(filename.c_str());
}

void makePlots(const ValueMap& valMap, const BinningMap& binningMap)
{
  for (const auto& val : valMap) plotEff(val, binningMap);
}

const double dRcut = 0.3;
const double dPtCut = 0.4;

#ifndef __CINT__
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto fn = parser.getOptionVal<std::string>("--file");
  const auto ofn = parser.getOptionVal<std::string>("--outfile", "efficiencies.root");

  TFile* f = TFile::Open(fn.c_str());
  TTree* t = static_cast<TTree*>(f->Get("theNtuples/ntupleTree"));
  ntupleEvent* event = nullptr;
  t->SetBranchAddress("event", &event);

  const auto l1SeedMap = createL1SeedMap();
  const auto varMap = createVariableMap();
  ValueMap valMap = createValueMap(l1SeedMap, varMap);

  // keeping this map here, so that it doesn't get created/destroyed every event
  std::unordered_map<std::string, bool> l1ResultMap;

  unsigned multJpsiInEvent{};
  unsigned passedOffline{};

  for (int i = 0; i < t->GetEntries(); ++i) {
    t->GetEntry(i);

    // only check L1 seed (and match, etc.) if the event passes the offline selection
    const auto dimuIdcs = offlineCuts(event->mumucands);
    if (dimuIdcs.empty()) continue;
    passedOffline++;

    if (dimuIdcs.size() > 1) multJpsiInEvent++;

    // for each possible dimuon candidate check if the muons match the L1 muons and if so,
    // also if these satisfy the different L1 seeds and store the results in a results map
    for (const auto seed : l1SeedMap) { l1ResultMap[seed.first] = false; } // reset map
    for (const int idx : dimuIdcs) {
      const auto l1MuIdcs = l1Match(event->mumucands[idx], event->L1muons, dRcut, dPtCut);
      if ((l1MuIdcs.first < 0 && l1MuIdcs.second < 0) ||
          (l1MuIdcs.first == l1MuIdcs.second)) continue; // no match or both have the same l1 muon
      for (const auto& seed : l1SeedMap) {

        if (seed.second(event->L1muons[l1MuIdcs.first],
                        event->L1muons[l1MuIdcs.second])) {
          l1ResultMap[seed.first] = true;
        }
      }
    }

    // WARNING: only taking the varibles of the first dimuon candidate here (even if there are possibly more in the event)
    fillValues(valMap, varMap, event->mumucands[dimuIdcs[0]], l1ResultMap);
  }
  f->Close();

  std::cout << passedOffline << " events passed the offline selection, " << multJpsiInEvent
            << " of them had more then one dimuon candidate. f = " << (double)multJpsiInEvent/passedOffline << std::endl;

  TFile* of = new TFile(ofn.c_str(), "recreate");
  writeRawToFile(of, "raw_vals", valMap);

  printIntegratedEffs(valMap);

  gROOT->ProcessLine("gErrorIgnoreLevel = 1001");
  const auto binningMap = createBinningMap();
  of->cd();
  makePlots(valMap, binningMap);

  of->Close();

  return 0;
}

#endif
