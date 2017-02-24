#ifndef POLFW_ROOFITUTILITIES_H__
#define POLFW_ROOFITUTILITIES_H__

////////////////////////////////////////////////////////////////////////////////////////////////
// header including some small helper classes for easier creation of models in a RooWorkspace //
////////////////////////////////////////////////////////////////////////////////////////////////

#include "string_helper.h"

#include "RooRealVar.h"
#include "RooFormulaVar.h"
#include "RooWorkspace.h"
#include "RooGenericPdf.h"

#include <string>
#include <vector>
#include <array>

/** set the constants in the format {"name", val} to be constant in the fit*/
void setConstants(RooWorkspace* ws, const std::vector<std::pair<std::string, double> >& constVals)
{
  for (const auto& cv : constVals) {
    setVarConstant(ws, cv.first, cv.second);
  }
}

/**
 * get the name as it will appear in the RooWorkspace from an expression, that is imported via
 * RooWorkspace::factory(expr.c_str())
 */
std::string getNameFromExpression(const std::string& expr)
{
  const auto strListColon = splitString(expr, ':');
  if (strListColon.size() > 1) {
    const auto strListParen = splitString(strListColon[1], '(');
    if (!strListParen.empty()) {
      return strListParen[0];
    }
  }
  std::cerr << "Could not extract name from expression \'" << expr << "\'" << std::endl;
  return "";
}

/**
 * abstract base class for a common-interface for generic stuff that cannot be handled by a simple
 * expression and the RooWorkspace::factory()
 */
struct FitGeneric {
  /** import the formula to the passed workspace. */
  virtual void importToWorkspace(RooWorkspace* ws) const = 0;
};

/**
 * Implementation of the FitFormula interface, templated by the number of arguments in the formula.
 */
template<size_t N>
struct FitFormula : FitGeneric {
  FitFormula(const std::string& forname, const std::string expression, const std::array<std::string, N> arguments) :
    name(forname), expr(expression), args(arguments) {}

  /** import the forumla to the passed workspace. NOTE: only needed spezializations exist! */
  virtual void importToWorkspace(RooWorkspace* ws) const override;

  const std::string name;
  const std::string expr;
  const std::array<std::string, N> args;
};

template<>
void FitFormula<2>::importToWorkspace(RooWorkspace* ws) const
{
  RooFormulaVar tmp(name.c_str(), expr.c_str(),
                     RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()))
                     );
  ws->import(tmp);
}

template<>
void FitFormula<3>::importToWorkspace(RooWorkspace* ws) const
{
  RooFormulaVar tmp(name.c_str(), expr.c_str(),
                    RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()), *ws->var(args[2].c_str()))
                    );
  ws->import(tmp);
}

template<>
void FitFormula<4>::importToWorkspace(RooWorkspace* ws) const
{
  RooFormulaVar tmp(name.c_str(), expr.c_str(),
                    RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()), *ws->var(args[2].c_str()),
                               *ws->var(args[3].c_str()))
                    );
  ws->import(tmp);
}

/**
 * Implementation of the FitFormula interface for the RooProduct / RooAddition (class of RooFit, of which I am not sure if
 * I handle it correclty with the FitFormulaInt<2>.)
 * NOTE: this works only for expressions with 2 arguments!
 * This could be done via RooWorkspace::factory(), see:
 * https://root.cern.ch/root/htmldoc/tutorials/roofit/rf512_wsfactory_oper.C.html#67
 */
template<typename T>
struct RooClass : FitGeneric {
  RooClass(const std::string& cname, const std::string& carg1, const std::string& carg2) :
    name(cname), arg1(carg1), arg2(carg2) {}

  /** import the product to the passed workspace. */
  virtual void importToWorkspace(RooWorkspace* ws) const override;

  const std::string name;
  const std::string arg1;
  const std::string arg2;
};

template<typename T>
void RooClass<T>::importToWorkspace(RooWorkspace* ws) const
{
  T tmp(name.c_str(), name.c_str(), RooArgList(*ws->var(arg1.c_str()), *ws->var(arg2.c_str())));
  ws->import(tmp);
}

/**
 * Implementation of the RooGenericPdf interface templated by the number of arguments
 * This could be done via RooWorkspace::factory() by enclosing the expression in single quotes ('')
 * see: https://root.cern.ch/root/htmldoc/tutorials/roofit/rf512_wsfactory_oper.C.html#83
 */
template<size_t N>
struct GenericPdf : FitGeneric {
  GenericPdf(const std::string& pname, const std::string& pexpr, const std::array<std::string, N>& pargs) :
    name(pname), expr(pexpr), args(pargs) {}

  virtual void importToWorkspace(RooWorkspace* ws) const override;

  const std::string name;
  const std::string expr;
  const std::array<std::string, N> args;
};

template<>
void GenericPdf<3>::importToWorkspace(RooWorkspace* ws) const
{
  RooGenericPdf tmp(name.c_str(), name.c_str(), expr.c_str(),
                    RooArgList(*ws->var(args[0].c_str()), *ws->var(args[1].c_str()), *ws->var(args[2].c_str()))
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

/** complete model description in one class. */
struct FitModel {
  FitModel(const std::vector<FitVariable>& vars, const std::vector<FitGeneric*>& gens,
           const std::vector<std::string>& exprs, const std::string& fullModel)
    : fitVariables(vars), fitGenerics(gens), fitExpressions(exprs), fullModelExpression(fullModel)
  {/*No-op*/}

  /** import to the passed workspace. */
  void importToWorkspace(RooWorkspace* ws) const;

  /**
   * all the fitvariables that are needed for the fit, that are not already in the workspace
   * or are not implicitly declared in one of the expressions.
   */
  std::vector<FitVariable> fitVariables;

  /**
   * RooFormulaVars that will be used in the fitExpressions or in the full model
   */
  std::vector<FitGeneric*> fitGenerics;

  /**
   * string expressions that will be directly handed to the RooWorkspace::factory() method.
   * Can make use of all the variables already in the workspace or can declare new ones on the fly
   * (with the 'normal' RooWorkspace::factory() syntax)
   */
  std::vector<std::string> fitExpressions;

  /**
   * The full model expression, describing the final model that can make use of all varibles, formulas and expressions
   * previously declared/used (or already in workspace from somewhere else).
   * COULD in principle also reside in the fitExpressions vector, but is given a slightly more prominent own variable.
   */
  std::string fullModelExpression;

  std::string name() const { return getNameFromExpression(fullModelExpression); }
};

void FitModel::importToWorkspace(RooWorkspace* ws) const
{
  for (const auto& var : fitVariables) {
    var.importToWorkspace(ws);
  }

  for (const auto* form : fitGenerics) {
    form->importToWorkspace(ws);
  }

  for (const auto& expr : fitExpressions) {
    ws->factory(expr.c_str());
  }

  ws->factory(fullModelExpression.c_str());
}

#endif
