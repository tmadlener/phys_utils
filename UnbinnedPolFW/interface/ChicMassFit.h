#ifndef PHYSUTILS_UNBINNEDPOLFW_CHICMASSFIT_H__
#define PHYSUTILS_UNBINNEDPOLFW_CHICMASSFIT_H__

#include "general/roofit_utilities.h"

#include "RooWorkspace.h"
#include "RooArgSet.h"
#include "RooAbsPdf.h"
#include "RooAddition.h"
#include "RooMinuit.h"
#include "RooFitResult.h"

#include <vector>
#include <string>
#include <utility>
#include <iostream>

///////////////////////////////////////////////////////////////////
// SOME CONSTANTS NEEDED BELOW THAT SHOULD AT ONE POINT BE MOVED //
// TO A SEPARATE HEADER                                          //
///////////////////////////////////////////////////////////////////
const double MpsiPDG = 3.096916;
const double Mchi0PDG = 3.41475;
const double Mchi1PDG = 3.51066;
const double Mchi2PDG = 3.55620;
///////////////////////////////////////////////////////////////////


// Define the Fit model

struct ChicMassModel {
  FitModel fitModel;
  std::vector<std::pair<std::string, double> > constVals;
};

ChicMassModel createModel()
{
  const std::vector<std::pair<std::string, double> > constVals = {
    {"BK_p2", 0.1},
    {"CBn", 2.75},
    {"fracSignal_chic0", 0.03}
  };

  const std::vector<std::string> fitExpressions = {
    "RooCBShape::M_chic1(chicMass, CBmass1, CBsigma1, CBalpha1, CBn)",
    "RooCBShape::M_chic2(chicMass, CBmass2, CBsigma2, CBalpha2, CBn)",
    "RooVoigtian::M_chic0(chicMass, CBmass0, CBsigma0, CBwidth0[0.0104])",
    "RooPolynomial::M_background(chicMass, BK_p1, BK_p2)",
    "SUM::M_signal(fracSignal_chic1 * M_chic1, fracSignal_chic0 * M_chic0, M_chic2)",
  };

  const std::string fitModelExpr = {
    "SUM:M_fullModel(fracBackground * M_background, jpsi_fBkg * M_background, M_signal)"
  };

  const std::vector<FitVariable> fitVariables = {
    {"CBmass1", 3.5, 3.45, 3.54},
    {"CBsigma1", 0.008, 0.003, 0.02},
    {"CBalpha1", 0.6, 0.2, 1.1},
    {"CBalpha2", 0.6, 0.2, 1.1},
    {"CBn", 2.5, 1.8, 3.2},
    {"fracBackground", 0.6, 0.0, 1.0},
    {"fracSignal_chic1", 0.7, 0.6, 0.8},
    {"fracSignal_chic0", 0.02, 0.0, 0.1},
    {"BK_p1", 0.0, -1, 1},
    {"BK_p2", 0.0, -0.1, 1}
  };

  const std::vector<FitGeneric*> fitFormulas = {
    new FitFormula<2>("CBalpha0", "(@0+@1)/2.", {"CBalpha1", "CBalpha2"}),
    new FitFormula<1>("CBn2", "@0", {"CBn"}),
    new FitFormulaV<2,1>("PES", "(@0-%f/%f)", {MpsiPDG, Mchi1PDG-MpsiPDG}, {"CBmass1"}),
    new FitFormulaV<1,1>("CBsigma0", "@0+%f", {(Mchi0PDG - MpsiPDG)/(Mchi1PDG - MpsiPDG)}, {"CBsigma1"}),
    new FitFormulaV<1,1>("CBsigma2", "@0+%f", {(Mchi2PDG - MpsiPDG)/(Mchi1PDG - MpsiPDG)}, {"CBsigma1"}),
    new FitFormulaV<2,1>("CBmass0", "@0*%f+%f", {Mchi0PDG - MpsiPDG, MpsiPDG}, {"PES"}),
    new FitFormulaV<2,1>("CBmass2", "@0*%f+%f", {Mchi2PDG - MpsiPDG, MpsiPDG}, {"PES"})
  };

  const FitModel fitModel{fitVariables, fitFormulas, fitExpressions, fitModelExpr};

  return ChicMassModel{fitModel, constVals};
}

void doFit(RooWorkspace *ws, RooDataSet *data, const std::string& modelName,
           const std::string& saveName)
{
  using namespace RooFit;
  auto *model = ws->pdf(modelName.c_str());

  RooArgSet NLLs{};
  auto *MassNLL = static_cast<RooAbsReal*>(model->createNLL(*data, NumCPU(4)));
  NLLs.add(*MassNLL);

  RooAddition simNLL("add", "add", NLLs);
  RooMinuit *mMinuit = new RooMinuit(simNLL);

  mMinuit->setPrintEvalErrors(-1);
  mMinuit->setEvalErrorWall(false);
  mMinuit->setVerbose(false);

  mMinuit->migrad();
  RooFitResult *rltPostMigrad = static_cast<RooFitResult*>(mMinuit->save(("fitrlt_" + saveName + "_postmigrad").c_str()));
  double covQualMigrad = rltPostMigrad->covQual();
  if (covQualMigrad < 3) {
    std::cout << "covQualMigrad = " << covQualMigrad << " -> repeating migrad()\n";
    mMinuit->migrad();
    rltPostMigrad = static_cast<RooFitResult*>(mMinuit->save(("fitrlt_" + saveName + "_postmigrad").c_str()));
    std::cout << "Second turn: covQualMigrad = " << covQualMigrad << "\n";
    covQualMigrad *= 10;
    covQualMigrad += rltPostMigrad->covQual();
  }

  mMinuit->hesse();
  std::cout << "End of fit\n";

  RooFitResult *rltPostHesse = static_cast<RooFitResult*>(mMinuit->save(("fitrlt_" + saveName + "_posthesse").c_str()));
  double covQualHesse = rltPostHesse->covQual();
  std::cout << "covQualHesse = " << covQualHesse << "\n";

  std::cout << "save snapshot\n";
  ws->saveSnapshot(("snap_" + saveName).c_str(), ws->allVars());

  FitVariable("var_covQualMigrad", covQualMigrad).importToWorkspace(ws);
  FitVariable("var_covQualHesse", covQualHesse).importToWorkspace(ws);
}

#endif
