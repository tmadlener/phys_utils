#include "general/string_helper.h"
#include "general/recurse.h"
#include "general/root_utils.h"

#include "TFile.h"
#include "TObjArray.h"
#include "TTree.h"
#include "TBranch.h"
#include "TList.h"
#include "TIterator.h"
#include "TKey.h"
#include "TObject.h"
#include "TDirectory.h"

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

// compile with: g++ printRootFile.C -o printRootFile $(root-config --libs --cflags)
// run: ./printRootFile /path/to/the/root/file.root

/**
 * which TObjects have we already printed?
 * TTree::AutoSave leads to duplicate TTrees when looded up like this. ROOT manages this when the tree is obtained
 * via TFile::Get(), however here we have to do it manually.
 * This can also happen for other TObjects.
 */
static std::vector<std::string> printedTObjects;

/** keep track of the current "working" directory. */
static std::string currentDir;

/** templatized print function that does the actual work. */
template<typename O>
void print(O* obj)
{
  std::cout << currentDir +  obj->GetName() << " (<" << obj->ClassName() << ">)\n";
}

/** templatized print function that does the actual work. Spezialization needed for TTree to also print TBranches. */
template<>
void print(TTree* tree)
{
  // NOTE: If the tree is on the root-directory currently two slashes are printed instead of only one
  std::cout << currentDir + tree->GetName() << ":\n";
  for (const auto& name : getBranchNames(tree)) {
    std::cout << std::string(2, ' ') << name << "\n";
  }
}

/**
 * print any object.
 * This function checks the class of obj and checks if a special version of print is necessary for that class. If
 * so, performs a cast and passes on to the function which does the actual printing.
 */
void printObject(TObject* obj, const std::string& fullPath)
{
  std::string objName = obj->GetName();
  currentDir = splitString(fullPath, ':')[1] + "/";
  objName = currentDir + objName;

  // check if the object has been printed yet and skip if so.
  if (std::find(printedTObjects.begin(), printedTObjects.end(), objName) != printedTObjects.end()) return;

  if ( inheritsFrom<TTree>(obj) ) {
    print(static_cast<TTree*>(obj));
  } else {
    print(obj);
  }

  printedTObjects.push_back(objName);
}

/** print recursively by going through all TDirectories that are in the file and contain a TTree. */
template<typename T>
void printRecursively(const T* fileOrDir)
{
  auto printO = [](TObject* o, const std::string& s) { printObject(o, s); };
  recurseOnFile(fileOrDir, printO);
}

/** "main" function. */
void printRootFile(const std::string& filename)
{
  printedTObjects = std::vector<std::string>{};
  currentDir = "";

  TFile* file = TFile::Open(filename.c_str());
  if (!file) {
    std::cerr << "Cannot open file \'" << filename << "\'\n";
    exit(1);
  }
  std::cout << "contents of file \'" << filename << "\':\n";
  printRecursively(file);

  file->Close();
}

#ifndef __CINT__
int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "please provide a filename\n";
    return 1;
  }

  printRootFile(std::string(argv[1]));
  return 0;
}
#endif
