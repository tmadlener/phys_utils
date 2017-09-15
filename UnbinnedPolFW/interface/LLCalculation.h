#ifndef PHYSUTILS_UNBINNEDPOLFW_LLCALCULATION_H__
#define PHYSUTILS_UNBINNEDPOLFW_LLCALCULATION_H__

#include "NormCalculation.h"
#include "AngularDistribution.h"
#include "Data.h"

#include <vector>
#include <cmath>
#include <algorithm>

template<size_t N, size_t M, size_t P>
double calcLogL(const PolData& data,
                const AngularParametrization<N,M,P>& A,
                const PartialExpValues<N,M,P>& pExpVals)
{
  double sum{};
  for (size_t i = 0; i < data.pT.size(); ++i) {
    const auto params = A.eval(data.x[i]);

    // calculate per-event likelihood
    const double eventLH = 1 + params.L + (1 - 3 * params.L) * data.cth2[i]
      + params.phi * data.sth2c2ph[i] + params.tp * data.s2thcph[i];

    sum += data.w[i] * std::log(eventLH);
  }

  const double norm = calcExpFcosthphi(A, pExpVals);
  const double sumWeights = std::accumulate(data.w.cbegin(), data.w.cend(), 0.0);

  sum -= std::log(norm) * sumWeights;

  return sum;
}

#endif
