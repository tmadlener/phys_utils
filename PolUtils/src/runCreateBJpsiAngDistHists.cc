#include "general/ArgParser.h"
#include "general/string_helper.h"

#include "misc_utils.h"
#include "general/root_utils.h"

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"

#include <vector>
#include <string>

#ifndef __CINT__
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto infile = parser.getOptionVal<std::string>("--inputfile");
  const auto outputfile = parser.getOptionVal<std::string>("--outputfile", "bjpsiAngDists.root");

  TFile* fin = checkOpenFile(infile);
  TTree* tin = checkGetFromFile<TTree>(fin, "rawdata");
  TFile* fout = new TFile(outputfile.c_str(), "recreate");
  for (const auto& branch : getBranchNames(tin)) {
    std::vector<double>* vals = nullptr;
    tin->SetBranchAddress(branch.c_str(), &vals);
    tin->GetEntry(0); // all values are stored in the only element of the branch
    if (!vals->empty()) {
      const double min = startsWith(branch, "cosTh2") ? 0.0 : -1.0;
      TH1D* h = new TH1D(branch.c_str(), "", 100, min, 1.0);
      for (const auto& v : *vals) h->Fill(v);

      std::string xAxis = "";
      if (startsWith(branch, "cosTh2")) xAxis = "cos^{2}#Theta";
      else if (startsWith(branch, "sinTh2cos2Ph")) xAxis = "sin^{2}#Theta cos2#Phi";
      else if (startsWith(branch, "sin2ThcosPh")) xAxis = "sin 2#Theta cos#Phi";
      h->SetXTitle(xAxis.c_str());

      h->Write();
    }
    delete vals;
  }

  fin->Close();
  fout->Write();
  fout->Close();

  return 0;
}
#endif
