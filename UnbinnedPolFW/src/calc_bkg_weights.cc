#include "Region.h"
#include "BkgWeightCalcChic.h"

#include "general/ArgParser.h"
#include "general/root_utils.h"
#include "general/misc_utils.h"

#include "RooWorkspace.h"
#include "TFile.h"
#include "TGraphErrors.h"

#include <string>
#include <vector>
#include <unordered_map>
#include <iostream>
#include <regex>

using WeightMap = std::unordered_map<std::string, BkgWeights2D>;

std::pair<TGraphErrors, TGraphErrors>
createGraphs(const WeightMap &weights, const std::vector<double> &binning)
{
  if (weights.size() > binning.size() - 1) {
    std::cerr << "I have more weights than bins!\n";
  }
  if (binning.size() - 1 > weights.size()) {
    std::cerr << "I have more bins than weights and will ignore tha last bins!\n";
  }

  constexpr auto rgxstr = ".*_pt([0-9]+)(_.*)?";
  std::regex rgx(rgxstr);
  std::smatch sm;

  TGraphErrors graph1(weights.size());
  TGraphErrors graph2(weights.size());

  for (const auto &binW : weights) {
    if (std::regex_match(binW.first, sm, rgx)) {
      const int index = std::stoi(sm[1].str());
      const auto min = binning[index - 1];
      const auto max = binning[index];
      const auto mean = (max + min) * 0.5;

      std::cout << binW.first << " " << index - 1 << " "  << mean << " "
                << binW.second.wChic1 << " " << binW.second.wChic2 << " " << min << " "
                << max << "\n";

      graph1.SetPoint(index - 1, mean, binW.second.wChic1);
      graph1.SetPointError(index - 1, mean - min, 0); // TODO; error on weights?
      graph2.SetPoint(index - 1, mean, binW.second.wChic2);
      graph2.SetPointError(index - 1, mean - min, 0); // TODO; error on weights?

    }
  }

  graph1.SetName("wChic1_v_pt");
  graph2.SetName("wChic2_v_pt");

  return {graph1, graph2};
}

#if !(defined(__CINT__) or defined(__CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto fitFiles = parser.getOptionVal<std::vector<std::string>>("--files");
  const auto pTBinning = parser.getOptionVal<std::vector<double>, ValueOrdering::Ascending>("--ptbinning");
  const auto outputFile = parser.getOptionVal<std::string>("--outfile", "weight_graphs.root");

  WeightMap weights;

  for (const auto &fileN : fitFiles) {
    std::cout << "Processing file " << fileN << "\n";

    TFile *f = checkOpenFile(fileN);
    auto *ws = checkGetFromFile<RooWorkspace>(f, "ws_masslifetime");
    const auto binname = getBinFromFile(fileN);
    const auto snapname = "snapshot_" + binname;

    const auto massRegions = calcMassRegions(ws, snapname);
    Region<Boundary::TwoSided> fullMR(massRegions.SR1.min(), massRegions.SR2.max());
    const auto ltRegions = calcLifetimeRegions(ws, fullMR, "data_" + binname + "_SR");

    const auto w = calculate2DWeights(ws, massRegions, ltRegions, snapname);

    if (!weights.insert({binname, w}).second) {
      std::cerr << "Could not insert weights for bin " << binname << "\n";
    }

    f->Close();
  }

  for (const auto &kv : weights) {
    std::cout << kv.first << " -> " << kv.second.wChic1 << " " << kv.second.wChic2 << "\n";
  }

  TFile *of = new TFile(outputFile.c_str(), "recreate");
  auto graphs = createGraphs(weights, pTBinning);

  of->cd();
  graphs.first.Write();
  graphs.second.Write();


  of->Write();
  of->Close();

  return 0;
}

#endif
