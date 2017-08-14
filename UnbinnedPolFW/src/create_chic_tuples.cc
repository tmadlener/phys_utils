#include "ChicMassFit.h"
#include "BkgWeightCalcChic.h"
#include "ChicInputEvent.h"
#include "ChicTupleEvent.h"
#include "ChicTuplingWithWeights.h"
#include "Region.h"

#include "../PolUtils/interface/TTreeLooper.h"

#include "general/ArgParser.h"
#include "general/root_utils.h"
#include "general/misc_utils.h"

#include <string>
#include <iostream>

void createTupleFile(const std::string &ifn, const std::string &ofn,
                     const MassRegions &mr, const Region<Boundary::TwoSided> &ltr,
                     const std::pair<double, double> &weights)
{
  TFile *dataFile = checkOpenFile(ifn);
  TTree *dataTree = checkGetFromFile<TTree>(dataFile, "selectedData");

  TFile *outFile = new TFile(ofn.c_str(), "recreate");
  TTree *tupleTree = new TTree("chic_tuple", "tupled chic data with 1 dim mass bkg weights");
  tupleTree->SetDirectory(outFile);

  TTreeLooper<ChicInputEvent, ChicTupleEvent> treeLooper(dataTree, tupleTree);

  // use lambda to bind additional parameters to confirm to interface requirements
  auto tuplingFunc = [&mr, &weights, &ltr] (const ChicInputEvent &ie, ChicTupleEvent &e) -> bool {
    return chicTuplingWithMassWeights(ie, e, mr, ltr, weights);
  };
  treeLooper.loop(tuplingFunc);

  outFile->Write();
  outFile->Close();

  dataFile->Close();
}

#if !(defined(__CINT__) or defined(__CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto fitFileName = parser.getOptionVal<std::string>("--fitfile");
  const auto dataFileName = parser.getOptionVal<std::string>("--datafile");
  const auto plotOutDir = parser.getOptionVal<std::string>("--plotdir", "");
  const bool noPlots = parser.getOptionVal<bool>("--noplots", false);
  const auto outFileBase = parser.getOptionVal<std::string>("--outbase", "chic_tuple");

  TFile *fitFile = checkOpenFile(fitFileName);
  auto *ws = checkGetFromFile<RooWorkspace>(fitFile, "ws_masslifetime");

  const auto binname = getBinFromFile(fitFileName);
  const auto snapPR = "snap_" + binname + "_PR";
  const auto snapNP = "snap_" + binname + "_NP";

  auto jpsiLTRegions = calcLifeTimeRegionsJpsi(ws, "data_" + binname + "_SR");

  if (!noPlots) {
    makePlot(ws, "M_fullModel", snapPR, "data_" + binname + "_SR_PR");
    makePlot(ws, "M_fullModel", snapNP, "data_" + binname + "_SR_NP");
  }

  auto mrPR = calcMassRegions(ws, snapPR, "PR");
  auto mrNP = calcMassRegions(ws, snapNP, "NP");

  auto wPR = calcMassWeights(ws, mrPR, "M_background", "chicMass", snapPR);
  auto wNP = calcMassWeights(ws, mrNP, "M_background", "chicMass", snapNP);

  // std::cout << wPR.first << " " << wPR.second << "\n";
  // std::cout << wNP.first << " " << wNP.second << "\n";

  fitFile->Close();

  createTupleFile(dataFileName, outFileBase + "_PR.root", mrPR, jpsiLTRegions.PR, wPR);
  createTupleFile(dataFileName, outFileBase + "_NP.root", mrNP, jpsiLTRegions.NP, wNP);

  return 0;
}
#endif
