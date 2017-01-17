#include "general/integration.h"
#include "general/progress.h"
#include "general/ArgParser.h"

#include "referenceMapCreation.h"
#include "misc_utils.h"

#include "TFile.h"
#include "TH2D.h"

#include <functional>
#include <string>

#ifndef __CINT__
int main(int argc, char* argv[])
{
  ArgParser parser(argc, argv);

  const auto fn = parser.getOptionVal<std::string>("--filename");
  const double lth = parser.getOptionVal<double>("--lth", 0);
  const double lph = parser.getOptionVal<double>("--lph", 0);
  const double ltp = parser.getOptionVal<double>("--ltp", 0);
  const int nBinsCt = parser.getOptionVal<double>("--binsCt", 64);
  const int nBinsPhi = parser.getOptionVal<double>("--binsPhi", 16);
  // if a pt AND a rap bin is passed as argument, the histogram will be named after them in the produced file
  // otherwise the actual parameters (fixed to 1 decimal point) are used for naming
  const int ptBin = parser.getOptionVal<int>("--pt", -1);
  const int rapBin = parser.getOptionVal<int>("--rap", -1);

  TFile* f = new TFile(fn.c_str(), "update"); // make it possible to store more than one ref map in a file
  auto* test = createReferenceMap(lth, lph, ltp, nBinsCt, nBinsPhi);

  if (ptBin >= 0 && rapBin >= 0) {
    test->SetName(("cosThPhi_refMap" + getBinString(rapBin, ptBin)).c_str());
  }

  test->Write();

  f->Write();
  f->Close();

  return 0;
}

#endif
