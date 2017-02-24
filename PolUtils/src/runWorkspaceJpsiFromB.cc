#include "general/ArgParser.h"
#include "root_utils.h"

#include "config/GeneralJpsiFromB.h"

#include "createWorkspaceJpsiFromB.h"

#include "TFile.h"
#include "RooDataSet.h"

#ifndef __CINT__
int main(int argc, char* argv[])
{
  const ArgParser inArgs(argc, argv);

  const auto infile = inArgs.getOptionVal<std::string>("--inputfile");
  TFile* fin = checkOpenFile(infile);

  const std::string intree = "selectedData";
  TTree* tin = checkGetFromFile<TTree>(fin, intree);

  auto* fullDataSet = createFullDataSet(tin);

  const auto outfileBase = inArgs.getOptionVal<std::string>("--outputbase");
  splitAndStoreDataSets(fullDataSet, config::Jpsi.ptBinning, config::Jpsi.rapBinning, outfileBase);

  return 0;
}


#endif
