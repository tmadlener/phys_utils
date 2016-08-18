#ifndef ROOT_UTILS__TMADLENER
#define ROOT_UTILS__TMADLENER

#include "TFile.h"
#include "TIterator.h"
#include "TKey.h"
#include "TObject.h"
#include "TClass.h"
#include "TCanvas.h"

#include <string>
#include <vector>

/** check if the object is of type RT via TObject::InheritsFrom()*/
template<typename RT> inline
bool inheritsFrom(const TObject* obj)
{
  // it seems the call to ::IsA() is not necessary (This might has to be revisited for versions below 6).
  // it is even no longer listed in the documentation for version 6.07/07 (17.08.16). Nevertheless it still compiles
  return obj/*->IsA()*/->InheritsFrom(RT::Class());
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

#endif
