#ifndef POLFW_ROOFITUTILITIES_H__
#define POLFW_ROOFITUTILITIES_H__

////////////////////////////////////////////////////////////////////////////////////////////////
// header including some small helper classes for easier creation of models in a RooWorkspace //
////////////////////////////////////////////////////////////////////////////////////////////////

#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooWorkspace.h"

#include <string>
#include <vector>
#include <array>

/**
 * abstract base class for a common-interface.
 * Needed as a formula can have different numbers of arguments.
 */
struct FitFormula {
  /** import the formula to the passed workspace. */
  virtual void importToWorkspace(RooWorkspace* ws) const = 0;
};

/**
 * Implementation of the FitFormula interface, templated by the number of arguments in the formula.
 */
template<size_t N>
struct FitFormulaInt : FitFormula {
  FitFormulaInt(const std::string& forname, const std::string expression, const std::array<std::string, N> arguments) :
    name(forname), expr(expression), args(arguments) {}

  /** import the forumla to the passed workspace. NOTE: only needed spezializations exist! */
  virtual void importToWorkspace(RooWorkspace* ws) const override;

  const std::string name;
  const std::string expr;
  const std::array<std::string, N> args;
};

template<>
void FitFormulaInt<3>::importToWorkspace(RooWorkspace* ws) const
{
  RooFormulaVar tmp(name.c_str(), expr.c_str(),
                    RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()), *ws->var(args[2].c_str()))
                    );
  ws->import(tmp);
}

template<>
void FitFormulaInt<4>::importToWorkspace(RooWorkspace* ws) const
{
  RooFormulaVar tmp(name.c_str(), expr.c_str(),
                    RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()), *ws->var(args[2].c_str()),
                               *ws->var(args[3].c_str()))
                    );
  ws->import(tmp);
}

/** Fit Variable abstraction. */
struct FitVariable {
  FitVariable(const std::string& varname, const double val) : name(varname), vals({val}) {}
  FitVariable(const std::string& varname, const double min, const double max) : name(varname), vals({min, max}) {}
  FitVariable(const std::string& varname, const double val, const double min, const double max)
    : name(varname), vals({val, min, max}) {}

  /** import variable to the passed workspace. */
  void importToWorkspace(RooWorkspace* ws) const;

  const std::string name;
  const std::vector<double> vals;
};

void FitVariable::importToWorkspace(RooWorkspace* ws) const
{
  RooRealVar* tmpVar;
  switch(vals.size()) {
  case 1:
    tmpVar = new RooRealVar(name.c_str(), name.c_str(), vals[0]);
    break;
  case 2:
    tmpVar = new RooRealVar(name.c_str(), name.c_str(), vals[0], vals[1]);
    break;
  case 3:
    tmpVar = new RooRealVar(name.c_str(), name.c_str(), vals[0], vals[1], vals[2]);
    break;
  default:
    std::cerr << "Cannot import " << name
              << " into workspace, because the number of numerical arguments is not between 1 and 3" << std::endl;
    return;
  }

  ws->import(*tmpVar);
  delete tmpVar;
}

#endif
