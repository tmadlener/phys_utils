#ifndef PHYSUTILS_UNBINNEDPOLFW_DATA_H__
#define PHYSUTILS_UNBINNEDPOLFW_DATA_H__

#include "general/progress.h"

#include "Range.h"
#include "MiscHelper.h"

#include "TFile.h"
#include "TTree.h"

#include <vector>
#include <array>
#include <string>
#include <type_traits>

/**
 * Struct that holds all data needed for one ML fit.
 *
 * Structured as an SoA, since some (non-extensive but hopefully conclusive) tests have
 * shown that this produces less cache misses and is slightly faster in calculating logL
 * for a <3,2,1> parametrization. (2 ms per calculation on 890k event sample).
 */
struct PolData {
  PolData() = default;

  /** convenience wrapper. */
  void push_back(double _x, double _pT, double _cth2, double _sth2c2ph, double _s2thcph,
                 double _w, double _cth, double _phi)
  {
    x.push_back(_x);
    pT.push_back(_pT);
    cth2.push_back(_cth2);
    sth2c2ph.push_back(_sth2c2ph);
    s2thcph.push_back(_s2thcph);
    w.push_back(_w);
    cth.push_back(_cth);
    phi.push_back(_phi);
  }

  /** convenience wrapper. */
  void reserve(size_t n)
  {
    x.reserve(n);
    pT.reserve(n);
    cth2.reserve(n);
    sth2c2ph.reserve(n);
    s2thcph.reserve(n);
    w.reserve(n);
    cth.reserve(n);
    phi.reserve(n);
  }

  /** assuming that all vectors are filled equally! */
  size_t size() const { return x.size(); }

  std::vector<double> x;
  std::vector<double> pT;
  std::vector<double> cth2;
  std::vector<double> sth2c2ph;
  std::vector<double> s2thcph;
  std::vector<double> w;
  std::vector<double> cth;
  std::vector<double> phi;
};

/**
 * Struct that contains the branch names of the different branches in the root file.
 */
struct DataBranchNames {
  explicit DataBranchNames(const std::string& _x, const std::string& _pt, const std::string& _cth,
                           const std::string& _phi, const std::string& _w) :
    x(_x), pT(_pt), cth(_cth), phi(_phi), w(_w) {}

  const std::string x;
  const std::string pT;
  const std::string cth;
  const std::string phi;
  const std::string w;
};



/**
 * Read data from file and return it in PolData format. Pre-calculates everything that is
 * possible.
 *
 * If 'x' and 'pT' are the same in the passed DataBranchNames, this function will handle
 * this appropriately and return the same values in both variables in PolData.
 *
 * The ranges will be applied to the x and pt variable.
 */
template<RangeType XRT, RangeType PTRT>
PolData readFromFile(TFile* f, const std::string& treename, const DataBranchNames& branches,
                     const Range<XRT>& xRange, const Range<PTRT>& ptRange)
{
  auto* t = static_cast<TTree*>(f->Get(treename.c_str()));

  double x, pT, cth, phi, w;

  // check if pT and x branchname are the same
  // If they are only set one branch and do a conditional assignment in the loop over the
  // TTree, since setting two branches with the same name to two different variables will
  // not work
  const bool xIsPt = (branches.x == branches.pT);

  if (!xIsPt) {
    t->SetBranchAddress(branches.x.c_str(), &x);
  }
  t->SetBranchAddress(branches.pT.c_str(), &pT);
  t->SetBranchAddress(branches.cth.c_str(), &cth);
  t->SetBranchAddress(branches.phi.c_str(), &phi);
  t->SetBranchAddress(branches.w.c_str(), &w);

  PolData data;
  const int nEntries = t->GetEntries();

  data.reserve(nEntries);

  std::cout << "Reading " << nEntries << " dilepton events" << std::endl; // flush

  const auto startTime = ProgressClock::now();
  for (int i = 0; i < nEntries; ++i) {
    t->GetEvent(i);

    const double xx = xIsPt ? pT : x;

    if (!(xRange.contains(xx) && ptRange.contains(pT))) continue;

    const double cth2 = cth * cth;
    const double sth2 = 1.0 - cth2;
    const double sth2c2ph = sth2 * std::cos(2 * toRad(phi));
    const double s2thcph = 2 * cth * std::sqrt(sth2) * std::cos(toRad(phi));

    data.push_back(xx, pT, cth2, sth2c2ph, s2thcph, w, cth, phi);

    printProgress(i, nEntries, startTime);
  }
  std::cout << "\n";

  delete t;
  return data;
}

#endif
