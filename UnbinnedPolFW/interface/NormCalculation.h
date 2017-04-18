#ifndef PHYSUTILS_UNBINNEDPOLFW_NORMCALCULATION_H__
#define PHYSUTILS_UNBINNEDPOLFW_NORMCALCULATION_H__

#include "AngularParameters.h"

#include <array>
#include <iostream>

/** helper struct holding the expectation values of the different parts needed. */
template<size_t N, size_t M, size_t P>
struct PartialExpValues {
  const double ct2;
  const std::array<double, N> AL;
  const std::array<double, N> AL_ct2;
  const std::array<double, M> Aphi_st2c2p;
  const std::array<double, P> Atp_s2tcp;
};

template<size_t N>
double evalExpAtCoeffs(const std::array<double, N>& partialExp,
                       const std::array<double, N>& coeffs)
{
  double sum{};
  for (size_t i = 0; i < N; ++i) {
    sum += partialExp[i] * coeffs[i];
  }
  return sum;
}

/** evaluate the normalization using a given set of values for the parametrization.
 * TODO: proper documentation
 */
template<size_t N, size_t M, size_t P>
double calcExpFcosthphi(const AngularParametrization<N,M,P>& A,
                        const PartialExpValues<N,M,P>& pExpVals)
{
  const double expAL = evalExpAtCoeffs(pExpVals.AL, A.getValAL());
  const double expALct2 = evalExpAtCoeffs(pExpVals.AL_ct2, A.getValAL());
  const double expAphi = evalExpAtCoeffs(pExpVals.Aphi_st2c2p, A.getValAphi());
  const double expAtp = evalExpAtCoeffs(pExpVals.Atp_s2tcp, A.getValAtp());

  return 1 + expAL + pExpVals.ct2 - 3 * expALct2 + expAphi + expAtp;
}

#endif
