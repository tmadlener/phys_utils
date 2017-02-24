#ifndef PHYSUTILS_POLUTILS_EFFSANDCUTS_H__
#define PHYSUTILS_POLUTILS_EFFSANDCUTS_H__

#include "misc_utils.h"

#include "TH2D.h"

/** define a templatized functor for the muon acceptance and define only those spezializations that are needed. */
template<size_t N>
struct MuonAcceptance {
  bool operator()(const double pT, const double absEta);
};

template<>
bool MuonAcceptance<11>::operator()(const double pT, const double absEta)
{
  if (pT > 4.5 && absEta < 1.2) return true;
  if (pT > 3.5 && inRange(absEta, 1.2, 1.4)) return true;
  if (pT > 3.0 && inRange(absEta, 1.4, 1.6)) return true;

  return false;
}

template<>
bool MuonAcceptance<15>::operator()(const double pT, const double)
{
  return pT > 3.5;
}

/**
 * Evaulate the correction map.
 *
 * costheta and phi have to be in the HX frame!
 *
 * TODO:
 *  - systematics (random eval within errors?)
 *  - ...?
 */
double evalCorrectionMap(const TH2D* corrMap, const double cosTh, const double phi)
{
  const int binX = corrMap->GetXaxis()->FindBin(cosTh);
  const int binY = corrMap->GetYaxis()->FindBin(phi);

  return corrMap->GetBinContent(binX, binY);
}

#endif
