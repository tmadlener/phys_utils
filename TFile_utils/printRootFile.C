#include "string_stuff.h"
#include "general/recurse.h"

#include "TFile.h"
#include "TObjArray.h"
#include "TTree.h"
#include "TBranch.h"
#include "TList.h"
#include "TIterator.h"
#include "TKey.h"
#include "TObject.h"

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

// compile with: g++ printRootFile.C -o printRootFile $(root-config --libs --cflags)
// run: ./printRootFile /path/to/the/root/file.root

// TODO: The really nice and more easily maintainable thing for the ouptut would be to define a facet instead of
// indenting different levels via prepending std::string consisting of whitespace only. However, I can't seem
// to get my head around facets at the moment and I have no time.

const int indentDifference = 2; /**< difference of indentation in spaces between different "levels". */
int indentLevel; /**< keep track of the current indentation globally. */

/**
 * which TObjects have we already printed?
 * TTree::AutoSave leads to duplicate TTrees when looded up like this. ROOT manages this when the tree is obtained
 * via TFile::Get(), however here we have to do it manually.
 * This can also happen for other TObjects.
 */
std::vector<std::string> printedTObjects;

/** keep track of the current "working" directory. */
std::string currentDir;

/** Get all the branches stored in the TTree via TTree::GetListOfBranches(). */
const std::vector<std::string> getBranchNames(TTree* tree)
{
  std::vector<std::string> names;
  TObjArray* branchArray = tree->GetListOfBranches();
  for (int i = 0; i < branchArray->GetEntries(); ++i) {
    names.push_back( std::string(((TBranch*) branchArray->At(i))->GetName()) );
  }
  return names;
}

/** templatized print function that does the actual work. */
template<typename O>
void print(O* obj)
{
  std::cout << std::string(indentLevel, ' ') << obj->GetName() << " (<" << obj->ClassName() << ">)" <<std::endl;
}

/** templatized print function that does the actual work. Spezialization needed for TTree to also print TBranches. */
template<>
void print(TTree* tree)
{
  std::cout << std::string(indentLevel, ' ') << tree->GetName() << "/" << std::endl;
  indentLevel += indentDifference;
  for (const auto& name : getBranchNames(tree)) {
    std::cout << std::string(indentLevel, ' ') << name << std::endl;
  }
  indentLevel -= indentDifference;
}

/**
 * print any object.
 * This function checks the class of obj and checks if a special version of print is necessary for that class. If
 * so, performs a cast and passes on to the function which does the actual printing.
 */
void printObject(TObject* obj)
{
  std::string objName = obj->GetName();
  objName = currentDir + "/" + objName;
  indentLevel = indentDifference * countStrInStr(currentDir, "/");
  // check if the object has been printed yet and skip if so.
  if (std::find(printedTObjects.begin(), printedTObjects.end(), objName) != printedTObjects.end()) return;

  if ( obj->IsA()->InheritsFrom(TTree::Class()) ) {
    print(static_cast<TTree*>(obj));
  } else {
    print(obj);
  }

  printedTObjects.push_back(objName);
}

/**
 * print the TDirectory name and adjust the indenting.
 * Due to this not being part of the recursion routine we have to do some cleanup from possible
 * previously visited TDirectories before we can actually print the name and adjust the indentation
 * for the content of the current TDirectory.
 *
 * TODO: There are currently some differences in indentation when there are no TTrees in the TDirectory.
 * Not sure where they come from or how they can be mitigated at the moment, but in any case are only
 * whitespace related!
 */
void printDirectory(TDirectory* dir)
{
  currentDir = removeAfterLast(currentDir, "/");
  indentLevel = indentDifference * countStrInStr(currentDir, "/");

  std::string dirName = dir->GetName();
  std::cout << std::string(indentLevel, ' ') << dirName << "/" << std::endl;
  currentDir += ("/" + dirName);
}


/** print recursively by going through all TDirectories that are in the file and contain a TTree. */
template<typename T>
void printRecursively(const T* fileOrDir)
{
  auto printO = [](TObject* o) { printObject(o); };
  auto printD = [](TDirectory* d) { printDirectory(d); };
  recurseOnFile(fileOrDir, printO, printD);
}

/** "main" function. */
void printRootFile(const std::string& filename)
{
  indentLevel = 0;
  printedTObjects = std::vector<std::string>{};
  currentDir = "";

  TFile* file = TFile::Open(filename.c_str());
  if (!file) {
    std::cerr << "Cannot open file \'" << filename << "\'" << std::endl;
    exit(1);
  }
  std::cout << "contents of file \'" << filename << "\':" << std::endl;
  printRecursively(file);

  file->Close();
}

#ifndef __CINT__
int main(int argc, char* argv[])
{
  if (argc < 2) {
    std::cerr << "please provide a filename" << std::endl;
    return 1;
  }

  printRootFile(std::string(argv[1]));
  return 0;
}
#endif
