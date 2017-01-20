#include "general/ArgParser.h"
#include "general/progress.h"

#include "root_utils.h"
#include "misc_utils.h"

#include "TFile.h"
#include "TTree.h"
#include "TH2D.h"

#include <string>
#include <sstream>

#ifndef __CINT__
int main(int argc, char* argv[])
{
  ArgParser parser(argc, argv);
  const auto inbase = parser.getOptionVal<std::string>("--inputbase");
  const auto outfile = parser.getOptionVal<std::string>("--outputfile");
  const int ptMin = parser.getOptionVal<int>("--ptMin", 1);
  const int ptMax = parser.getOptionVal<int>("--ptMax", 12);
  const int rapMin = parser.getOptionVal<int>("--rapMin", 1);
  const int rapMax = parser.getOptionVal<int>("--rapMax", 2);

  TFile* fout = new TFile(outfile.c_str(), "recreate");
  for (auto iRap = rapMin; iRap <= rapMax; ++iRap) {
    for (auto iPt = ptMin; iPt <= ptMax; ++iPt) {
      std::stringstream binStr;
      binStr << "_rap" << iRap << "_pT" << iPt;
      const auto fn = inbase + binStr.str() + ".root";
      TFile* f = checkOpenFile(fn);
      TTree* t = checkGetFromFile<TTree>(f, "angles");
      double cosTh;
      double phi;
      t->SetBranchAddress("costh_HX", &cosTh);
      t->SetBranchAddress("phi_HX", &phi);

      auto* hist = new TH2D(("costhphi" + binStr.str()).c_str(), "", 64, -1, 1, 16, -180, 180);
      hist->GetXaxis()->SetTitle("cos#Theta");
      hist->GetYaxis()->SetTitle("#phi");
      const int nEntries = t->GetEntries();
      auto startTime = ProgressClock::now();
      for (auto i = 0; i < nEntries; ++i) {
        t->GetEntry(i);
        hist->Fill(cosTh, phi);
        printProgress(i, nEntries, startTime, 10);
      }
      fout->cd();
      hist->Write();

      f->Close();
    }
  }

  fout->Write();
  fout->Close();

  return 0;
}
#endif
