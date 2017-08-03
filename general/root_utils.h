#ifndef PHYSUTILS_GENERAL_ROOTUTILS_H__
#define PHYSUTILS_GENERAL_ROOTUTILS_H__

#include "TFile.h"
#include "TIterator.h"
#include "TKey.h"
#include "TObject.h"
#include "TClass.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TObjArray.h"
#include "TGraphAsymmErrors.h"
#include "TChain.h"
#include "RooWorkspace.h"
#include "RooRealVar.h"

#include <string>
#include <vector>
#include <iostream>

/** check if the object is of type RT via TObject::InheritsFrom()*/
template<typename RT> inline
bool inheritsFrom(const TObject* obj)
{
  // it seems the call to ::IsA() is not necessary (This might has to be revisited for versions below 6).
  // it is even no longer listed in the documentation for version 6.07/07 (17.08.16). Nevertheless it still compiles
  return obj/*->IsA()*/->InheritsFrom(RT::Class());
}

/** check if the object inherits from RT and return a pointer to it after a static_cast. */
template<typename RT> inline
RT* conditionalCast(TObject* obj)
{
  if (!inheritsFrom<RT>(obj)) return nullptr;
  return static_cast<RT*>(obj);
}

/** get from Root file. Using dynamic cast to get throught the inheritance stuff correctly. */
template<typename T> inline
T* getFromFile(TFile* f, const std::string& name)
{
  return dynamic_cast<T*>(f->Get(name.c_str()));
}

/**
 * get all objects that are of a certain type from the TFile or TDirectory.
 * NOTE: Does only search in the current directory and does not traverse to sub directories.
 */
template<typename T, typename F=TFile>
std::vector<T*> getAllFromFile(F* f)
{
  std::vector<T*> objects;
  TIter nextKey(f->GetListOfKeys());
  TKey* key = nullptr;
  while ( (key = static_cast<TKey*>(nextKey())) ) { // loop over all keys and check if they inherit from the desired class
    TObject* obj = key->ReadObj();
    if (inheritsFrom<T>(obj)) {
      objects.push_back(static_cast<T*>(obj));
    }
  }
  return objects;
}

/** get from a TCanvas by name via dynamic cast. */
template<typename T> inline
T* getFromCanvas(const TCanvas* c, const std::string& name)
{
  return dynamic_cast<T*>(c->FindObject(name.c_str()));
}

/** clone the passed TObject with a new name (defaults to empty string). */
template<typename T> inline
T* clone(const T* obj, const std::string& name = "")
{
  return static_cast<T*>(obj->Clone(name.c_str()));
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

/** scale the passed histogram such, that it's integral will be equal to f. */
template<typename H> inline
void selfScale(H* h, const double f = 1.0)
{
  h->Scale(f / h->Integral());
}

/** get a scaled clone of the passed histogram. */
template<typename H> inline
H* cloneScale(const H* hist, const double f = 1.0)
{
  H* h = clone(hist);
  selfScale(h, f);
  return h;
}

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

RooRealVar* getVar(RooWorkspace* ws, const std::string& name)
{
  auto* var = static_cast<RooRealVar*>(ws->var(name.c_str()));
  if (var) return var;
  var = static_cast<RooRealVar*>(ws->function(name.c_str()));
  if (var) return var;

  std::cerr << "Could not get " << name << " from workspace\n";
  return nullptr;
}

/** get value of variable with name from workspace. */
double getVarVal(RooWorkspace* ws, const std::string& name)
{
  if (auto* var = getVar(ws, name)) {
    return var->getVal();
  }
  return 0; // returning zero to make this bugs a bit more subtle and harder to detect ;)
}

/** get value error of variable with name from workspace. */
double getVarError(RooWorkspace* ws, const std::string& name)
{
  if (auto* var = getVar(ws, name)) {
    return var->getError();
  }
  return 0; // returning zero to make this bugs a bit more subtle and harder to detect ;)
}

/** set workspace variable constant to the passed value. */
inline void setVarConstant(RooWorkspace* ws, const std::string& name, const double val)
{
  getVar(ws, name)->setVal(val);
  getVar(ws, name)->setConstant(true);
}

TChain* createTChain(const std::vector<std::string>& fileNames, const std::string& treename)
{
  TChain* inChain = new TChain(treename.c_str());
  for (const auto& name : fileNames) {
    inChain->Add(name.c_str());
  }
  return inChain;
}

TChain* createTChain(const std::string& filename, const std::string& treename)
{
  TChain* inChain = new TChain(treename.c_str());
  inChain->Add(filename.c_str());
  return inChain;
}

#endif
