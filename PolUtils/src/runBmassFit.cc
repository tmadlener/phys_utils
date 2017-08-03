#include "general/ArgParser.h"
#include "general/root_utils.h"

#include "BmassFit.h"

#include "config/BMassFit.h"

#include "TFile.h"
#include "RooWorkspace.h"

#include <string>

#ifndef __CINT__
int main(int argc, char* argv[])
{
  ArgParser parser(argc, argv);

  // do this first unbinned
  const auto ifn = parser.getOptionVal<std::string>("--input");
  const auto ofn = parser.getOptionVal<std::string>("--output");

  TFile* f = checkOpenFile(ifn);
  RooWorkspace* ws = checkGetFromFile<RooWorkspace>(f, "ws_masslifetime");

  setupMassFit(ws, config::bPlusMassFit.fitModel, config::bPlusMassFit.constVals);

  doFit(ws, "Bmass_model");

  ws->writeToFile(ofn.c_str());

  return 0;
}

#endif
