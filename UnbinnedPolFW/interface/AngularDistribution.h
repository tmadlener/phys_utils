#ifndef PHYSUTILS_UNBINNEDPOLFW_ANGULARDISTRIBUTION_H__
#define PHYSUTILS_UNBINNEDPOLFW_ANGULARDISTRIBUTION_H__

#include "AngularParameters.h"

#include <cmath>

/**
 * Angular distribution linear in polarization parameters
 */
double fCosThetaPhi(const double costh, const double phi,
                    const double A_L, const double A_phi, const double A_thetaphi)
{
  using namespace std;
  constexpr double pi = atan(1) * 4;
  constexpr double toRad = pi / 180.0;

  const double costh2 = costh * costh;
  const double sinth2 = 1 - costh2;
  const double sinth2cos2phi = sinth2 * cos(2 * phi * toRad);
  // 2 * sin(x) * cos(x) = sin(2*x)
  const double sin2thcosph = 2 * sqrt(sinth2) * costh * cos(phi * toRad);

  return 1 + A_L + (1 - 3 * A_L) * costh2 + A_phi * sinth2cos2phi + A_thetaphi * sin2thcosph;
}

/**
 * Angular distribution linear in polarization parameters, overload for direct call with
 * parametrization and point x to eval on.
 */
template<size_t N, size_t M, size_t P>
double fCosThetaPhi(const double costh, const double phi,
                    const AngularParametrization<N,M,P>& A, const double x)
{
  const auto params = A.eval(x);
  return fCosThetaPhi(costh, phi, params.L, params.phi, params.tp);
}

#endif
