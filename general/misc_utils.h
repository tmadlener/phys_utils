#ifndef PHYSUTILS_GENERAL_MISCUTILS_H__
#define PHYSUTILS_GENERAL_MISCUTILS_H__

#include <cmath>

/** check if the passed value is in ragne (min, max) */
template<typename T, typename U>
inline bool inRange(const T val, const U min, const U max)
{
  return (val >= min && val < max);
}

/** int specialization of inRange function, where min and max are considered to belong to the range. */
template<>
inline bool inRange(const int val, const int min, const int max)
{
  return (val >= min && val <= max);
}

/** get the bin of val in binning. return -1 if no bin can be found. */
template<typename T, typename C>
int getBin(const T val, const C& binning)
{
  for (size_t i = 0; i < binning.size() - 1; ++i) {
    if (inRange(val, binning[i], binning[i+1])) return i;
  }
  return -1;
}

/** delta phi. */
double deltaPhi(const double phi1, const double phi2)
{
  constexpr double twopi = 2 * M_PI;
  const double dphi = phi1 - phi2;
  if (std::abs(dphi) > M_PI) {
    return dphi < 0 ? twopi + dphi : dphi - twopi;
  }
  return dphi;
}

/** delta R */
inline double deltaR(const double eta1, const double eta2, const double phi1, const double phi2)
{
  const double dPhi = deltaPhi(phi1, phi2);
  const double dEta = eta1 - eta2;
  return std::sqrt(dEta*dEta + dPhi*dPhi);
}

#endif
