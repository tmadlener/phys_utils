#include "Region.h"
#include "BkgWeightCalcChic.h"
#include "ChicInputEvent.h"
#include "ChicTupleEvent.h"
#include "ChicTuplingWithWeights.h"

#include "general/root_utils.h"
#include "../PolUtils/interface/calcAngles.h"
#include "../PolUtils/interface/TTreeLooper.h"

#include "general/ArgParser.h"

#include "RooWorkspace.h"

#include <string>
#include <iostream>

#include <limits>

#if !(defined(__CINT__) or defined(__CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto fitFileName = parser.getOptionVal<std::string>("--fitFile");
  const auto dataFileName = parser.getOptionVal<std::string>("--dataFile");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile", "chic_tuple.root");

  TFile* fitFile = checkOpenFile(fitFileName);
  auto* ws = checkGetFromFile<RooWorkspace>(fitFile, "ws_masslifetime");
  // ws->Print("v");

  const auto massRegions = calcMassRegions(ws);
  const auto fullChicMSR = Region<Boundary::TwoSided>(massRegions.SR1.min(), massRegions.SR2.max());
  const auto lifetimeRegions = calcLifetimeRegions(ws, fullChicMSR);
  const auto weights = calculateRegionWeights(ws, massRegions, lifetimeRegions);

  fitFile->Close();

  TFile* dataFile = checkOpenFile(dataFileName);
  TTree* dataTree = checkGetFromFile<TTree>(dataFile, "selectedData");

  TFile* outFile = new TFile(outFileName.c_str(), "recreate");
  TTree* tupleTree = new TTree("chic_tuple", "tupled data chic events with applied weights");
  tupleTree->SetDirectory(outFile);

  TTreeLooper<ChicInputEvent, ChicTupleEvent> treeLooper(dataTree, tupleTree);

  // bind the regions and weights via a lambda to confirm to the interface requirements
  // of the TTreeLooper::loop function
  auto tuplingFunc = [&massRegions, &lifetimeRegions, &weights]
    (const ChicInputEvent &ie, ChicTupleEvent &e) -> bool
    {
      return chicTuplingWithWeights(ie, e, massRegions, lifetimeRegions, weights);
    };
  // treeLooper.loop<decltype(tuplingFunc), PrintStyle::None>(tuplingFunc, 1000);
  treeLooper.loop(tuplingFunc);

  outFile->Write();
  outFile->Close();

  return 0;
}


#endif
