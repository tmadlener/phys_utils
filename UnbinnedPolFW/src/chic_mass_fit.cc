#include "Region.h"
#include "BkgWeightCalcChic.h"
#include "ChicMassFit.h"

#include "general/root_utils.h"
#include "general/roofit_utilities.h"
#include "general/ArgParser.h"

#include <string>
#include <iostream>
#include <regex>

std::string getBinFromFile(const std::string& filename)
{
  // declaring a separate (char *) string here to be able to print it later in case
  constexpr auto rgxstr = ".*_rap([0-9]+)_pt([0-9]+).*";
  std::regex rgx(rgxstr);
  std::smatch cm;
  if (std::regex_match(filename, cm, rgx)) {
    return "rap" + cm[1].str() + "_pt" + cm[2].str();
  }

  std::cerr << "Could't match " << rgxstr << " to " << filename << "\n";
  return "";
}

#if !(defined(__CINT__) or defined(__CLING__))
int main(int argc, char *argv[])
{
  ArgParser parser(argc, argv);
  const auto fitFileName = parser.getOptionVal<std::string>("--fitfile");
  const auto outFileName = parser.getOptionVal<std::string>("--outfile", "mass_fit_chic.root");

  TFile* fitFile = checkOpenFile(fitFileName);
  auto* ws = checkGetFromFile<RooWorkspace>(fitFile, "ws_masslifetime");

  const std::string binname = getBinFromFile(fitFileName);
  const std::string dataname = "data_" + binname + "_SR";
  // 1) split J/psi sample into PR & NP sample
  //   a) define PR signal region and NP region

  auto jpsiLTRegions = calcLifeTimeRegionsJpsi(ws, dataname);
  auto *dataSR = static_cast<RooDataSet*>(ws->data(dataname.c_str()));
  auto *dataPR = static_cast<RooDataSet*>(dataSR->reduce(jpsiLTRegions.PR.getCutStr("Jpsict").c_str()));
  auto *dataNP = static_cast<RooDataSet*>(dataSR->reduce(jpsiLTRegions.NP.getCutStr("Jpsict").c_str()));

  // 2) build mass moe
  const auto massModel = createModel();
  massModel.fitModel.importToWorkspace(ws);
  setConstants(ws, massModel.constVals);

  const std::string modelName = getNameFromExpression(massModel.fitModel.fullModelExpression);

  doFit(ws, dataPR, modelName, binname + "_PR");
  doFit(ws, dataNP, modelName, binname + "_NP");

  ws->writeToFile(outFileName.c_str(), true);

  return 0;
}

#endif
