#ifndef PHYS_UTILS_RECURSE__
#define PHYS_UTILS_RECURSE__

#include "TObject.h"
#include "TDirectory.h"
#include "TIterator.h"
#include "TKey.h"
#include "TClass.h"

// #include "type_deduction_helper.h"

#include <functional>

/**
 * No-op function taking a TDirectory* that is used as default argument for the dirFunc in
 * recurseOnFile below.
 */
std::function<void(TDirectory*)> noopVoidFunction([](TDirectory*)->void {;});

/**
 * Generic root file recursion function that traverses all TDirectories that can be found in a TFile.
 * For every key that is found in the file it is checked if it inherits from TDirectory and if not
 * the function passed in via the func parameter is executed.
 *
 * The definition of the default argument of dirFunc makes it possible to invoke this function with
 * only two arguments when no additional action on a directory is needed.
 *
 * @param file the TFile or TDirectory to recurse on.
 * @param func any function object that takes a TObject* as its single argument.
 * @param dirFunc any function taking a TDirectory* as its single argument that is executed on each
 * directory but is not part of the recursion.
 */
template<typename T, typename F, typename DF = decltype(noopVoidFunction)>
void recurseOnFile(const T* file, F func, DF dirFunc = noopVoidFunction)
{
  TIter nextKey(file->GetListOfKeys());
  TKey* key = nullptr;
  while ((key = static_cast<TKey*>(nextKey()))) {
    TObject* obj = key->ReadObj();
    if ((!obj->IsA()->InheritsFrom(TDirectory::Class()))) {
      func(obj);
    } else {
      // TDirectory is the base-class that implements the GetListOfKeys function
      // static_cast should be fine here, since we already checked above if we inherit from TDirectory
      TDirectory* dir = static_cast<TDirectory*>(obj);
      dirFunc(dir);
      recurseOnFile(dir, func, dirFunc);
    }
  }
}

#endif
