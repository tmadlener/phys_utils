#ifndef PHYSUTILS_POLUTILS_ROOTUTILS_H__
#define PHYSUTILS_POLUTILS_ROOTUTILS_H__

#include "TFile.h"
#include "TTree.h"
#include "TGraphAsymmErrors.h"
#include "TObjArray.h"

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
inline T* clone(const T* t)
{
  return static_cast<T*>(t->Clone());
}

/** deep-copy clone of object. */
template<typename T>
inline T* clone(const T& t)
{
  return clone(&t);
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

#endif
