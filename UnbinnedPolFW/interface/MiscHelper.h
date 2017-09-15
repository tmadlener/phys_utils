#ifndef PHYSUTILS_UNBINNEDPOLFW_MISCHELPERS_H__
#define PHYSUTILS_UNBINNEDPOLFW_MISCHELPERS_H__

#include <cmath>

/**
 * convert deg to rad
 */
inline double toRad(const double x)
{
  constexpr double piO180 = M_PI / 180;
  return x * piO180;
}

#endif
