#include "TTreeLooper.h"
#include "JpsiFromBInputEvent.h"
#include "JpsiFromBEvent.h"
#include "JpsiFromBPreselection.h"

#include "general/ArgParser.h"
#include "general/root_utils.h"

#include "TFile.h"
#include "TTree.h"
#include "TChain.h"
#include "TH1D.h"

#include <vector>
#include <string>
#include <functional>
#include <iostream>

#ifndef __CINT__
int main(int argc, char* argv[])
{
  const ArgParser inArgs(argc, argv);

  const auto outfilename = inArgs.getOptionVal<std::string>("--outputfile");
  const auto inputfileNames = inArgs.getOptionVal<std::vector<std::string> >("--inputfiles");

  // TChain* inChain = new TChain("tree_jpsi");
  // for (const auto& name : inputfileNames) {
  //   inChain->Add(name.c_str());
  // }
  // TTree* tin = inChain;
  TTree* tin = createTChain(inputfileNames, "tree_jpsi");

  TFile* fout = new TFile(outfilename.c_str(), "recreate");
  TTree* tout = new TTree("selectedData", "selected events");
  tout->SetDirectory(fout); // just to make sure this does not get a memory resident TTree

  TH1D* Reco_StatEv = new TH1D("Reco_StatEv", "", 12, 0.0, 12.0);

  TTreeLooper<JpsiFromBInputEvent, JpsiFromBEvent> treeLooper(tin, tout);

  using namespace std::placeholders;
  treeLooper.loop(std::bind(jpsiFromBPreSelection, _1, _2, Reco_StatEv), -1);

  fout->cd();
  Reco_StatEv->Write();

  fout->Write();
  fout->Close();

  return 0;
}
#endif
