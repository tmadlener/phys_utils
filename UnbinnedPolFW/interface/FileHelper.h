#ifndef PHYSUTILS_UNBINNEDPOLFW_FILEHELPER_H__
#define PHYSUTILS_UNBINNEDPOLFW_FILEHELPER_H__

#include "TTree.h"

#include <array>
#include <vector>
#include <string>

/** hacky way to get all needed values from file. */
std::array<std::vector<double>,4> readFromTTree(TTree* t, const std::string& pt,
                                                const std::string& costh, const std::string& phi,
                                                const std::string& wS)
{
  double p;
  double c;
  double ph;
  double w;

  const int nEntries = t->GetEntries();
  std::vector<double> ptVals;
  ptVals.reserve(nEntries);
  std::vector<double> costhVals;
  costhVals.reserve(nEntries);
  std::vector<double> phiVals;
  phiVals.reserve(nEntries);
  std::vector<double> wVals;
  wVals.reserve(nEntries);

  t->SetBranchAddress(pt.c_str(), &p);
  t->SetBranchAddress(costh.c_str(), &c);
  t->SetBranchAddress(phi.c_str(), &ph);
  t->SetBranchAddress(wS.c_str(), &w);

  for (int i = 0; i < nEntries; ++i) {
    t->GetEntry(i);
    ptVals.push_back(p);
    costhVals.push_back(c);
    phiVals.push_back(ph);
    wVals.push_back(w);
  }

  return std::array<std::vector<double>, 4>{ptVals, costhVals, phiVals, wVals};
}

#endif
