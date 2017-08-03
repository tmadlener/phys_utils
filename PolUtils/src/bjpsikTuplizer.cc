#include "TTreeLooper.h"
#include "JpsiFromBInputEvent.h"
#include "BRootupleEvent.h"
#include "JpsiFromBRootupling.h"

#include "general/ArgParser.h"
#include "general/root_utils.h"

#include "TChain.h"
#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <string>

#if !(defined(__CINT__) or defined( __CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto inputFiles = parser.getOptionVal<std::vector<std::string>>("--inputfiles");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile");
  const int maxEvents = parser.getOptionVal<int>("--nevents", -1);

  TTree* tin = createTChain(inputFiles, "tree_jpsi");

  TFile* fout = new TFile(outFileName.c_str(), "recreate");
  TTree* tout = new TTree("rootuple", "rootupled events");
  tout->SetDirectory(fout);

  TTreeLooper<JpsiFromBInputEvent, BRootupleEvent> treeLooper(tin, tout);
  treeLooper.loop(jpsiFromBRootupling, maxEvents);

  fout->Write();
  fout->Close();

  return 0;
}

#endif
