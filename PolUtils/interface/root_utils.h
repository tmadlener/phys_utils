#ifndef PHYSUTILS_POLUTILS_ROOTUTILS_H__
#define PHYSUTILS_POLUTILS_ROOTUTILS_H__

#include "TFile.h"
#include "TTree.h"
#include "TGraphAsymmErrors.h"
#include "TObjArray.h"
#include "RooWorkspace.h"
#include "RooRealVar.h"

#include <string>
#include <iostream>
#include <vector>

/** Try to open TFile with passed filename. */
TFile* checkOpenFile(const std::string& filename)
{
  TFile* f = TFile::Open(filename.c_str());
  if (f) return f;

  std::cerr << "Could not open file: \'" << filename << "\'" << std::endl;
  return nullptr;
}

/** Try to get object with name from TFile f. */
template<typename T>
T* checkGetFromFile(TFile* f, const std::string& name)
{
  T* t = static_cast<T*>(f->Get(name.c_str()));
  if (t) return t;

  std::cerr << "Could not get \'" << name << "\' from TFile \'" << f->GetName() << "\'" << std::endl;
  return nullptr;
}

inline bool checkGetEntry(TTree* t, const int event)
{
  if (t->GetEntry(event) < 0) {
    std::cerr << "I/O error while reading event " << event << " in TTree \'" << t->GetName() << "\'" << std::endl;
    return false;
  }
  return true;
}

/** deep-copy clone of object. */
template<typename T>
inline T* clone(T* t)
{
  return static_cast<T*>(t->Clone());
}

/** deep-copy clone of object. */
template<typename T>
inline T* clone(const T& t)
{
  // return clone(&t);
  return static_cast<T*>(t.Clone());
}

inline
void setPoint(TGraphAsymmErrors* g, const int i, const double x, const double y,
              const double exl, const double exh, const double eyl, const double eyh)
{
  g->SetPoint(i, x, y);
  g->SetPointError(i, exl, exh, eyl, eyh);
}

/** Get the list of all branch names in the passed TTree. */
std::vector<std::string> getBranchNames(TTree* t)
{
  std::vector<std::string> branchNames;
  auto* branchObjList = t->GetListOfBranches();
  for (int i = 0; i < branchObjList->GetEntries(); ++i) {
    branchNames.push_back(branchObjList->At(i)->GetName());
  }
  return branchNames;
}

/** get value of variable with name from workspace. */
double getVarVal(RooWorkspace* ws, const std::string& name)
{
  auto* var = static_cast<RooRealVar*>(ws->var(name.c_str()));
  if (var) return var->getVal();
  var = static_cast<RooRealVar*>(ws->function(name.c_str()));
  if (var) return var->getVal();

  std::cerr << "Could not get " << name << " from workspace" << std::endl;
  return 0; // returning zero to make this bugs a bit more subtle and harder to detect ;)
}

/** get value error of variable with name from workspace. */
double getVarError(RooWorkspace* ws, const std::string& name)
{
  auto* var = static_cast<RooRealVar*>(ws->var(name.c_str()));
  if (var) return var->getError();
  var = static_cast<RooRealVar*>(ws->function(name.c_str()));
  if (var) return var->getError();

  std::cerr << "Could not get " << name << " from workspace" << std::endl;
  return 0; // returning zero to make this bugs a bit more subtle and harder to detect ;)
}

/** set workspace variable constant to the passed value. */
inline void setVarConstant(RooWorkspace* ws, const std::string& name, const double val)
{
  ws->var(name.c_str())->setVal(val);
  ws->var(name.c_str())->setConstant(true);
}

#endif
