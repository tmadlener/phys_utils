#include "general/ArgParser.h"
#include "root_utils.h"

#include "BmassFit.h"

#include "TFile.h"
#include "RooWorkspace.h"

#include <string>

#ifndef __CINT__
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);

  const auto ifn = parser.getOptionVal<std::string>("--input");
  const auto pdffile = parser.getOptionVal<std::string>("--pdfname");
  const double nSigma = parser.getOptionVal<double>("--nSig", 3.0);

  TFile* f = checkOpenFile(ifn);
  auto* ws = checkGetFromFile<RooWorkspace>(f, "ws_masslifetime");

  auto srAndFrac = calcSRandBGFraction(ws, nSigma);

  auto* c = plotMassProjection(ws, srAndFrac);

  c->SaveAs(pdffile.c_str());

  return 0;
}

#endif
