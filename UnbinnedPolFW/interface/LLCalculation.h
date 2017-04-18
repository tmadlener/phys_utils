#ifndef PHYSUTILS_UNBINNEDPOLFW_LLCALCULATION_H__
#define PHYSUTILS_UNBINNEDPOLFW_LLCALCULATION_H__

#include "NormCalculation.h"
#include "AngularDistribution.h"

#include <vector>
#include <cmath>
#include <algorithm>

template<size_t N, size_t M, size_t P>
double calcLogL(const std::vector<double>& pT, const std::vector<double>& costh,
                const std::vector<double>& phi, const std::vector<double>& wS,
                const AngularParametrization<N,M,P>& A,
                const PartialExpValues<N,M,P>& pExpVals)
{
  double sum{};
  for (size_t i = 0; i < pT.size(); ++i) {
    sum += wS[i] * std::log(fCosThetaPhi(costh[i], phi[i], A, pT[i]));
  }

  const double norm = calcExpFcosthphi(A, pExpVals);
  const double sumWeights = std::accumulate(wS.cbegin(), wS.cend(), 0.0);
  sum -= std::log(norm) * sumWeights;

  return sum;
}

#endif
